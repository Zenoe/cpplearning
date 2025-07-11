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
using std::string_view;
using std::string;
using std::vector;

std::string remove_bom(const std::string &line) {
  static const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
  if (line.size() >= 3 && (unsigned char)line[0] == bom[0] &&
      (unsigned char)line[1] == bom[1] && (unsigned char)line[2] == bom[2]) {
    return line.substr(3);
  }
  return line;
}

void copyRealPassScript(){
  std::vector<std::string> ignoredDirs{"其他日志"};
  std::vector<fs::path> logDateDirs;
  logDateDirs.reserve(32);
    std::map<std::string, std::string> log_script_map = {
        {"0419", "pass20250419"},
        {"0506", "pass20250506"},
        {"0508", "pass20250508"},
        {"0513", "pass20250513"},
        {"0521", "pass20250521"},
        {"0526", "pass20250526"},
        {"0527", "pass20250527"},
        {"0528", "pass20250528"},
        {"0529", "pass20250529"},
        {"0530", "pass20250530"},
        {"0602", "pass20250602"},
        {"0605", "pass20250605"},
        {"0609", "pass20250609"},
        {"0612", "pass20250612"},
        {"0615", "pass20250615"},
        {"0618", "pass20250618"},
        {"0619", "pass20250619"},
        {"0621", "pass20250621"},
        {"0622", "pass20250622"},
        {"0623", "pass20250623"},
        {"0624", "pass20250624"},
        {"062102", "pass2026062102"},
        {"allPassed", "allPassed"},
    };
  string logDir = "/root/work/script/log";
  string destDir = "/root/work/script/realPass";
  if(!fs::exists(destDir)){
    fs::create_directory(destDir);
  }
  size_t filecount = 0;
  vector<string> pmsIds;
  try{
    for(auto const &entry: fs::directory_iterator(logDir)){
      // cout << entry.path().string() << "   " << entry.path().string() << "\n";
      if(entry.is_directory() && !gutils::vector_contains(ignoredDirs, entry.path().filename().string()) ){
        // cout << "ok " << entry << "\n";
        logDateDirs.push_back(entry);
      }
    }
    for(const auto& logdir : logDateDirs){
      for(auto const &entry: fs::recursive_directory_iterator(logdir)){
        // const auto& logfile = entry.path().filename().string();
        auto logfile = entry.path().filename().string();
        if(gutils::starts_with(logfile, "RG-") && gutils::ends_with(logfile, ".txt")){
          std::size_t pos = logfile.find('(');
          if(pos != std::string::npos){
            string_view caseId(logfile.data(), pos);
            auto tmp = gutils::get_last_third_part(caseId);
            // cout << tmp << "\n";
            // pmsIds.push_back(gutils::get_last_third_part(caseId));
            pmsIds.push_back(string(tmp));
            // std::cout << caseId << "\n";

            // auto it = log_script_map.find(logdir);
            // if(it != log_script_map.end())
            // fs::path can be implicitly converted to string, but better to call .string() for portability and readability
            if(auto it = log_script_map.find(logdir.filename().string()); it != log_script_map.end())
            {
              string fullPath = "/root/work/script/" + string(it->second);
              if (auto found_path = find_file(fullPath, string(caseId) + ".txt")){
                // cout << *found_path << '\n';
                filecount += 1;

                fs::copy_file(*found_path, destDir / found_path->filename(),
                              fs::copy_options::overwrite_existing);
              }
            }
          }
        }
      }
    }


    vector<string_view> sv(pmsIds.begin(), pmsIds.end());
    // write_lines_to_file(sv, "/root/work/script/combinedIds.txt");
  }catch(std::exception &e){
     std::cerr << "error: " << e.what() << std::endl;
  }
  print("total copied files: ", filecount);
}

void copyScript(){
  std::string input_txt = "input.txt"; // Each line: file.txt
  std::vector<std::string> target_dirs = read_file_to_vector("dirs.txt");
  // Prepare output directory
  std::string out_dir = "script" + gutils::get_today();
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
          fs::path sub_dir = fourth_part == "script" ? fs::path(out_dir) / "重跑pass" : fs::path(out_dir) / fourth_part / fifth_part;
          if(!fs::exists(sub_dir)){
            fs::create_directories(sub_dir);
          }
          fs::path dest = sub_dir / found_path->filename();
          fs::copy_file(*found_path, dest, fs::copy_options::overwrite_existing);
          cout << "Copied: " << *found_path << " -> " << dest << std::endl;
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
  // copyRealPassScript();
  copyScript();
  return 0;
}

