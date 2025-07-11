#include <chrono>
#include "gutils.h"

namespace gutils {
std::string glob_to_regex(std::string_view glob) {
    std::string regex_str;
    for (char c : glob) {
        switch (c) {
            case '*': regex_str += ".*"; break;
            case '?': regex_str += '.';  break;
            case '.': regex_str += "\\."; break;
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

}
