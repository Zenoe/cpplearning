#ifndef ECHOSERVER_H_
#define ECHOSERVER_H_

#include "./socket.h"
#include "threadpool.h"

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
                thread_pool_.enqueue([this, client = std::move(client_socket)]() mutable {
                    handle_client(std::move(client));
                });
            } catch (const std::exception& e) {
                std::cerr << "Failed to enqueue client: " << e.what() << std::endl;
            }
        }

        return true;
    }

    void stop() {
        running_.store(false);
        server_socket_.close();
    }

    int get_active_connections() const {
        return active_connections_.load();
    }
};

#endif // ECHOSERVER_H_
