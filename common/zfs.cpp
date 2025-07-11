#include "zfs.h"
#include <re2/re2.h>
#include <iostream>
// Recursively search filename in directory
// bool find_file(const fs::path &dir, const std::string &fname,
//                fs::path &result, bool fuzzy ) {
//   // std::cout << "searching " << fname << '\n';
//   try {
//     for (auto const &entry : fs::recursive_directory_iterator(dir)) {
//       // std::cout << "searching " << fname << " in " << entry << '\n';
//       if(entry.is_regular_file()){
//         if (fuzzy) {
//           std::unique_ptr<RE2> pattern = std::make_unique<RE2>(fname);
//           if (RE2::PartialMatch(entry.path().filename().string(), *pattern)) {
//             result = entry.path();
//             return true;
//           }
//         } else if (entry.path().filename().string().find(fname) !=
//                    std::string::npos) {
//           result = entry.path();
//           return true;
//         }
//       }
//     }
//   } catch (...) {
//   }
//   return false;
// }


std::optional<fs::path> find_file(const fs::path &dir, const std::string &fname, bool fuzzy ) {
  try {
    for (auto const &entry : fs::recursive_directory_iterator(dir)) {
      // std::cout << "searching " << fname << " in " << entry << '\n';
      if(entry.is_regular_file()){
        if (fuzzy) {
          std::unique_ptr<RE2> pattern = std::make_unique<RE2>(fname);
          if (RE2::PartialMatch(entry.path().filename().string(), *pattern)) {
            return entry.path();
          }
        } else if (entry.path().filename().string().find(fname) !=
                   std::string::npos) {
          return entry.path();
        }
      }
    }
  } catch (...) {

  }
  return std::nullopt;

}

bool write_lines_to_file(const std::vector<std::string_view>& lines,
                         const std::string& filename,
                         bool append)
{
    std::filesystem::path filePath(filename);

    // std::error_code for non-throwing <filesystem> errors (as recommended for robust applications)
    std::error_code ec_dir;
    if (!std::filesystem::create_directories(filePath.parent_path(), ec_dir) && ec_dir) {
        std::cerr << "Error: Failed to create directory '"
                  << filePath.parent_path() << "': " << ec_dir.message() << '\n';
        return false;
    }

    if (!append && std::filesystem::exists(filePath)) {
        std::error_code ec_rm;
        if (!std::filesystem::remove(filePath, ec_rm)) {
            std::cerr << "Error: Failed to remove file '"
                      << filename << "': " << ec_rm.message() << '\n';
            return false;
        }
    }

    std::ios::openmode mode = std::ios::out;
    if (append) mode |= std::ios::app;

    std::ofstream ofs(filename, mode);
    if (!ofs) {
        std::cerr << "Error: Failed to open file '" << filename << "' for writing.\n";
        return false;
    }

    for (std::string_view line : lines) {
        ofs << line << '\n';
        if (!ofs) {
            std::cerr << "Error: Failed to write to file '" << filename << "'.\n";
            return false;
        }
    }

    return true;
}
