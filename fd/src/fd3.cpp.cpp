#include <iostream>
#include <filesystem>
#include <thread>
#include <regex>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <queue>
#include <atomic>
// #include <re2/re2>

using std::cout;
using std::cerr;
using std::string;
using std::unique_ptr;
using std::vector;
using std::deque;
namespace fs = std::filesystem;


void worker(){
  while (true) {

  }
}
void fd_search(const fs::path& dir, const std::regex& pattern, int max_depth=-1, int num_threads = std::thread::hardware_concurrency()){
  vector<std::thread> workers;

  for(int i=0; i < num_threads; i++){
    workers.emplace_back()
  }
}

int main(int argc, char* argv[]){
  if(argc < 2){
    cerr << "shoule have more than 2 params\n";
  }

  std::regex pattern_str = std::regex(argv[1]);
  fs::path dir = argc > 2 ? argv[2] : ".";

  int num_threads = std::thread::hardware_concurrency();
  int max_depth = -1;
  fd_search(dir, pattern_str, max_depth, num_threads);
  return 0;
}
