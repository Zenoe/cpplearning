#include <chrono>
#include <iostream>
#include <fstream>

#include "gutils.h"

namespace gutils {

std::string glob_to_regex(std::string_view glob) {
    std::string regex_str;
    for (char c : glob) {
        switch (c) {
        case '*': regex_str += ".*"; break;
        case '?': regex_str += '.';  break;
        case '.': regex_str += "\\."; break; // Escape regex special chars
        case '\\': regex_str += "\\\\"; break;
        case '+': case '^': case '$': case '(': case ')':
        case '{': case '}': case '|': case '[': case ']':
          regex_str += '\\';
          break;
        default:  regex_str += c;
        }
    }
    return regex_str;
}


  // void hello(){
  // }
  bool starts_with(const std::string &str, const std::string &prefix) {
    return str.size() >= prefix.size() &&
           std::equal(prefix.begin(), prefix.end(), str.begin());
  }

  bool ends_with(const std::string &str, const std::string &suffix) {
    return str.size() >= suffix.size() &&
           std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
  }

  std::string_view get_last_third_part(std::string_view s){
    int count = 0;
    size_t pos = s.size();
    while (count < 3) {
        pos = s.rfind('-', pos - 1);
        if (pos == std::string_view::npos) return "";
        ++count;
    }
    size_t next = s.find('-', pos + 1);
    if (next == std::string_view::npos) next = s.size();
    return s.substr(pos + 1, next - pos - 1);
  }

  std::string get_today() {
    auto t = std::chrono::system_clock::now();
    std::time_t t_time = std::chrono::system_clock::to_time_t(t);
    std::tm buf;
#ifdef _WIN32
    localtime_s(&buf, &t_time);
#else
    localtime_r(&t_time, &buf);
#endif
    char date[16];
    std::strftime(date, sizeof(date), "%Y-%m-%d", &buf);
    return date;
  }

  namespace fs = std::filesystem;
  using std::unique_ptr;
  using std::make_unique;
  std::vector<unique_ptr<RE2>> load_gitignore_rules(const fs::path& dir) {
    std::vector<unique_ptr<RE2>> rules;
    std::ifstream gitignore(dir / ".gitignore");
    if (!gitignore) return rules;

    std::string line;
    while (std::getline(gitignore, line)) {
        if (line.empty() || line.find("#") == 0) continue;
        // Convert glob to regex (simplified)
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

}
