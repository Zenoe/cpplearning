// #include "hello.h"
#include <iostream>
#include <limits>
#include <vector>
#include "pt.h"
#include "../tool.h"

using ivec = std::vector<int>;
void printp2dv(const std::vector<p2d> &p2dv){
  for(const auto i : p2dv){
    std::cout << "(" << i.x << "," << i.y  << ")";
  }
  print();
}

void testlimit() {
  // smallest negative value:
  std::cout << "lowest:  " << std::numeric_limits<double>::lowest() << '\n';
  // float/double: smallest value > 0
  // integers: smallest value
  std::cout << "min:     " << std::numeric_limits<double>::min() << '\n';
  // largest positive value:
  std::cout << "max:     " << std::numeric_limits<double>::max() << '\n';
  // smallest difference btw. 1 and next value:
  std::cout << "epsilon: " << std::numeric_limits<double>::epsilon() << '\n';
}

void printVector(const std::vector<int> &v) {
  for (auto x : v) {
    std::cout << x << ' ';
  }
  std::cout << '\n';
}

template <typename T> void printVectorT(const std::vector<T> &v) {
  for (const auto &x : v) {
    std::cout << x << ' ';
  }
  std::cout << '\n';

  // std::cout << "iterate through iterator \n";
  // for(auto i = begin(v); i!= end(v); ++i){
  //   std::cout << *i << ' ';
  // }
  // std::cout << '\n';
}

void testvector() {
  ivec v{1, 24, 6, 9};
  for (auto i = rbegin(v); i != rend(v); ++i) {
    std::cout << *i << ' ';
  }
  std::cout << "\n";
  std::vector<int>::iterator endIt = end(v);
  std::cout << "end it " << *(--endIt) << "\n";

  auto it = begin(v);

  // assign by itrator

  *(it + 1) = 3;
  v.push_back(999);
  printVectorT(v);
  v.resize(10, 1);
  printVectorT(v);
  std::vector<int> v2 = v;
  v[2] = 888;
  printVectorT(v);
  printVectorT(v2);

  std::vector<int> v3{0, 1, 2, 3, 5, 6};
  auto i2 = begin(v3) + 2;
  std::cout << "i2: " << *i2 << "\n";
  // an iterator pointing to the first of all the newly inserted elements
  i2 = v3.insert(i2, 8); // i2 pt to 8
  printVectorT(v3);
  std::cout << "i2: " << *i2 << "\n";
  i2 = v3.insert(begin(v3), {77,88, 99});  // i2 pt to 77
  printVectorT(v3);
  std::cout << "i2: " << *i2 << "\n";
  i2 = v3.insert(end(v3)-1, 22);
  std::cout << "i2: " << *i2 << " " << *( i2+1 ) <<  "\n";
  printVectorT(v3);

  std::cout << "------------erase" << '\n';
  std::vector<int> v4{0, 1, 2, 3, 5, 6};
  printVectorT(v4);
  auto i4 = v4.erase(begin(v4));
  std::cout << *i4 <<'\n';
  i4 = v4.erase(begin(v4) + 1);
  std::cout << *i4 <<'\n';
  std::cout << "------------emplace" << '\n';
  // emplace, make a new element with forwarded counstructor args
  std::vector<p2d> p2dv {p2d(1,2)};
  p2dv.emplace_back(9,7);
  p2dv.emplace(begin(p2dv), 88,88);
  for(auto i : p2dv){
    std::cout << "(" << i.x << " " << i.y  << ")";
  }
  print("\n");

  print("-----------swap");
  std::vector<int> v5{0, 1, 2, 3, 5, 6};
  std::vector<int> v6{70, 71, 72, 73 };
  std::swap(v5,v6);
  printVectorT(v5);
  printVectorT(v6);

  print("-----------value initialization");
  std::vector<int> v7 (4);
  print(v7.size(), v7.capacity());  // 4, 4
  printVectorT(v7);// all initialize to 0
  std::vector<p2d> v8 (2);  // call default ctor to initialize all elements in vector
  print("-----------p2d");
  printp2dv(v8);
}

void testEnum(){
  // prefer scoped enum(with class)
  enum class week{mon, tue, wed,thu,fri,sat,sun};
  int a = static_cast<int> ( week::mon );
  print(a);
  // int i = 3;
  // week w1 = static_cast<week> (i);
}

void testString(){
  std::string path = R"(C:\Users\Name\Documents\file.txt)";  // No need to escape backslashes
  print(path);
  std::string poem = R"(Roses are red,
Violets are blue,
Raw strings are great,
And so are you.)";
  print(poem);
  // custom delimiters, to preserve ( ) or other special sequence in string
  std::string complex = R"xyz(Text with )" inside)xyz";
  print(complex);
}
int main() {
  std::cout << "hello\n";
  // testlimit();
  // testvector();
  // testEnum();
  testString();
}
