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
}
