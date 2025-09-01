#include "tool.h"
#include "pt.h"
#include <iostream>
#include <limits>
#include <vector>
#include <algorithm>  // min_element
#include <memory>
#include "Counter.h"
#include "fraction.h"
#include "widget.h"
#include <functional>
#include "ClipItem.h"

using ivec = std::vector<int>;
void printp2dv(const std::vector<p2d> &p2dv) {
  for (const auto i : p2dv) {
    std::cout << "(" << i.x << "," << i.y << ")";
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
  ivec v{1, 24, -1, 6, 9};
  // distance returns the size of an iterator range
  print("distance", distance(v.begin(), v.end()));
  auto argmin = distance(begin(v), min_element(begin(v),end(v)) );
  print("argmin", argmin);  // 2
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
  i2 = v3.insert(begin(v3), {77, 88, 99}); // i2 pt to 77
  printVectorT(v3);
  std::cout << "i2: " << *i2 << "\n";
  i2 = v3.insert(end(v3) - 1, 22);
  std::cout << "i2: " << *i2 << " " << *(i2 + 1) << "\n";
  printVectorT(v3);

  std::cout << "------------erase" << '\n';
  std::vector<int> v4{0, 1, 2, 3, 5, 6};
  printVectorT(v4);
  auto i4 = v4.erase(begin(v4));
  std::cout << *i4 << '\n';
  i4 = v4.erase(begin(v4) + 1);
  std::cout << *i4 << '\n';
  std::cout << "------------emplace" << '\n';
  // emplace, make a new element with forwarded counstructor args
  std::vector<p2d> p2dv{p2d(1, 2)};
  p2dv.emplace_back(9, 7);
  p2dv.emplace(begin(p2dv), 88, 88);
  for (auto i : p2dv) {
    std::cout << "(" << i.x << " " << i.y << ")";
  }
  print("\n");

  print("-----------swap");
  std::vector<int> v5{0, 1, 2, 3, 5, 6};
  std::vector<int> v6{70, 71, 72, 73};
  std::swap(v5, v6);
  printVectorT(v5);
  printVectorT(v6);

  print("-----------value initialization");
  std::vector<int> v7(4);
  print(v7.size(), v7.capacity()); // 4, 4
  printVectorT(v7);                // all initialize to 0
  std::vector<p2d> v8(
      2); // call default ctor to initialize all elements in vector
  print("-----------p2d");
  printp2dv(v8);
}

void testEnum() {
  // prefer scoped enum(with class)
  enum class week { mon, tue, wed, thu, fri, sat, sun };
  int a = static_cast<int>(week::mon);
  print(a);
  // int i = 3;
  // week w1 = static_cast<week> (i);
}

void testString() {
  std::string path =
      R"(C:\Users\Name\Documents\file.txt)"; // No need to escape backslashes
  print(path);
  std::string poem = R"(Roses are red,
Violets are blue,
Raw strings are great,
And so are you.)";
  print(poem);
  // custom delimiters, to preserve ( ) or other special sequence in string
  std::string complex = R"xyz(Text with )" inside)xyz";
  print(complex);

  std::string ss = "abcde";
  std::string_view sv(ss.data(), ss.size()-1);
  print(sv);
}

void foo(Counter const &c) {
  print("in foo");
  c.print();
}

void testPtr(){
  // c++11, no make_unique
  // std::unique_ptr<Counter> pct(new Counter(9,"counter1"));
  // or brace
  std::unique_ptr<Counter> pct(new Counter{9,"counter1"});
  pct->print();

  auto pct2 = std::make_unique<Counter>(99, "counter2");
  pct2->print();

  auto ptr = std::make_unique<int>(42);
  std::cout << *ptr << std::endl;
  *ptr = 420;
  print(*ptr);

  // auto p2 = ptr;  unique_ptr is not copyable
  auto p3 = std::move(ptr);  // unique_ptr is moveable
  print(*p3);
  // print(*ptr);  // ptr was moved, this cause segmentation fault
}

void testCls() {
  Counter ct1{2};
  ct1.print();
  // if the ctor defined in Counter is explicit, the following statement trigger error
  // no implicit conversion
  // foo(3);
  foo(Counter(3));

  Counter ct2 = std::move(ct1);
  print("ct2.print");
  ct2.print();
  print("ct1.print");
  ct1.print();
}

void testThrow(){
  try{
    int d = 1;
    std::cin >> d;
    Fraction f(1,d);
  }catch(std::invalid_argument const& e){
    std::cerr <<"error: " << e.what() << '\n';
  }
}

void testWidget(){
  Widget w {{1,2,3}, "aaa"};
  w.print();
  Widget w2(w);
  w2.print();
  // print(w2.v_);
  Widget w3(std::move(w2));
  print("w3");
  w3.print();
  print("w2");
  w2.print();
  Widget w4 = w3;
  print("w4");
  w4.print();
}

void testWidgetOperator(){
  Widget w {{1,2,3}, "aaa"};
  print(w[0]);
  const Widget cw {{9,8,7}, "bbb"};
  print(cw[0]);
  // print((*( new Widget{{19,18,17}, "bbb"} ))[0]);
  Widget w3 {{19,18,17}, "bbb"};
  // float s = std::move(w3[0]);
  float s = std::move(w3)[0];// Explicitly moved object
  print("sss ", s);
  w3.print();
  float y = Widget{{3,2,1}, "tmp"}[0];  // Temporary object, 优先调用 &&operator, 如果没有，会调用const operator
  // Widget{{1,2,3},"x"}[0] is rvalue
  // Widget{{1,2,3},"x"}[0] = 5.0f;  // compile error, lvalue required as left operand of assignment
}

void testRefRef(){
  print("testRefRef");
  const Widget & w0 = Widget{{1,2,3}, "ddd"};
  w0.print();

  Widget && w1 = Widget{{21,22,23}, "ddd"};
  w1.print();

  Widget && w2 = std::move(Widget{{11,12,13}, "xxx"});
  w2.print();
}


void variadicTpl(){
  print("empty arguments for variaic");
}

template<typename T, typename... args>
void variadicTpl(T arg1, args... otherArgs){
  print(arg1);
  variadicTpl(otherArgs...);
}
void print_sum(int a, int b) { print(a + b); }

void printab(int a, int b){
  print("a: ", a, "b: ", b);
}

void testBind(){
  auto bound_print = std::bind(print_sum, 10 ,std::placeholders::_1);
  bound_print(5);

// In modern C++ (C++11 and later), lambdas are often preferred over std::bind because:
// They're more readable
// They have better performance in many cases
// They're more flexible
  auto lambda_print = [](int b){print_sum(5, b);};
  lambda_print(10);

  std::function<void(int, int)> swapab = std::bind(printab, std::placeholders::_2, std::placeholders::_1 );
  swapab(1,2);

  MyClass obj;
  auto bound_pt999 = std::bind(&MyClass::print,&obj,std::placeholders::_1);
  bound_pt999(33);

  // Binding with Smart Pointers
  auto bound_shared = std::bind(&MyClass::print, std::make_unique<MyClass>(),std::placeholders::_1);
  bound_shared(8888);

}

void testCtor(){
  print("testCtor");
  ClipItem item;
  std::vector<bool> highlight_mask(10, false);
  DisplayClipItem dci(item, std::move(highlight_mask));
  print(dci.id);

}
int main() {
  // testlimit();
  // testvector();
  // testEnum();
  // testString();
  testCtor();
  // testCls();
  // testPtr();
  // testThrow();
  // testWidget();
  // testWidgetOperator();
  // testRefRef();
  // variadicTpl("hello", "there", "not", "here");
  // testBind();
}
