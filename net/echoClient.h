#ifndef ECHOCLIENT_H_
#define ECHOCLIENT_H_
#include "socket.h"

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

#endif // ECHOCLIENT_H_
