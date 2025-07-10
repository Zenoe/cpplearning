
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <re2/re2.h>
#include "tool.h"
#include "gutils.h"
#include "zfs.h"

// #include "gutils.h"
namespace fs = std::filesystem;

using std::cout;

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



// Remove BOM if present at the beginning of the line
std::string remove_bom(const std::string &line) {
  static const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
  if (line.size() >= 3 && (unsigned char)line[0] == bom[0] &&
      (unsigned char)line[1] == bom[1] && (unsigned char)line[2] == bom[2]) {
    return line.substr(3);
  }
  return line;
}

void copyRealPassScript(const std::vector<std::string>& ignoreDirs){
  std::vector<fs::path> logDateDirs;
  logDateDirs.reserve(32);

  std::string logDir = "/root/work/script/log";
  try{
    for(auto const &entry: fs::directory_iterator(logDir)){
      // cout << entry.path().string() << "   " << entry.path().string() << "\n";
      if(entry.is_directory() && !gutils::vector_contains(ignoreDirs, entry.path().filename().string()) ){
        // cout << "ok " << entry << "\n";
        logDateDirs.push_back(entry);
      }
    }
    for(const auto& dir : logDateDirs){
      for(auto const &entry: fs::recursive_directory_iterator(dir)){
        // const auto& logfile = entry.path().filename().string();
        auto logfile = entry.path().filename().string();
        if(gutils::starts_with(logfile, "RG-") && gutils::ends_with(logfile, ".txt")){
          std::size_t pos = logfile.find('(');
          if(pos != std::string::npos){
            std::string_view caseId(logfile.data(), pos);
            // std::cout << caseId << "\n";
            if(auto found_path = find_file("/root/work/script", std::string(caseId) + ".txt" ))
              std::cout << *found_path << '\n';
            }
        }
      }
    }
  }catch(...){

  }
}

void copyScript(){
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
    return ;
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
    // bool found = false;
    for (const auto &d : target_dirs) {
      if (auto found_path = find_file(d, line)) {
        fs::path fifth_part = found_path->parent_path().filename();
        fs::path fourth_part = found_path->parent_path().parent_path().filename();
        try {
          fs::path sub_dir = fs::path(out_dir) / fourth_part / fifth_part;
          if(!fs::exists(sub_dir)){
            fs::create_directories(sub_dir);
          }
          fs::path dest = sub_dir / found_path->filename();
          fs::copy_file(*found_path, dest, fs::copy_options::overwrite_existing);
          std::cout << "Copied: " << *found_path << " -> " << dest << std::endl;
        } catch (std::exception &e) {
          std::cerr << "Copy error: " << e.what() << std::endl;
        }
        // found = true;
        // break;
      }
    }
    // if (found) {
    //   try {
    //     fs::path dest = fs::path(out_dir) / found_path.filename();
    //     fs::copy_file(found_path, dest, fs::copy_options::overwrite_existing);
    //     std::cout << "Copied: " << found_path << " -> " << dest << std::endl;
    //   } catch (std::exception &e) {
    //     std::cerr << "Copy error: " << e.what() << std::endl;
    //   }
    // } else {
    //   std::cout << "Not found: " << line << std::endl;
    // }
  }
}
int main() {
  // copyScript();
  std::vector<std::string> ignoredDirs{"其他日志"};
  copyRealPassScript(ignoredDirs);
  return 0;
}
