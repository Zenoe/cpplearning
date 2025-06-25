#ifndef SERVER_H_
#define SERVER_H_

/* #include <iostream> */
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
/* #ifdef _WIN32 */
/*     #include <winsock2.h> */
/*     #include <ws2tcpip.h> */
/*     #pragma comment(lib, "ws2_32.lib") */
/*     typedef SOCKET socket_t; */
/*     #define CLOSE_SOCKET closesocket */
/*     #define SOCKET_ERROR_CHECK(x) ((x) == SOCKET_ERROR) */
/* #else */
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <errno.h>
    typedef int socket_t;
    #define INVALID_SOCKET -1
    #define CLOSE_SOCKET ::close
    #define SOCKET_ERROR_CHECK(x) ((x) < 0)
/* #endif */

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
            close();
            sock_ = other.sock_;
            other.sock_ = INVALID_SOCKET;
        }
        return *this;
    }

    // Disable copy
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    ~Socket() {
        close();
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

    void close() {
        if (sock_ != INVALID_SOCKET) {
            CLOSE_SOCKET(sock_);
            sock_ = INVALID_SOCKET;
        }
    }

    bool is_valid() const { return sock_ != INVALID_SOCKET; }
    socket_t get() const { return sock_; }
};


#endif // SERVER_H_
