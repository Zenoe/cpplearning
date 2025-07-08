#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <memory>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <regex>
#include <chrono>
#include <re2/re2.h>

#include "g_utils.h"

namespace fs = std::filesystem;
using std::unique_ptr;
using std::make_unique;
using std::cout;

int g_count = 0;

// Parse .gitignore rules (simplified)
std::vector<unique_ptr<RE2>> load_gitignore_rules(const fs::path& dir) {
    std::vector<unique_ptr<RE2>> rules;
    std::ifstream gitignore(dir / ".gitignore");
    if (!gitignore) return rules;

    std::string line;
    while (std::getline(gitignore, line)) {
        if (line.empty() || line.find("#") == 0) continue;
        std::string regex_str = gutils::glob_to_regex(line);
        auto rule = make_unique<RE2>(regex_str);
        if(!rule->ok()){
          std::cerr << "Invalid .gitignore regex pattern: " << rule->error() << std::endl;
            continue;
        }
        rules.emplace_back(std::move(rule));
    }
    return rules;
}

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

// Work item for directory processing
struct WorkItem {
    fs::path path;
    int depth;

    WorkItem(const fs::path& p, int d) : path(p), depth(d) {}
};

// Thread-safe queue for directory paths
class DirQueue {
    std::queue<WorkItem> q;
    std::mutex m;
    std::condition_variable cv;
    bool finished = false;

public:
    void push(const WorkItem& item) {
        std::lock_guard<std::mutex> lock(m);
        q.push(item);
        cv.notify_one();
    }

    bool pop(WorkItem& item) {
        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, [&]{ return !q.empty() || finished; });
        if (q.empty()) return false;
        item = q.front();
        q.pop();
        return true;
    }

    void set_finished() {
        std::lock_guard<std::mutex> lock(m);
        finished = true;
        cv.notify_all();
    }

    bool empty()  {
        std::lock_guard<std::mutex> lock(m);
        return q.empty();
    }
};

void worker(
    DirQueue& dq,
    const unique_ptr<RE2>& pattern,
    const std::vector<unique_ptr<RE2>>& gitignore_rules,
    int max_depth,
    std::atomic<int>& pending_work,
    std::mutex& output_mtx,
    std::condition_variable& worker_cv,
    std::mutex& worker_mtx

) {
    WorkItem item(fs::path{}, 0);

    while (dq.pop(item)) {
        try {
            // Process current directory
            for (const auto& entry : fs::directory_iterator(item.path)) {
                if (is_ignored(entry.path(), gitignore_rules)) {
                    continue;
                }

                // Check if filename matches pattern
                std::string filename = entry.path().filename().string();
                if (RE2::PartialMatch(filename, *pattern)) {
                    std::lock_guard<std::mutex> lock(output_mtx);
                    std::cout << entry.path().string() << std::endl;
                    g_count +=1;
                }

                // Add subdirectories to queue
                if (entry.is_directory() && (max_depth == -1 || item.depth < max_depth)) {
                    pending_work.fetch_add(1);
                    dq.push(WorkItem(entry.path(), item.depth + 1));
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::lock_guard<std::mutex> lock(output_mtx);
            std::cerr << "Error accessing " << item.path << ": " << e.what() << std::endl;
        }

        // Mark this work item as complete
        pending_work.fetch_sub(1);

        {
          // this extra braces is toLimit the Scope of the Lock
          std::lock_guard<std::mutex> lock(worker_mtx);
          worker_cv.notify_all();
        }
    }
}

void fd_search_enhanced(
    const fs::path& start_dir,
    const unique_ptr<RE2>& pattern,
    const std::vector<unique_ptr<RE2>>& gitignore_rules,
    int max_depth = -1,
    int num_threads = std::thread::hardware_concurrency()
) {
    DirQueue dir_queue;
    std::atomic<int> pending_work{1};  // Start with 1 for the initial directory
    std::mutex output_mtx;
    std::mutex worker_mtx;
    std::condition_variable worker_cv;

    // Add initial directory to queue
    dir_queue.push(WorkItem(start_dir, 0));

    // Create worker threads
    std::vector<std::thread> workers;
    workers.reserve(num_threads);

    for (int i = 0; i < num_threads; ++i) {
        workers.emplace_back(worker,
            std::ref(dir_queue),
            std::ref(pattern),
            std::ref(gitignore_rules),
            max_depth,
            std::ref(pending_work),
            std::ref(output_mtx),
            std::ref(worker_cv),
            std::ref(worker_mtx)
        );
    }

    {
      // must keep in a {}, to unlock worker_mtx, allowing worker thread joinning to finish
      // in the end of worker thread, there's acquiring worker_mtx statement
        std::unique_lock<std::mutex> lock(worker_mtx);
        worker_cv.wait(lock, [&] {
          // cout << "\nwait" << pending_work.load() << " " << dir_queue.empty() << "\n";
          return pending_work.load() == 0 && dir_queue.empty();
        });
    }
    // cout << "set_finished" << '\n';
    // introduce worker_cv and worker_mtx. It’s more efficient and idiomatic than polling/sleeping.
    // Monitor completion - when no work is pending and queue is empty
    // while (pending_work.load() > 0 || !dir_queue.empty()) {
    //     std::this_thread::sleep_for(std::chrono::milliseconds(10));
    // }

    // Signal completion and wait for workers
    dir_queue.set_finished();
    for (auto& worker : workers) {
        worker.join();
    }
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

    // Compile regex
    RE2::Options options;
    options.set_case_sensitive(case_sensitive);

    auto pattern = make_unique<RE2>(pattern_str, options);
    if(!pattern->ok()){
      std::cerr << "Invalid regex pattern: " << pattern->error() << std::endl;
      return 1;
    }

    // Load .gitignore rules
    std::vector<unique_ptr<RE2>> gitignore_rules = load_gitignore_rules(dir);

    // Perform search with timing
    auto start_time = std::chrono::high_resolution_clock::now();

    fd_search_enhanced(dir, pattern, gitignore_rules, max_depth, num_threads);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cerr << "Search completed in " << duration.count() << " ms using " << num_threads << " threads" << std::endl;

    std::cout << g_count << '\n';
    return 0;
}
