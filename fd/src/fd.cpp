#include <iostream>
#include <filesystem>
#include <regex>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

namespace fs = std::filesystem;

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
std::vector<std::regex> load_gitignore_rules(const fs::path& dir) {
    std::vector<std::regex> rules;
    std::ifstream gitignore(dir / ".gitignore");
    if (!gitignore) return rules;

    std::string line;
    while (std::getline(gitignore, line)) {
        if (line.empty() || line.find("#") == 0) continue;
        // Convert glob to regex (simplified)
        std::string regex_str;
        for (char c : line) {
            if (c == '*') regex_str += ".*";
            else if (c == '?') regex_str += ".";
            else regex_str += c;
        }
        rules.emplace_back(regex_str);
    }
    return rules;
}

// Check if a path matches any .gitignore rule
bool is_ignored(const fs::path& path, const std::vector<std::regex>& rules) {
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
        if (std::regex_match(path_view.begin(), path_view.end(), rule)) {
            return true;
        }
    }
    return false;
}

// Recursive directory search with regex matching
//
void fd_search(
    const fs::path& dir,
    const std::regex& pattern,
    const std::vector<std::regex>& gitignore_rules,
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
            if (std::regex_search(filename, pattern)) {
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
    std::regex pattern;
    try {
        pattern = case_sensitive
            ? std::regex(pattern_str) // Case-sensitive
            : std::regex(pattern_str, std::regex_constants::icase); // Case-insensitive
    } catch (const std::regex_error& e) {
        std::cerr << "Invalid regex pattern: " << e.what() << std::endl;
        return 1;
    }

    // Load .gitignore rules
    std::vector<std::regex> gitignore_rules = load_gitignore_rules(dir);

    // Perform search
    fd_search(dir, pattern, gitignore_rules, max_depth);

    return 0;
}
