#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queuemutex;
    std::condition_variable condition;
    std::atomic<bool> stop_{false};

public:
    explicit ThreadPool(size_t threads = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queuemutex);
                        condition.wait(lock, [this] {
                            return stop_.load() || !tasks.empty();
                        });
                        if (stop_.load() && tasks.empty()) return;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    try {
                        task();
                    } catch (const std::exception& e) {
                        std::cerr << "Task exception: " << e.what() << std::endl;
                    }
                }
            });
        }
    }

    ~ThreadPool() {
        stop_.store(true);
        condition.notify_all();
        for (std::thread& worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    // accept a function and multiple vars
    // able to pass arguments directly (as with std::thread or std::async),
    template<class F, class... Args>
    void enqueue(F&& f, Args&&... args) {
      {
        std::unique_lock<std::mutex> lock(queuemutex);
        if (stop_.load()) throw std::runtime_error("ThreadPool is stopped");
        tasks.emplace(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
      }
      condition.notify_one();
    }
    // void enqueue(F&& f, Args&&... args) {
    //     {
    //         std::unique_lock<std::mutex> lock(queuemutex);
    //         if (stop_.load()) {
    //             throw std::runtime_error("ThreadPool is stopped");
    //         }
    //         // initialized lambda pack captures are a c++20 extension
    //         tasks.emplace([f = std::forward<F>(f), ...args = std::forward<Args>(args)]() mutable {
    //             f(std::move(args)...);
    //         });
    //     }
    //     condition.notify_one();
    // }

    // template<class F, class... Args>
    // only accept one arg: f, one has to wrap all agrs in a lambda along with captures to be passed to f
    // accept any callable (including lambdas with captures), so you can "simulate argument passing" by capturing what you want.
    // void enqueue(F&& f) {
    //     {
    //         std::lock_guard<std::mutex> lock(queuemutex);
    //         // std::cout << "enque begin" << "\n";
    //         if (stop_) return;
    //         // std::cout << "enque end" << "\n";
    //         tasks.emplace(std::forward<F>(f));
    //     }
    //     condition.notify_one();
    // }
};


#endif // THREADPOOL_H_
