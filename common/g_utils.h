// glob_utils.h
#pragma once
#include <string>
#include <string_view>

namespace gutils {
  std::string glob_to_regex(std::string_view glob);
}
