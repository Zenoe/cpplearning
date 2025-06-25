#ifndef WIDGET_H_
#define WIDGET_H_

#include <string>
#include <vector>

using vec = std::vector<float>;
using string = std::string;
class Widget {
public:
  /* Widget() : v_{0} {} */ // initialize v_ with single element 0, capacity 0

  // Widget(const Widget &cw) : v_(cw.getVec()), name_{cw.getName()} {}
  Widget(const Widget &cw) : v_(cw.v_), name_{cw.name_} {}
  // reserve(64) → Capacity = 64, Size = 0 (no elements, just reserved memory)
  // v_(64) → Capacity ≥ 64, Size = 64 (all elements initialized to 0.0f)
  // Widget() : v_(64) {}
  Widget() { v_.reserve(64); }

  // The ability to access private members of another instance of the same class is what makes the copy constructor work.
  // Private Access Within the Same Class: Private members are accessible by any member function of the same class, regardless of which instance of the class is being accessed.
  // Copy Constructor Purpose: The copy constructor is specifically designed to copy the state of one object into another. To do this, it needs access to private members of the object being copied.
  Widget(Widget &&cw) noexcept : v_(std::move(cw.v_)), name_(std::move(cw.name_)) {}

  Widget(vec &&v, string &&n) noexcept {
    v_ = std::move(v);
    name_ = std::move(n);
  }

  Widget &operator=(const Widget &other) {
    if (this != &other) {
      v_ = other.getVec();
      name_ = other.getName();
    }
    return *this;
  }

  Widget &operator=(const Widget &&other) noexcept {
    if (this != &other) {
      v_ = std::move(other.getVec());
      name_ = std::move(other.getName());
    }
    return *this;
  }
  ~Widget() = default;

  void print() {
    std::cout << "widget :" << ' ' << v_.capacity() << ' ' << name_ << '\n';
    for (auto x : v_) {
      std::cout << x << ' ';
    }
    std::cout << '\n';
  }

  // string getName() const { return name_; }
  // vec getVec() const { return v_; }

    // return const reference to avoid deep copying
  const string& getName() const { return name_; }
  const vec& getVec() const { return v_; }
private:
  vec v_;
  // string name_ = "";
  string name_; // Initialized to empty string by default
};

#endif // WIDGET_H_
