// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/base/zbuffer.h"
#include <stdlib.h>     // ::malloc,::free
#include <atomic>
#include <new>

namespace p {
namespace base {

std::atomic<size_t>     g_block_number;
std::atomic<size_t>     g_block_memory;

struct ZBuffer::Block {
    Block(uint16_t cap) : next{nullptr}, ref_num{1}, offset{0},
                capacity{cap} {
        capacity -= (uint16_t)sizeof(Block);
        g_block_number.fetch_add(1, std::memory_order_relaxed);
        g_block_memory.fetch_add(cap, std::memory_order_relaxed);
    }

    bool full() const {
        return ((uint32_t)offset + 7) >= capacity;
    }

    uint32_t left_space() const {
        return (uint32_t)capacity - offset;
    }

    void inc_ref() {
        ref_num.fetch_add(1, std::memory_order_release);
    }

    void dec_ref() {
        if (ref_num.fetch_sub(1, std::memory_order_release) <= 1) {
            g_block_number.fetch_sub(1, std::memory_order_relaxed);
            g_block_memory.fetch_sub(capacity + sizeof(Block), std::memory_order_relaxed);
            ::free(this);
        }
    }

    static Block* create_block(uint16_t size);

public:
    struct ZBuffer::Block*  next;
    std::atomic<int32_t>    ref_num;
    uint16_t                offset;
    uint16_t                capacity;
    char                    data[0];
};

static_assert(sizeof(ZBuffer::Block) == 16, "invalid sizeof ZBuffer::Block");

inline ZBuffer::Block* ZBuffer::Block::create_block(uint16_t size) {
    void* ptr = ::malloc(size);
    ZBuffer::Block* block =  new (ptr) ZBuffer::Block(size);
    return block;
}

class ThreadLocalBlockCache {
public:
    ThreadLocalBlockCache() : head_{nullptr} {
    }

    ZBuffer::Block* acquire_block();

    void release_block(ZBuffer::Block* block);
private:
    ZBuffer::Block*     head_;
    uint32_t            block_number_;
};

static thread_local ThreadLocalBlockCache tls_block_cache;

inline ZBuffer::Block* ThreadLocalBlockCache::acquire_block() {
    ZBuffer::Block* block = nullptr;
    if (head_) {
        if (!head_->full()) {
            return head_;
        }
        block = head_->next;
        head_->dec_ref();
        --block_number_;
    }

    if (!block) {
        block = ZBuffer::Block::create_block(ZBuffer::kBlockSize);
        ++block_number_;
    }

    head_ = block;
    return block;
}

inline void ThreadLocalBlockCache::release_block(ZBuffer::Block* block) {
    if (block->full()) {
        block->dec_ref();
        return ;
    }

    if (block_number_ < ZBuffer::kBlockCachedPerThread) {
        ++block_number_;
        block->next = head_;
        head_ = block;
        return ;
    }

    block->dec_ref();
}



} // end namespace base
} // end namespace p

