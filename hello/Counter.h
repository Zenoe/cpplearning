#ifndef COUNTER_H_
#define COUNTER_H_
#include <iostream>
#include <utility>

#include <cstring> // For std::strlen, std::strcpy
class Counter {
  int count_ = 0;
  std::string name_="tom";

public:
    // primary constructor
    explicit Counter(int initial, std::string name):count_{initial}, name_{std::move(name)}{}
    // Counter(int initial) : count_{initial} {}
    // delegated ctor, Reuses the primary constructor
    explicit Counter(int initial) : Counter(initial, "tom"){}
    // // Optionally, add a constructor for const char* (avoids temporary string)
    explicit Counter(int initial, const char* name) : count_{initial}, name_(name){}

  void print() const{
      std::cout<< count_ << ' ' << name_ << std::endl;
  }
};


class CounterCStyle {
    int count_ = 0;
    char* name_ = nullptr; // Use nullptr instead of raw "tom"

public:
    // Constructor (takes ownership of a dynamically allocated string)
    explicit CounterCStyle(int initial, const char* name = "tom")
        : count_{initial} {
        if (name) {
            name_ = new char[std::strlen(name) + 1];
            std::strcpy(name_, name);
        }
    }

    // --- Rule of Five ---
    // 1. Destructor
    ~CounterCStyle() { delete[] name_; }

    // 2. Copy Constructor
    CounterCStyle(const CounterCStyle& other)
        : count_{other.count_} {
        if (other.name_) {
            name_ = new char[std::strlen(other.name_) + 1];
            std::strcpy(name_, other.name_);
        }
    }

    // 3. Copy Assignment
    CounterCStyle& operator=(const CounterCStyle& other) {
        if (this != &other) {
            delete[] name_; // Free existing resource
            count_ = other.count_;
            if (other.name_) {
                name_ = new char[std::strlen(other.name_) + 1];
                std::strcpy(name_, other.name_);
            } else {
                name_ = nullptr;
            }
        }
        return *this;
    }

    // 4. Move Constructor (Steals resources from 'other')
    CounterCStyle(CounterCStyle&& other) noexcept
        : count_{other.count_}, name_{other.name_} {
        other.name_ = nullptr; // Leave 'other' in a valid but empty state
    }

    // 5. Move Assignment (Steals resources from 'other')
    CounterCStyle& operator=(CounterCStyle&& other) noexcept {
        if (this != &other) {
            delete[] name_; // Free existing resource
            count_ = other.count_;
            name_ = other.name_;
            other.name_ = nullptr; // Leave 'other' in a valid but empty state
        }
        return *this;
    }
};

// Key Points for Move Semantics
// Move Constructor (Counter(Counter&& other))
// Steals the name_ pointer from other (instead of deep copying).
// Sets other.name_ = nullptr to prevent double-free.
// Marked noexcept for compatibility with STL containers.
//
// Move Assignment (operator=(Counter&& other))
// Releases the current name_ (if any).
// Steals other.name_ and sets other.name_ = nullptr.
// Also marked noexcept.

// Why noexcept?
// STL containers (e.g., std::vector) use move operations only if they are noexcept.
// Ensures strong exception safety.
#endif // COUNTER_H_
