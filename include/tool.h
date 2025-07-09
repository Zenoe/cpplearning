#ifndef TOOL_H_
#define TOOL_H_
#include <iostream>
#include <vector>

void print(){
    std:: cout << '\n';
}

template<typename T, typename... Args>
void print(T first, Args... args) {
    std::cout << first << " ";
    print(args...); // Recursively process the remaining arguments
}

std::vector<std::string> read_file_to_vector(const std::string &filename) {
  std::vector<std::string> lines;
  std::ifstream infile(filename);
  if (!infile) {
    std::cerr << "Error opening file: " << filename << std::endl;
    return lines;
  }
  std::string line;
  while (std::getline(infile, line)) {
    if(!line.empty())
      lines.push_back(line);
  }
  return lines;
}

#endif // TOOL_H_
