#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <queue>
#include <memory>
#include <functional>
#include <chrono>
#include <exception>

// Platform-specific includes
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
    #define CLOSE_SOCKET closesocket
    #define SOCKET_ERROR_CHECK(x) ((x) == SOCKET_ERROR)
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <errno.h>
    typedef int socket_t;
    #define INVALID_SOCKET -1
    #define CLOSE_SOCKET close
    #define SOCKET_ERROR_CHECK(x) ((x) < 0)
#endif

// =============================================================================
// RAII Socket Wrapper - Automatic cleanup
// =============================================================================
class Socket {
private:
    socket_t sock_;

public:
    Socket() : sock_(INVALID_SOCKET) {}

    explicit Socket(socket_t sock) : sock_(sock) {}

    // Move constructor and assignment
    Socket(Socket&& other) noexcept : sock_(other.sock_) {
        other.sock_ = INVALID_SOCKET;
    }

    Socket& operator=(Socket&& other) noexcept {
        if (this != &other) {
            close_socket();
            sock_ = other.sock_;
            other.sock_ = INVALID_SOCKET;
        }
        return *this;
    }

    // Disable copy
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    ~Socket() {
        close_socket();
    }

    bool create(int domain = AF_INET, int type = SOCK_STREAM, int protocol = 0) {
        sock_ = socket(domain, type, protocol);
        return sock_ != INVALID_SOCKET;
    }

    bool bind(const std::string& address, int port) {
        if (sock_ == INVALID_SOCKET) return false;

        // Enable address reuse - CRITICAL for servers
        int reuse = 1;
        if (setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR,
                      reinterpret_cast<const char*>(&reuse), sizeof(reuse)) < 0) {
            return false;
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);  // Network byte order!
        inet_pton(AF_INET, address.c_str(), &addr.sin_addr);

        return !SOCKET_ERROR_CHECK(::bind(sock_,
                                         reinterpret_cast<sockaddr*>(&addr),
                                         sizeof(addr)));
    }

    bool listen(int backlog = 5) {
        return sock_ != INVALID_SOCKET &&
               !SOCKET_ERROR_CHECK(::listen(sock_, backlog));
    }

    Socket accept() {
        if (sock_ == INVALID_SOCKET) return Socket{};

        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        socket_t client_sock = ::accept(sock_,
                                       reinterpret_cast<sockaddr*>(&client_addr),
                                       &client_len);

        return Socket{client_sock};
    }

    bool connect(const std::string& address, int port) {
        if (sock_ == INVALID_SOCKET) return false;

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, address.c_str(), &addr.sin_addr);

        return !SOCKET_ERROR_CHECK(::connect(sock_,
                                           reinterpret_cast<sockaddr*>(&addr),
                                           sizeof(addr)));
    }

    // Handle partial sends - CRITICAL for TCP
    ssize_t send_all(const void* data, size_t length) {
        if (sock_ == INVALID_SOCKET) return -1;

        const char* ptr = static_cast<const char*>(data);
        size_t sent = 0;

        while (sent < length) {
            ssize_t result = send(sock_, ptr + sent, length - sent, 0);
            if (SOCKET_ERROR_CHECK(result)) {
                return -1;
            }
            sent += result;
        }
        return sent;
    }

    // Handle partial receives - CRITICAL for TCP
    ssize_t recv_all(void* buffer, size_t length) {
        if (sock_ == INVALID_SOCKET) return -1;

        char* ptr = static_cast<char*>(buffer);
        size_t received = 0;

        while (received < length) {
            ssize_t result = recv(sock_, ptr + received, length - received, 0);
            if (result <= 0) {
                return result; // Error or connection closed
            }
            received += result;
        }
        return received;
    }

    void close_socket() {
        if (sock_ != INVALID_SOCKET) {
            CLOSE_SOCKET(sock_);
            sock_ = INVALID_SOCKET;
        }
    }

    bool is_valid() const { return sock_ != INVALID_SOCKET; }
    socket_t get() const { return sock_; }
};

// =============================================================================
// Thread Pool for handling connections
// =============================================================================
class ThreadPool {
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_{false};

public:
    explicit ThreadPool(size_t threads = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < threads; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;

                    // Critical section for task queue
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex_);

                        // Wait for task or stop signal
                        condition_.wait(lock, [this] {
                            return stop_.load() || !tasks_.empty();
                        });

                        if (stop_.load() && tasks_.empty()) {
                            return; // Exit thread
                        }

                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }

                    try {
                        task(); // Execute task outside critical section
                    } catch (const std::exception& e) {
                        std::cerr << "Task exception: " << e.what() << std::endl;
                    }
                }
            });
        }
    }

    ~ThreadPool() {
        stop_.store(true);
        condition_.notify_all();

        for (std::thread& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    // Disable copy and move
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    template<class F, class... Args>
    void enqueue(F&& f, Args&&... args) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);

            if (stop_.load()) {
                throw std::runtime_error("ThreadPool is stopped");
            }

            tasks_.emplace([f = std::forward<F>(f), args...]() mutable {
                f(std::move(args)...);
            });
        }
        condition_.notify_one();
    }
};

// =============================================================================
// Echo Server Implementation
// =============================================================================
class EchoServer {
private:
    Socket server_socket_;
    ThreadPool thread_pool_;
    std::atomic<bool> running_{false};
    std::atomic<int> active_connections_{0};

    void handle_client(Socket client_socket) {
        ++active_connections_;

        // RAII cleanup counter
        struct ConnectionGuard {
            std::atomic<int>& counter;
            explicit ConnectionGuard(std::atomic<int>& c) : counter(c) {}
            ~ConnectionGuard() { --counter; }
        } guard(active_connections_);

        std::cout << "Client connected. Active connections: "
                  << active_connections_.load() << std::endl;

        char buffer[1024];
        while (running_.load()) {
            ssize_t bytes_received = recv(client_socket.get(), buffer,
                                         sizeof(buffer) - 1, 0);

            if (bytes_received <= 0) {
                break; // Client disconnected or error
            }

            buffer[bytes_received] = '\0';
            std::cout << "Received: " << buffer << std::endl;

            // Echo back to client
            if (client_socket.send_all(buffer, bytes_received) < 0) {
                std::cerr << "Failed to send response" << std::endl;
                break;
            }
        }

        std::cout << "Client disconnected. Active connections: "
                  << (active_connections_.load() - 1) << std::endl;
    }

public:
    explicit EchoServer(size_t thread_count = 4) : thread_pool_(thread_count) {
#ifdef _WIN32
        WSADATA wsa_data;
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
            throw std::runtime_error("WSAStartup failed");
        }
#endif
    }

    ~EchoServer() {
        stop();
#ifdef _WIN32
        WSACleanup();
#endif
    }

    bool start(const std::string& address = "127.0.0.1", int port = 8080) {
        if (!server_socket_.create()) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }

        if (!server_socket_.bind(address, port)) {
            std::cerr << "Failed to bind to " << address << ":" << port << std::endl;
            return false;
        }

        if (!server_socket_.listen()) {
            std::cerr << "Failed to listen" << std::endl;
            return false;
        }

        running_.store(true);
        std::cout << "Server listening on " << address << ":" << port << std::endl;

        // Accept connections in main thread
        while (running_.load()) {
            Socket client_socket = server_socket_.accept();

            if (!client_socket.is_valid()) {
                if (running_.load()) {
                    std::cerr << "Failed to accept connection" << std::endl;
                }
                continue;
            }

            // Handle client in thread pool
            try {
                thread_pool_.enqueue([this](Socket client) {
                    handle_client(std::move(client));
                }, std::move(client_socket));
            } catch (const std::exception& e) {
                std::cerr << "Failed to enqueue client: " << e.what() << std::endl;
            }
        }

        return true;
    }

    void stop() {
        running_.store(false);
        server_socket_.close_socket();
    }

    int get_active_connections() const {
        return active_connections_.load();
    }
};

// =============================================================================
// Simple Echo Client for Testing
// =============================================================================
class EchoClient {
private:
    Socket client_socket_;

public:
    EchoClient() {
#ifdef _WIN32
        WSADATA wsa_data;
        WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif
    }

    ~EchoClient() {
#ifdef _WIN32
        WSACleanup();
#endif
    }

    bool connect(const std::string& address = "127.0.0.1", int port = 8080) {
        if (!client_socket_.create()) {
            std::cerr << "Failed to create client socket" << std::endl;
            return false;
        }

        if (!client_socket_.connect(address, port)) {
            std::cerr << "Failed to connect to server" << std::endl;
            return false;
        }

        std::cout << "Connected to " << address << ":" << port << std::endl;
        return true;
    }

    bool send_message(const std::string& message) {
        if (client_socket_.send_all(message.c_str(), message.length()) < 0) {
            std::cerr << "Failed to send message" << std::endl;
            return false;
        }

        char buffer[1024];
        ssize_t bytes_received = client_socket_.recv_all(buffer, message.length());

        if (bytes_received <= 0) {
            std::cerr << "Failed to receive response" << std::endl;
            return false;
        }

        buffer[bytes_received] = '\0';
        std::cout << "Echo: " << buffer << std::endl;
        return true;
    }
};

// =============================================================================
// Demonstration and Testing
// =============================================================================
void run_server() {
    try {
        EchoServer server(4); // 4 worker threads
        server.start("127.0.0.1", 8080);
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }
}

void run_client_test() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Let server start

    try {
        EchoClient client;
        if (client.connect("127.0.0.1", 8080)) {
            client.send_message("Hello, Server!");
            client.send_message("This is a test message");
            client.send_message("Goodbye!");
        }
    } catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "Starting Echo Server Demo..." << std::endl;

    // Start server in separate thread
    std::thread server_thread(run_server);

    // Run client test
    std::thread client_thread(run_client_test);

    // Let them run for a bit
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // In a real application, you'd have graceful shutdown
    std::cout << "Demo completed. Press Ctrl+C to exit." << std::endl;

    client_thread.join();
    // Note: server_thread will run indefinitely in this demo

    return 0;
}
