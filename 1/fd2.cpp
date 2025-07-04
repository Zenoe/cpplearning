#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <memory>
#include <re2/re2.h>

namespace fs = std::filesystem;
using std::unique_ptr;
using std::make_unique;

// std::string glob_to_regex(const std::string& glob) {
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
        // fixme explicitly move or not
        rules.emplace_back(std::move(rule));
    }
    return rules;
}

// Check if a path matches any .gitignore rule
bool is_ignored(const fs::path& path, const std::vector<unique_ptr<RE2>>& rules) {
    // std::string path_str = path.native();  // native return diff type on diff platform
    // std::string path_str = path.string();  // more portable way
    // for (const auto& rule : rules) {
    //     if (std::regex_match(path_str, rule)) {
    //         return true;
    //     }
    // }
    std::string_view path_view = path.string();
    for (const auto& rule : rules) {
        // std::regex_match don't accept string_view, this is trivial improvement on performance
        if (RE2::PartialMatch(path_view, *rule)) {
            return true;
        }
    }
    return false;
}

// Recursive directory search with regex matching
//
void fd_search(
    const fs::path& dir,
    // const std::regex& pattern,
    const unique_ptr<RE2>& pattern,
    const std::vector<unique_ptr<RE2>>& gitignore_rules,
    int max_depth = -1,
    int current_depth = 0
) {
    try {
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (is_ignored(entry.path(), gitignore_rules)) {
                continue;
            }

            // Check if the filename matches the pattern
            std::string filename = entry.path().filename().string();
            if (RE2::PartialMatch(filename, *pattern)) {
                std::cout << entry.path().string() << std::endl;
            }

            // Recurse into subdirectories
            if (entry.is_directory() && (max_depth == -1 || current_depth < max_depth)) {
                fd_search(entry.path(), pattern, gitignore_rules, max_depth, current_depth + 1);
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error accessing " << dir << ": " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <pattern> [directory] [--case-sensitive] [--max-depth N]\n";
        return 1;
    }

    // Parse arguments
    // std::string pattern_str = argv[1];
    std::string pattern_str = glob_to_regex(argv[1]);
    fs::path dir = (argc > 2) ? argv[2] : ".";
    bool case_sensitive = (argc > 3 && std::string(argv[3]) == "--case-sensitive");
    int max_depth = -1;

    if (argc > 4 && std::string(argv[3]) == "--max-depth") {
        max_depth = std::stoi(argv[4]);
    }

    // Compile regex with case-insensitive flag if needed

    RE2::Options options;
    if(case_sensitive)
      options.set_case_sensitive(case_sensitive);

    auto pattern = make_unique<RE2>(pattern_str, options);
    if(!pattern->ok()){
      std::cerr << "Invalid regex pattern: " << pattern->error() << std::endl;
      return 1;
    }

    // try {
    //     pattern = case_sensitive
    //         ? std::regex(pattern_str) // Case-sensitive
    //         : std::regex(pattern_str, std::regex_constants::icase); // Case-insensitive
    // } catch (const std::regex_error& e) {
    //     std::cerr << "Invalid regex pattern: " << e.what() << std::endl;
    //     return 1;
    // }

    // Load .gitignore rules
    std::vector<unique_ptr<RE2>> gitignore_rules = load_gitignore_rules(dir);

    // Perform search
    fd_search(dir, pattern, gitignore_rules, max_depth);

    return 0;
}
