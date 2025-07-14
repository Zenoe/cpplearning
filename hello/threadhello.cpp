#include <atomic>
#include <iostream>
#include <thread>

std::atomic<int> counter(0);

void increment(int a, int b) {
  std::cout << a << " " << b << '\n';
    for (int i = 0; i < 1000; ++i) {
        ++counter; // Atomic increment
    }
}

int main() {
    std::thread t1(increment, 2,3);
    std::thread t2(increment, 12,13);
    t1.join();
    t2.join();
    std::cout << "Final count: " << counter << '\n';
}
