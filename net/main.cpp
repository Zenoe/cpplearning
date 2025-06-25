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

#include "echoServer.h"
#include "echoClient.h"

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
