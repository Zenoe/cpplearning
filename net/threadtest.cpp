#include "threadpool.h"

int main() {
    ThreadPool pool(4); // Use 4 threads

    for (int i = 0; i < 10; ++i) {
        pool.enqueue([i] {
            std::cout << "Task " << i << " is running in thread "
                      << std::this_thread::get_id() << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    }

    std::this_thread::sleep_for(std::chrono::seconds(2)); // Give threads time to finish
    return 0;
}
