
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include "tool.h"

// #include "gutils.h"
namespace fs = std::filesystem;

// Helper to get YYYY-MM-DD string
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

// Recursively search filename in directory
bool find_file(const fs::path &dir, const std::string &fname,
               fs::path &result) {
  // std::cout << "searching " << fname << '\n';
  try {
    for (auto const &entry : fs::recursive_directory_iterator(dir)) {
      // std::cout << "searching " << fname << " in " << entry << '\n';
      if (entry.is_regular_file() &&
          entry.path().filename().string().find(fname) != std::string::npos) {
        result = entry.path();
        return true;
      }
    }
  } catch (...) {
  }
  return false;
}

// Remove BOM if present at the beginning of the line
std::string remove_bom(const std::string &line) {
  static const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
  if (line.size() >= 3 && (unsigned char)line[0] == bom[0] &&
      (unsigned char)line[1] == bom[1] && (unsigned char)line[2] == bom[2]) {
    return line.substr(3);
  }
  return line;
}


int main() {
  std::string input_txt = "input.txt"; // Each line: file.txt

  std::vector<std::string> target_dirs = read_file_to_vector("dirs.txt");
  // Prepare output directory
  std::string out_dir = "script" + get_today();
  if (!fs::exists(out_dir))
    fs::create_directory(out_dir);

  // Read input lines
  std::ifstream in(input_txt);
  if (!in.is_open()) {
    std::cerr << "Cannot open " << input_txt << std::endl;
    return 1;
  }

  std::string line;
  bool first_line = true;
  while (std::getline(in, line)) {
    if (first_line) {
      line = remove_bom(line);
      first_line = false;
    }
    if (line.empty())
      continue;
    fs::path found_path;
    bool found = false;
    for (const auto &d : target_dirs) {
      if (find_file(d, line, found_path)) {
        found = true;
        break;
      }
    }
    if (found) {
      try {
        fs::path dest = fs::path(out_dir) / found_path.filename();
        fs::copy_file(found_path, dest, fs::copy_options::overwrite_existing);
        std::cout << "Copied: " << found_path << " -> " << dest << std::endl;
      } catch (std::exception &e) {
        std::cerr << "Copy error: " << e.what() << std::endl;
      }
    } else {
      std::cout << "Not found: " << line << std::endl;
    }
  }
  return 0;
}
