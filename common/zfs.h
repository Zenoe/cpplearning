#ifndef ZFS_H_
#define ZFS_H_
#include <string>
#include <filesystem>
#include <optional>
#include <fstream>
#include <vector>
#include <set>
#include <string>
#include <optional>

namespace fs = std::filesystem;

// Recursively search filename in directory
std::optional<fs::path> find_file(const fs::path &dir, const std::string &fname, bool fuzzy = false);

bool write_lines_to_file(const std::vector<std::string_view>& lines, const std::string& filename, bool append=false);
std::optional<std::set<std::string>> read_file_to_set(const std::string& filename);
#endif // ZFS_H_
