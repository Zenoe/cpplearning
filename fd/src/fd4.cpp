// multithreading by claude
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

namespace fs = std::filesystem;
using std::unique_ptr;
using std::make_unique;
using std::cout;

int g_count = 0;
std::string glob_to_regex(std::string_view glob) {
    std::string regex_str;
    for (char c : glob) {
        switch (c) {
            case '*': regex_str += ".*"; break;
            case '?': regex_str += '.';  break;
            case '.': regex_str += "\\."; break; // Escape regex special chars
            default:  regex_str += c;
        }
    }
    return regex_str;
}

// Parse .gitignore rules (simplified)
std::vector<unique_ptr<RE2>> load_gitignore_rules(const fs::path& dir) {
    std::vector<unique_ptr<RE2>> rules;
    std::ifstream gitignore(dir / ".gitignore");
    if (!gitignore) return rules;

    std::string line;
    while (std::getline(gitignore, line)) {
        if (line.empty() || line.find("#") == 0) continue;
        // Convert glob to regex (simplified)
        std::string regex_str = glob_to_regex(line);
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

// Thread pool for directory processing
class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable cv;
    std::atomic<bool> stop;

public:
    ThreadPool(size_t num_threads = std::thread::hardware_concurrency()) : stop(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        cv.wait(lock, [this] { return stop || !tasks.empty(); });

                        if (stop && tasks.empty()) return;

                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    template<typename F>
    void enqueue(F&& f) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            if (stop) return;
            tasks.emplace(std::forward<F>(f));
        }
        cv.notify_one();
    }

    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            stop = true;
        }
        cv.notify_all();
        for (std::thread& worker : workers) {
            worker.join();
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
    active_tasks++;


    // {

    // std::lock_guard<std::mutex> lock(output_mtx);
    // std::cout << "try " << dir.string() << "\n";
    // }
    try {
        std::vector<fs::path> subdirs;

        // Process current directory
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (is_ignored(entry.path(), gitignore_rules)) {
                continue;
            }

            // Check if the filename matches the pattern
            std::string filename = entry.path().filename().string();
            // cout << "trying " << entry.path().string() << "\n";
            if (RE2::PartialMatch(filename, *pattern)) {
              std::lock_guard<std::mutex> lock(output_mtx);
              cout << "match: " << entry.path().string() << "\n";
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
            pool.enqueue([&, subdir]() {
                fd_search_threaded(subdir, pattern, gitignore_rules, collector, pool, active_tasks, output_mtx, max_depth, current_depth + 1);
            });
        }

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error accessing " << dir << ": " << e.what() << std::endl;
    }

    active_tasks--;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <pattern> [directory] [--case-sensitive] [--max-depth N] [--threads N]\n";
        return 1;
    }

    // Parse arguments
    std::string pattern_str = glob_to_regex(argv[1]);
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

    // Load .gitignore rules
    std::vector<unique_ptr<RE2>> gitignore_rules = load_gitignore_rules(dir);

    // Create thread pool and result collector
    ThreadPool pool(num_threads);
    ResultCollector collector;
    std::atomic<int> active_tasks(0);

    std::mutex output_mtx;
    // Start the search
    auto start_time = std::chrono::high_resolution_clock::now();

    // Use the thread pool approach for better resource management
    fd_search_threaded(dir, pattern, gitignore_rules, collector, pool, active_tasks, output_mtx, max_depth);

    // Wait for all tasks to complete
    while (active_tasks > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Print results
    collector.print_results();

    std::cerr << "Search completed in " << duration.count() << " ms using " << num_threads << " threads" << std::endl;

    std::cout << g_count << '\n';
    return 0;
}
