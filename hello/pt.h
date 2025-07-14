#ifndef PT_H_
#define PT_H_
#include <iostream>

struct p2d {
  p2d() : x(-1), y(-1) {}
  p2d(int _x, int _y) : x(_x), y(_y) {}
  int x;
  int y;
};

class MyClass{
  public:
  void print(int x){
    std::cout << x << '\n';
  }
};
#endif // PT_H_
