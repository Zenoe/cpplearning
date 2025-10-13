// g++ -std=c++17 -O2 -o memory_pool memory_pool.cpp
#include <cstddef>
#include <cstdlib>
#include <new>
#include <memory>
#include <iostream>
#include <vector>
#include <chrono>  // 添加缺失的头文件
#include <cstdint> // 添加 uintptr_t 支持

// 内存池类模板，用于分配固定大小的对象 T
template <typename T>
class MemoryPool {
public:
    // 使用 alignas 确保内存池本身和其内部结构有合适的对齐（至少与 T 一样）
    struct alignas(alignof(T)) Chunk {
        Chunk* next;
    };

    MemoryPool() = default;
    ~MemoryPool() {
        // 释放所有申请的大内存块
        for (void* block : blocks_) {
            std::free(block);
        }
    }

    // 禁用拷贝和赋值
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    // 分配内存并构造对象
    template <typename... Args>
    T* construct(Args&&... args) {
        T* obj = allocate();
        // 使用 placement new 在已分配的内存上构造对象
        return new (obj) T(std::forward<Args>(args)...);
    }

    // 仅分配内存，不构造对象
    T* allocate() {
        if (!free_list_) {
            // 自由链表为空，需要申请新的内存块
            allocate_block();
        }

        // 从自由链表头部取出一个块
        Chunk* chunk = free_list_;
        free_list_ = free_list_->next;
        ++allocated_count_;

        // 将 Chunk* 转换为 T*
        return reinterpret_cast<T*>(chunk);
    }

    // 销毁对象并释放内存
    void destroy(T* ptr) {
        if (ptr) {
            // 显式调用析构函数
            ptr->~T();
            // 将内存块放回自由链表
            deallocate(ptr);
        }
    }

    // 仅释放内存，不调用析构函数
    void deallocate(T* ptr) {
        if (!ptr) return;

        // 将 T* 转换为 Chunk*
        Chunk* chunk = reinterpret_cast<Chunk*>(ptr);

        // 将块插回自由链表头部
        chunk->next = free_list_;
        free_list_ = chunk;
        --allocated_count_;
    }

    // 获取当前已分配的对象数量
    std::size_t allocated_count() const { return allocated_count_; }

private:
    // 申请新的内存块
    void allocate_block() {
        // 每次申请 BLOCK_SIZE 个对象的内存
        constexpr std::size_t block_size = BLOCK_SIZE * chunk_size_;

        // 使用 aligned_alloc 确保内存对齐（C++17）
        // 注意：aligned_alloc 在某些平台可能需要特定条件
        void* block = nullptr;
        #if defined(_WIN32) || defined(__CYGWIN__)
            block = _aligned_malloc(block_size, alignof(Chunk));
        #else
            // POSIX 系统使用 aligned_alloc
            block = std::aligned_alloc(alignof(Chunk), block_size);
        #endif

        if (!block) {
            throw std::bad_alloc();
        }

        blocks_.push_back(block);

        // 将新内存块划分为 chunk 并加入自由链表
        char* current = static_cast<char*>(block);
        for (std::size_t i = 0; i < BLOCK_SIZE; ++i) {
            Chunk* chunk = reinterpret_cast<Chunk*>(current);
            chunk->next = free_list_;
            free_list_ = chunk;
            current += chunk_size_;
        }
    }

    // 每个 chunk 的大小，取对象大小和 Chunk 结构大小的最大值（为了内存对齐）
    static constexpr std::size_t chunk_size_ =
        (sizeof(T) > sizeof(Chunk)) ? sizeof(T) : sizeof(Chunk);

    // 每次申请的内存块包含的 chunk 数量
    static constexpr std::size_t BLOCK_SIZE = 64;

    // 自由链表头指针
    Chunk* free_list_ = nullptr;

    // 记录所有申请的大内存块，用于最后统一释放
    std::vector<void*> blocks_;

    // 统计当前已分配的对象数量（用于调试和监控）
    std::size_t allocated_count_ = 0;
};

// 示例使用类
struct alignas(64) CacheAlignedData { // 手动指定64字节对齐（缓存行对齐）
    int values[16]; // 64字节：16 * 4字节
    double metrics[8];

    CacheAlignedData(int init_val = 0) {
        for (int& val : values) val = init_val;
        for (double& metric : metrics) metric = init_val * 0.5;
    }

    ~CacheAlignedData() {
        // 析构函数示例
    }
};

// 测试函数
void test_memory_pool() {
    MemoryPool<CacheAlignedData> pool;
    std::vector<CacheAlignedData*> objects;

    std::cout << "Testing memory pool with aligned objects...\n";

    // 批量创建对象
    for (int i = 0; i < 10; ++i) {
        CacheAlignedData* obj = pool.construct(i); // 使用参数构造
        objects.push_back(obj);

        // 验证内存对齐
        std::cout << "Object " << i << " address: " << obj
                  << " (aligned to 64 bytes: "
                  << ((reinterpret_cast<uintptr_t>(obj) % 64 == 0) ? "yes" : "no")
                  << ")\n";
    }

    // 使用对象
    for (auto obj : objects) {
        obj->values[0] = 42;
    }

    // 批量销毁对象
    for (auto obj : objects) {
        pool.destroy(obj);
    }
    objects.clear();

    std::cout << "Memory pool test completed successfully.\n";
    std::cout << "Final allocated count: " << pool.allocated_count() << "\n";
}

// 性能对比测试
void performance_comparison() {
    constexpr int COUNT = 100000;
    MemoryPool<CacheAlignedData> pool;

    std::cout << "\nPerformance comparison for " << COUNT << " allocations:\n";

    // 测试内存池性能
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < COUNT; ++i) {
        CacheAlignedData* obj = pool.construct();
        pool.destroy(obj);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto pool_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // 测试 new/delete 性能
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < COUNT; ++i) {
        CacheAlignedData* obj = new CacheAlignedData();
        delete obj;
    }
    end = std::chrono::high_resolution_clock::now();
    auto new_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "MemoryPool time: " << pool_duration.count() << " μs\n";
    std::cout << "new/delete time: " << new_duration.count() << " μs\n";
    std::cout << "Speedup: " << double(new_duration.count()) / pool_duration.count() << "x\n";
}

// 简单的测试类
struct SimpleData {
    int data[8];
    SimpleData() : data{1, 2, 3, 4, 5, 6, 7, 8} {}
};

void simple_test() {
    MemoryPool<SimpleData> pool;

    // 测试基本功能
    SimpleData* obj1 = pool.construct();
    SimpleData* obj2 = pool.construct();

    std::cout << "\nSimple test:\n";
    std::cout << "Object 1 address: " << obj1 << "\n";
    std::cout << "Object 2 address: " << obj2 << "\n";
    std::cout << "Allocated count: " << pool.allocated_count() << "\n";

    pool.destroy(obj1);
    pool.destroy(obj2);

    std::cout << "After destruction - Allocated count: " << pool.allocated_count() << "\n";
}

template<typename T>
class MemoryPool2 {
public:
  struct alignas(alignof(T)) Chunk{
    Chunk* next;
  };
  MemoryPool2() = default;
  ~MemoryPool2(){

    T* construct(){

    }

  }
private:
    static constexpr std::size_t BLOCK_SIZE = 64;
  static constexpr std::size_t ChunkSize = sizeof(T) > sizeof(Chunk) ? sizeof(T) : sizeof(Chunk);
  char* free_list_ = nullptr;
  // vector<Chunk*> blocks;

  std::vector<void*> blocks_;
  void allocate_block(){
    constexpr blockSize = BLOCK_SIZE * ChunkSize;
  }

  // const blockSize =

};
int main() {
    std::cout << "=== Memory Pool Demo ===\n";

    simple_test();
    test_memory_pool();

    std::cout << "\n=== Performance Comparison ===\n";
    performance_comparison();

    return 0;
}
