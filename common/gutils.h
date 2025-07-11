// glob_utils.h
#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>

namespace gutils {
  std::string glob_to_regex(std::string_view glob);
  // void hello();
  template <typename T>
  bool vector_contains(const std::vector<T>& vec, const T& value){
    return std::find(vec.begin(), vec.end(), value) != vec.end();
  }


  // string
  bool starts_with(const std::string& str, const std::string& prefix);
  bool ends_with(const std::string& str, const std::string& suffix);

  std::string_view get_last_third_part(std::string_view s);
std::string get_today();
}
