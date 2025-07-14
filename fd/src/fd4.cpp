/**
 * * active_tasks: call atomic operation instead of ++ --
 * enclose pool creation in a {} ? dtor when out of scope
 * each thread should have their own copy of max_depth, current_depth
 * classic bug in recursive/multithreaded enqueuing if the work-counting isn’t handled precisely.
 * active_tasks will decrease to 0 before it increases again, and this causes the pool to go out of scope and be destroyed.

 * std::atomic<int> active_tasks(1);

 * in thread function:
 * active_tasks.fetch_add(1);  // when starting
// ... do work ...
for (auto &subdir: subdirs) {
    pool.enqueue([=](){
        fd_search_threaded(subdir, ...);
    });
}
active_tasks.fetch_sub(1);  // when done with THIS directory

main thread is waiting for active_tasks == 0, it might see zero before subthreads have incremented, and the thread pool can get destroyed.
because it's not guaranteeing that the increment (fetch_add(1)) for each subtask happens before its parent decrements.

Correct pattern:
Before you enqueue each child, increment the counter.
When the child finishes, it decrements.

// In fd_search_threaded, after finding subdirectories:
for (const auto& subdir : subdirs) {
    active_tasks.fetch_add(1); // Reserve a task slot, BEFORE enqueue!
    pool.enqueue([&, subdir]() {
        fd_search_threaded(subdir, ...);
        active_tasks.fetch_sub(1); // Mark this specific task done!
    });
}

// Decrement for this level, when THIS call is done.
active_tasks.fetch_sub(1);

At the "top level":
Main kicks off root by active_tasks.store(1) and fd_search_threaded(root, ...), waits until zero.
or put initial state to be 1
  std::atomic<int> active_tasks(1);
 */

#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <memory>
#include <thread>
#include <future>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <re2/re2.h>

#include "net/threadpool.h"
#include "gutils.h"
#include "tool.h"

namespace fs = std::filesystem;
using std::unique_ptr;
using std::make_unique;
using std::cout;

int g_count = 0;
std::mutex coutmtx;

// Check if a path matches any .gitignore rule
bool is_ignored(const fs::path& path, const std::vector<unique_ptr<RE2>>& rules) {
    std::string_view path_view = path.string();
    for (const auto& rule : rules) {
        if (RE2::PartialMatch(path_view, *rule)) {
            return true;
        }
    }
    return false;
}

// Thread-safe result collector
class ResultCollector {
private:
    std::vector<std::string> results;
    std::mutex mutex;

public:
    void add_result(const std::string& path) {
        std::lock_guard<std::mutex> lock(mutex);
        results.push_back(path);
    }

    void print_results() {
        std::lock_guard<std::mutex> lock(mutex);
        for (const auto& result : results) {
          g_count +=1;
            std::cout << result << std::endl;
        }
    }
};

// Alternative implementation using thread pool for better resource management
void fd_search_threaded(
    const fs::path& dir,
    const unique_ptr<RE2>& pattern,
    const std::vector<unique_ptr<RE2>>& gitignore_rules,
    ResultCollector& collector,
    ThreadPool& pool,
    std::atomic<int>& active_tasks,
    std::mutex& output_mtx,
    int max_depth = -1,
    int current_depth = 0
) {
    // active_tasks++;
  // active_tasks.fetch_add(1, std::memory_order_relaxed);
  // active_tasks.store(1);
    try {
        std::vector<fs::path> subdirs;
        // print("dir:", dir);
        // Process current directory
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (is_ignored(entry.path(), gitignore_rules)) {
                continue;
            }
            // print(entry.path().string());

            std::string filename = entry.path().filename().string();
            if (RE2::PartialMatch(filename, *pattern)) {
              std::lock_guard<std::mutex> lock(output_mtx);
                collector.add_result(entry.path().string());
            }

            // Collect subdirectories for parallel processing
            if (entry.is_directory() && (max_depth == -1 || current_depth < max_depth)) {
                subdirs.push_back(entry.path());
            }
        }

        // Enqueue subdirectories for parallel processing
        for (const auto& subdir : subdirs) {
          // Whenever you launch work (e.g., a thread, task, closure) inside a loop, and the work needs the current item, capture it by value in the closure.
          // subdir is captured by value to ensure each thread have their own copy of var
          // print("push ---------subdir:", subdir.string());
          active_tasks.fetch_add(1, std::memory_order_relaxed);
          pool.enqueue(fd_search_threaded, subdir, std::cref(pattern),
                       std::cref(gitignore_rules), std::ref(collector),
                       std::ref(pool), // always std::ref for thread pool itself
                       std::ref(active_tasks),
                       std::ref(output_mtx), // always std::ref for mutex
                       max_depth, current_depth + 1);

          // pool.enqueue(fd_search_threaded, subdir, pattern, gitignore_rules, collector, pool, active_tasks, output_mtx, max_depth, current_depth + 1);
          // pool.enqueue([&, subdir,  max_depth, current_depth]() {
          //       fd_search_threaded(subdir, pattern, gitignore_rules, collector, pool, active_tasks, output_mtx, max_depth, current_depth + 1);
          //   });
        }

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error accessing " << dir << ": " << e.what() << std::endl;
    }

    active_tasks.fetch_sub(1, std::memory_order_relaxed);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <pattern> [directory] [--case-sensitive] [--max-depth N] [--threads N]\n";
        return 1;
    }

    // Parse arguments
    std::string pattern_str = gutils::glob_to_regex(argv[1]);
    fs::path dir = (argc > 2) ? argv[2] : ".";
    bool case_sensitive = false;
    int max_depth = -1;
    int num_threads = std::thread::hardware_concurrency();

    // Parse command line arguments
    for (int i = 3; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--case-sensitive") {
            case_sensitive = true;
        } else if (arg == "--max-depth" && i + 1 < argc) {
            max_depth = std::stoi(argv[++i]);
        } else if (arg == "--threads" && i + 1 < argc) {
            num_threads = std::stoi(argv[++i]);
        }
    }

    // Compile regex with case-insensitive flag if needed
    RE2::Options options;
    options.set_case_sensitive(case_sensitive);

    auto pattern = make_unique<RE2>(pattern_str, options);
    if(!pattern->ok()){
      std::cerr << "Invalid regex pattern: " << pattern->error() << std::endl;
      return 1;
    }

    std::vector<unique_ptr<RE2>> gitignore_rules = gutils::load_gitignore_rules(dir);

    ResultCollector collector;
    auto start_time = std::chrono::high_resolution_clock::now();
    // {
      // Create thread pool and result collector
      ThreadPool pool(num_threads);
      std::atomic<int> active_tasks(1);

      std::mutex output_mtx;

      // Use the thread pool approach for better resource management
      fd_search_threaded(dir, pattern, gitignore_rules, collector, pool, active_tasks, output_mtx, max_depth, 0);

      // Wait for all tasks to complete
      while (active_tasks.load() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }

    // }
    // std::cout << "pool dtor.....\n";
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    // Print results
    std::cerr << "Search completed in " << duration.count() << " ms using " << num_threads << " threads" << std::endl;

    collector.print_results();


    std::cout << g_count << '\n';
    return 0;
}
