#include <iostream>
#include <memory>
#include <algorithm>
#include <chrono>

template<typename T>
class MyVector{
private:
  T* data;
  size_t size_;
  size_t capacity_;

  static constexpr double GROWTH_FACTOR = 1.5;

public:
  MyVector(): data(nullptr), size_(0), capacity_(0){}
  ~MyVector(){
    clear();
    deallocate();
  }

  MyVector(const MyVector& other): size_(other.size_), capacity_(other.capacity_){
    data = allocate(capacity_);
    // std::copy 有类型检查，调用assignment/copy ctor
    std::copy(other.data, other.data + size_, data);
    // 如果T是简单类型才可以用 memcpy 拷贝, 速度更快
    // memcpy(data, other.data, size_ * sizeof(T)); // Assuming T is trivially copyable
  }

  MyVector(MyVector&& other) noexcept : data(other.data), size_(other.size_), capacity_(other.capacity_){
    other.data = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
  }

  MyVector& operator = (const MyVector& other){
    if(this != &other){
      delete[] data;
      size_ = other.size_;
      capacity_ = other.capacity_;
      data = new T[other.capacity_];
      std::copy(other.data, other.data+size_, data);
      // T* tmp = std::malloc(sizeof(T)* other.capacity_);
      // std::copy(other.data, other.data+size_, tmp);
      // this.data = tmp;
    }
    return *this;
  }

  // better
  // MyVector& operator = (const MyVector& other){
  //   if(this != &other){
  //     MyVector tmp(other);
  //     swap(tmp);
  //     // 离开作用域后tmp自动析构
  //   }
  //   return *this;
  // }
  // 赋值构造
  MyVector& operator = (MyVector&& other) noexcept{
    if(this != &other){
      clear();
      deallocate();
      data = other.data;
      size_ = other.size_;
      capacity_ = other.capacity_;
      other.data = nullptr;
      other.size_ = 0;
      other.capacity_ = 0;
    }
    return *this;
  }

  void push_back(const T& value){
    if(size_ == capacity_){
      reallocate();
    }
    new(data+size_) T(value);
    size_ ++;
    // data[size_++] = value;
  }

  // no const before T
  void push_back(T&& value){
    if(size_ == capacity_){
      reallocate();
    }
    new(data+size_) T(std::move(value));
    size_ ++;
  }
  template<typename... Args>
  void emplace_back(Args&&... args){
    if(size_ == capacity_){
      reallocate();
    }
    // must have <Args>, or error: no matching function for call to ‘forward(int&)’
    new(data+size_) T(std::forward<Args>(args)...);
    ++size_;
  }

  void reserve(size_t new_capacity){
    if(new_capacity > capacity_)
      reallocate(new_capacity);
  }

  void resize(size_t new_size, const T& value){
    if(new_size > capacity_){
      reserve(new_size);
    }
    if(new_size > size_){
      for(size_t i = size_; i < new_size; i++){
        data[i] = value;
      }
    }else if(new_size < size_){
      for(size_t i = new_size; i < size_; i++){
        data[i].~T();
      }
    }
    size_ = new_size;
  }

  void pop_back(){
    if(size_ > 0){
      size_--;
      data[size_-1].~T();
    }
  }
  void clear(){
    for(size_t i=0; i < size_; i++){
      data[i].~T();
    }
    size_ = 0;
  }
  void shrink_to_fit(){
    if(size_ < capacity_){
      reallocate(size_);
    }
  }

  T& operator[](size_t idx){return data[idx];}
  const T& operator[](size_t idx) const { return data[idx];}
  T& at(size_t idx){
    if(idx > size_) throw std::out_of_range("index out of range");
    return data[idx];
  }
  size_t size() const { return size_; }
  size_t capacity() const { return capacity_; }
  bool empty() const { return size_ == 0; }

  T* begin(){return data;}
  T* end(){return data+size_;}
  const T* begin() const {return data;}
  const T* end() const {return data+size_;}
// 添加反向迭代器
T* rbegin() { return data + size_ - 1; }
T* rend() { return data - 1; }
const T* rbegin() const { return data + size_ - 1; }
const T* rend() const { return data - 1; }

  void swap(MyVector& other) noexcept {
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
    std::swap(data, other.data);
  }

private:
  T* allocate(size_t count){
    if(count == 0) return nullptr;
    return static_cast<T*> ( ::operator new(count*sizeof(T)) );
    // 使用 malloc 的情况：
    // 只存储基本类型（int, double, char 等）
    // 教育目的，代码简单易懂
    // 对性能要求极高，且确定类型没有特殊对齐需求

    // 使用 operator new 的情况：
    // 存储有对齐要求的类型
    // 需要更好的 C++ 标准一致性
    // 生产环境代码
    // 存储自定义类型时
    // return static_cast<T*> (std::malloc(count*sizeof(T)));
  }

  void reallocate(size_t new_capacity = 0){
    if(new_capacity == 0){
      // capacity_ * GROWTH_FACTOR, GROWTH_FACTOR=1.5, capacity_ = 1 时，会导致这里无法增长
      new_capacity = capacity_ == 0 ? 1 : static_cast<size_t> (capacity_ * GROWTH_FACTOR);
      if(new_capacity <= capacity_) new_capacity = capacity_ + 1;
    }

    T* new_data = allocate(new_capacity);
    if(data){
      for(size_t i = 0; i < size_; i++){
        if constexpr (std::is_move_constructible_v<T>){
          // new_data[i] = std::move(data[i]); 内存不是new 的，而是placement new, 不能这样赋值
          new(new_data + i) T(std::move(data[i]));
        }else{
          new(new_data + i) T(data[i]);
        }
      }
    }
    deallocate();
    data = new_data;
    capacity_ = new_capacity;
  }
  void deallocate(){
    if(data){
      // std::free(data);
      ::operator delete(data);
      data = nullptr;
    }
  }

};



void placementNewHello(){
  void* raw_memory = operator new(sizeof(std::string));

    // Construct the string object in raw_memory using placement new
    std::string* hello = new(raw_memory) std::string("Hello, World!");

    // Use it
    std::cout << *hello << std::endl;

    // Manually call the destructor
    hello->~basic_string();

    // Release the raw memory
    operator delete(raw_memory);
}


int main(){
  // placementNewHello();
  MyVector<int> vec;

    // 测试 push_back 和扩容
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i);
    }

    vec.push_back(1);  // 1 是右值 ,会调用 push_back(T&& )
    // 测试 emplace_back
    // vec.emplace_back(100);
    // vec.emplace_back(110);

    // 测试迭代
    for (auto& val : vec) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    // 测试容量管理
    std::cout << "Size: " << vec.size() << ", Capacity: " << vec.capacity() << std::endl;

    vec.shrink_to_fit();
    std::cout << "After shrink_to_fit - Size: " << vec.size()
              << ", Capacity: " << vec.capacity() << std::endl;

    return 0;
}
