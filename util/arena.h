//
// Created by 罗旭维 on 2021/11/18.
//

#ifndef MYLEVELDB_ARENA_H
#define MYLEVELDB_ARENA_H
#include <cstddef>
#include <vector>
#include <atomic>
#include <cassert>

namespace myleveldb {
    class Arena {
    public:
    Arena();

    Arena(const Arena& ) = delete;
    Arena& operator=(const Arena&) = delete;

    ~Arena();

    char* Allocate(size_t bytes);
    char* AllocateAligned(size_t bytes);//按平台位数，手工对齐

    size_t MemoryUsage() const {
        return memory_usage_.load(std::memory_order_relaxed);
    }

    private:
    char* AllocateFallback(size_t bytes);
    char* AllocateNewBlock(size_t block_bytes);

    //当前空闲内存 block 内的可用地址
    char* alloc_ptr_;
    //当前空闲内存 block 内的可用大小
    size_t alloc_bytes_remaining_;

    // 已经申请的内存 block
    std::vector<char*> blocks_;

    //累计分配的内存大小
    std::atomic<size_t> memory_usage_;
    };

    char *Arena::Allocate(size_t bytes) {
        assert(bytes > 0);
        if (bytes < alloc_bytes_remaining_) {
            char* result = alloc_ptr_;
            alloc_ptr_ += bytes;
            alloc_bytes_remaining_ -= bytes;
            return result;
        }
        return AllocateFallback(bytes);
    }
}



#endif //MYLEVELDB_ARENA_H
