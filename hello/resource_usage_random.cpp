#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <cmath>
#include <random>
#include <sys/sysinfo.h>
#include <algorithm>
#include <memory>

// Thread control
std::atomic<bool> keepRunning(true);

// Improved random number generation
namespace Random {
    thread_local std::mt19937 generator(std::random_device{}());

    template<typename T>
    T range(T min, T max) {
        if constexpr (std::is_integral_v<T>) {
            return std::uniform_int_distribution<T>(min, max)(generator);
        } else {
            return std::uniform_real_distribution<T>(min, max)(generator);
        }
    }
}

// CPU load generator
class CPULoadWorker {
    static constexpr std::chrono::milliseconds resolution{100};

public:
    void operator()() const {
        while (keepRunning) {
            // Random CPU load between 20-50%
            const float load = Random::range(0.2f, 0.5f);
            const auto duration = std::chrono::milliseconds(Random::range(2000, 8000));

            const auto end_time = std::chrono::steady_clock::now() + duration;

            while (keepRunning && std::chrono::steady_clock::now() < end_time) {
                const auto cycle_start = std::chrono::steady_clock::now();
                const auto work_end = cycle_start +
                    std::chrono::milliseconds(static_cast<int>(resolution.count() * load));

                // Busy work
                while (std::chrono::steady_clock::now() < work_end) {
                    volatile double x = 0;
                    for (int i = 0; i < 100'000; ++i) {
                        x += std::sin(i) * std::cos(i);
                    }
                }

                // Sleep remaining time
                std::this_thread::sleep_until(cycle_start + resolution);
            }
        }
    }
};

// Memory manager
class MemoryManager {
    static constexpr size_t chunk_size = 1 << 20; // 1MB
    std::vector<std::unique_ptr<char[]>> chunks;
    size_t current_bytes = 0;
    const size_t total_memory;

public:
    explicit MemoryManager(size_t total_mem) : total_memory(total_mem) {}

    void adjust() {
        // Random memory target between 30-64%
        const float target_percent = Random::range(0.3f, 0.64f);
        const size_t target_bytes = static_cast<size_t>(total_memory * target_percent);

        if (current_bytes < target_bytes) {
            allocate(target_bytes - current_bytes);
        } else {
            release(current_bytes - target_bytes);
        }

        std::cout << "Memory usage: " << (current_bytes >> 20) << " MB ("
                  << (current_bytes * 100 / total_memory) << "%)" << std::endl;
    }

private:
    void allocate(size_t bytes) {
        const size_t chunks_needed = (bytes + chunk_size - 1) / chunk_size;

        for (size_t i = 0; i < chunks_needed && keepRunning; ++i) {
            try {
                auto chunk = std::make_unique<char[]>(chunk_size);
                // Touch each page to ensure allocation
                for (size_t j = 0; j < chunk_size; j += 4096) {
                    chunk[j] = static_cast<char>(j % 256);
                }
                chunks.push_back(std::move(chunk));
                current_bytes += chunk_size;
            } catch (const std::bad_alloc&) {
                std::cerr << "Memory allocation failed at "
                          << (current_bytes >> 20) << " MB" << std::endl;
                break;
            }
        }
    }

    void release(size_t bytes) {
        const size_t chunks_to_free = std::min(bytes / chunk_size, chunks.size());
        chunks.resize(chunks.size() - chunks_to_free);
        current_bytes -= chunks_to_free * chunk_size;
    }
};

int main() {
    // Get system information
    struct sysinfo mem_info;
    sysinfo(&mem_info);
    const size_t total_mem = mem_info.totalram * mem_info.mem_unit;
    const unsigned num_cores = std::thread::hardware_concurrency();

    std::cout << "System Information:\n"
              << "  Total RAM: " << (total_mem >> 20) << " MB\n"
              << "  CPU Cores: " << num_cores << "\n"
              << "Press ENTER to stop..." << std::endl;

    // Start CPU load threads
    std::vector<std::thread> cpu_threads;
    for (unsigned i = 0; i < num_cores; ++i) {
        cpu_threads.emplace_back(CPULoadWorker{});
    }

    // Initialize memory manager
    MemoryManager mem_manager(total_mem);

    // Main control loop
    while (keepRunning) {
        const auto interval = std::chrono::milliseconds(Random::range(2000, 8000));
        mem_manager.adjust();

        const auto next_adjust = std::chrono::steady_clock::now() + interval;
        while (keepRunning && std::chrono::steady_clock::now() < next_adjust) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    // Cleanup
    for (auto& thread : cpu_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    std::cout << "Resources released. Exiting." << std::endl;
    return 0;
}
