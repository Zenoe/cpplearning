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

  // The ability to access private members of another instance of the same class
  // is what makes the copy constructor work. Private Access Within the Same
  // Class: Private members are accessible by any member function of the same
  // class, regardless of which instance of the class is being accessed. Copy
  // Constructor Purpose: The copy constructor is specifically designed to copy
  // the state of one object into another. To do this, it needs access to
  // private members of the object being copied.
  Widget(Widget &&cw) noexcept
      : v_(std::move(cw.v_)), name_(std::move(cw.name_)) {}

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

  // unqualified
  // Can be called on any object (lvalues, rvalues, temporaries)
  // Returns a reference to internal data
  // Danger: Allows modification of temporaries which is usually unsafe
  // make_obj()[0] = 5 ok
  // float y0 = Widget{{3,2,1}, "tmp"}[0];  ok
  // float &operator[](size_t index) { return v_[index]; }

  // qualified Can only be called on lvalue objects (named variables), cann't be call on rvalues(temporaries)
  // make_obj()[0] = 5 compile error
  // float y0 = Widget{{3,2,1}, "tmp"}[0]; compile error
  // c++11
  // 非const的对象优先调用，如果只定义了const 版本，则会调用const版本
  // eg:  Widget w {{1,2,3}, "aaa"};  w[0]
  float &operator[](size_t index) & {std::cout << "non-const operator  "; return v_[index]; }

  // const 版本
  const float& operator[] (size_t index) const & {std::cout << "const operator "; return v_[index];}

  // Key Points About the && Overload:
  // When it's called:
  // 1.On temporary objects (rvalues)
  // 2.When explicitly moving an object with std::move()
  // 3.When returning a Widget by value from a function and immediately indexing

  // return by value, cause The object is temporary and will be destroyed
  // Returning a reference to internal data would be dangerous
  float operator[](size_t index) && {std::cout << "&&operator "; return v_[index];}

  void print() const{
    std::cout << "widget :" << ' ' << v_.capacity() << ' ' << name_ << '\n';
    for (auto x : v_) {
      std::cout << x << ' ';
    }
    std::cout << '\n';
  }

  // string getName() const { return name_; }
  // vec getVec() const { return v_; }

  // return const reference to avoid deep copying
  const string &getName() const { return name_; }

  // const vec &getVec() const { return v_; } // work on both lvalue and rvalue
  const vec &getVec() const & { return v_; } // work on only lvalue
  const vec getVec() const && {return std::move(v_);}


  // no sure. the comment is copy from AI. and I saw the grammar from hackingcpp website
  // xvalue: expiring value
  // It returns an xvalue reference to the member that lives inside *this.
  // The returned reference is valid only until the owning object (the
  // temporary you called the function on) is destroyed at the end of the full
  // expression. Hold on to it any longer and you have a dangling reference.
  // vec&& getVec() && {return std::move(v_);} // dangerous?

private:
  vec v_;
  // string name_ = "";
  string name_; // Initialized to empty string by default
};

#endif // WIDGET_H_
