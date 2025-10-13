#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <cmath>
#include <sys/sysinfo.h>

std::atomic<bool> keepRunning(true);

// Function to perform CPU-intensive calculations
void cpuLoad() {
    while (keepRunning) {
        // Calculate for 30% of the time, then sleep for 70%
        auto start = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - start <
               std::chrono::milliseconds(300)) {
            // Perform some calculations
            volatile double x = 0;
            for (int i = 0; i < 100000; ++i) {
                x += std::sin(i) * std::cos(i);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(700));
    }
}

int main() {
    // Get system information
    struct sysinfo memInfo;
    sysinfo(&memInfo);

    // Calculate 40% of total RAM in bytes
    long totalMem = memInfo.totalram * memInfo.mem_unit;
    long targetMem = static_cast<long>(totalMem * 0.4);
    int chunkSize = 1024 * 1024; // 1MB chunks

    std::cout << "Total memory: " << totalMem / (1024 * 1024) << " MB" << std::endl;
    std::cout << "Target memory usage: " << targetMem / (1024 * 1024) << " MB" << std::endl;

    // Allocate memory in chunks until we reach the target
    std::vector<char*> memoryChunks;
    long allocated = 0;

    try {
        while (allocated < targetMem) {
            char* chunk = new char[chunkSize];
            // Touch each page to ensure it's actually allocated
            for (int i = 0; i < chunkSize; i += 4096) {
                chunk[i] = i % 256;
            }
            memoryChunks.push_back(chunk);
            allocated += chunkSize;

            if (allocated % (100 * 1024 * 1024) == 0) {
                std::cout << "Allocated: " << allocated / (1024 * 1024) << " MB" << std::endl;
            }
        }
    } catch (std::bad_alloc& e) {
        std::cerr << "Memory allocation failed: " << e.what() << std::endl;
        std::cerr << "Could only allocate " << allocated / (1024 * 1024) << " MB" << std::endl;
    }

    std::cout << "Memory load applied. Starting CPU load..." << std::endl;

    // Determine number of CPU cores
    unsigned int numCores = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;

    std::cout << "CPU cores: " << numCores << std::endl;

    // Create threads for CPU load
    for (unsigned int i = 0; i < numCores; ++i) {
        threads.emplace_back(cpuLoad);
    }

    // Wait for user input to stop
    std::cout << "Press Enter to stop..." << std::endl;
    std::cin.get();

    // Clean up
    keepRunning = false;
    for (auto& thread : threads) {
        thread.join();
    }

    for (char* chunk : memoryChunks) {
        delete[] chunk;
    }

    std::cout << "Resources released. Exiting." << std::endl;
    return 0;
}
