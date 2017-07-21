// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/base/zbuffer.h"
#include "p/base/logging.h"
#include <stdlib.h>     // ::malloc,::free
#include <algorithm>    // std::min
#include <atomic>       // std::atomic<>
#include <new>          // new

namespace p {
namespace base {

std::atomic<size_t>     g_block_number;
std::atomic<size_t>     g_block_memory;

size_t ZBuffer::total_block_number() {
    return g_block_number;
}

size_t ZBuffer::total_block_memory() {
    return g_block_memory;
}


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

            //LOG_WARN << "free a Block, ptr=" << this << ", size=" << capacity;
            ::free(this);
        }
    }

    static Block* create_block(uint32_t size);

public:
    struct ZBuffer::Block*  next;
    std::atomic<int64_t>    ref_num;
    int32_t                 offset;
    int32_t                 capacity;
    char                    data[0];
};
static_assert(sizeof(ZBuffer::Block) == 24, "invalid sizeof ZBuffer::Block");

constexpr size_t kNormalBlockPayloadSize = ZBuffer::kNormalBlockSize - sizeof(ZBuffer::Block);

inline void ZBuffer::BlockRef::inc_ref() const {
    block->inc_ref();
}

inline void ZBuffer::BlockRef::dec_ref() const {
    block->dec_ref();
}

inline void ZBuffer::BlockRef::release() {
    if (block) {
        block->dec_ref();
    }
    offset = 0;
    length = 0;
    block = nullptr;
}

inline ZBuffer::Block* ZBuffer::Block::create_block(uint32_t size) {
    void* ptr = ::malloc(size);
    ZBuffer::Block* block =  new (ptr) ZBuffer::Block(size);
    return block;
}

class ThreadLocalBlockCache {
public:
    ThreadLocalBlockCache() : head_{nullptr}, block_number_(0) {
    }

    ~ThreadLocalBlockCache() {
        ZBuffer::Block* block;
        while (head_) {
            block = head_;
            head_ = head_->next;
            block->dec_ref();
        }
    }

    ZBuffer::Block* acquire_block();

    void release_block(ZBuffer::Block* block);

    void* acquire_block_ref_array(ZBuffer::BlockRef* ref, uint32_t ref_size);

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
        block = ZBuffer::Block::create_block(ZBuffer::kNormalBlockSize);
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

inline void* ThreadLocalBlockCache::acquire_block_ref_array(ZBuffer::BlockRef* ref, uint32_t ref_size) {
    ZBuffer::Block* block = nullptr;
    size_t array_bytes = ref_size * sizeof(ZBuffer::BlockRef);
    array_bytes += (sizeof(ZBuffer::BlockRefArray) + sizeof(ZBuffer::BlockRef));
    if (array_bytes < kNormalBlockPayloadSize) {
        block = head_;
        while (block) {
            if (block->left_space() >= array_bytes) {
                break;
            }
            block = block->next;
        }
        if (!block) {
            block = ZBuffer::Block::create_block(ZBuffer::kNormalBlockSize);
            block->next = head_;
            head_ = block;
            ++block_number_;
        }
    } else {
        //LOG_WARN << "acquire_block_ref_array big size=" << array_bytes;
        block = ZBuffer::Block::create_block(array_bytes + sizeof(ZBuffer::Block));
        block->next = head_;
        head_ = block;
        ++block_number_;
    }

    uintptr_t offset = (uintptr_t)&(block->data[block->offset]);
    offset += (sizeof(ZBuffer::BlockRefArray) - 1);
    offset &= ~(sizeof(ZBuffer::BlockRefArray) - 1);

    uintptr_t end = offset + sizeof(ZBuffer::BlockRefArray)
            + ref_size * sizeof(ZBuffer::BlockRef);


    ref->offset = block->offset;
    ref->block = block;
    ref->length = uint32_t(end - offset);

    block->offset += ref->length;
    block->inc_ref();

    return (void*)offset;
}

ZBuffer::~ZBuffer() {
    if (!array()) {
        first_.release();
        second_.release();
        return ;
    }

    for (uint32_t i = 0; i < refs_num; ++i) {
        refs_array->ref_at(i).release();
    }
    first_ref.release();
}

inline void ZBuffer::transfor_to_array() {
    BlockRef a, b;
    a = first_;
    b = second_;

    void* ptr = tls_block_cache.acquire_block_ref_array(&first_ref, kInitBlockRefArraySize);
    refs_array = new (ptr) BlockRefArray(kInitBlockRefArraySize);

    magic_num = -1;
    refs_num = 0;

//    if (a.block) {
//        array_append_ref(std::move(a));
//    }
//    if (b.block) {
//        array_append_ref(std::move(b));
//    }

    if (a.block) {
        refs_array->ref_at(refs_num) = a;
        refs_array->nbytes += a.length;
        ++refs_num;
    }

    if (b.block) {
        refs_array->ref_at(refs_num) = b;
        refs_array->nbytes += b.length;
        ++refs_num;
    }
}

inline void ZBuffer::array_resize() {
    BlockRef    tmp;
    uint64_t array_size = (refs_array->cap_mask + 1) << 1;
    void* ptr = tls_block_cache.acquire_block_ref_array(&tmp, array_size);
    refs_array = new (ptr) BlockRefArray(refs_array, array_size);

    magic_num = -1;
    first_ref.dec_ref();
    first_ref = tmp;
}

inline void ZBuffer::array_append_ref(BlockRef&& ref) {
    // assert(ref_num > 0);
    BlockRef& last = refs_array->ref_at(refs_num - 1);
    refs_array->nbytes += ref.length;

    if (last.merge(ref)) {
        ref.dec_ref();
        return ;
    }

    if UNLIKELY(refs_num > refs_array->cap_mask) {
        array_resize();
    }

    refs_array->ref_at(refs_num) = ref;
    ++refs_num;
}

inline void ZBuffer::array_append_ref(const BlockRef& ref) {
    // assert(ref_num > 0);
    BlockRef& last = refs_array->ref_at(refs_num - 1);
    refs_array->nbytes += ref.length;

    if (last.merge(ref)) {
        return ;
    }

    if UNLIKELY(refs_num > refs_array->cap_mask) {
        array_resize();
    }

    (refs_array->ref_at(refs_num) = ref).inc_ref();
    ++refs_num;
}

inline void ZBuffer::append_ref(const BlockRef& ref) {
    if (!array()) {
        if (!first_.block) {
            first_ = ref;
            first_.inc_ref();
            return ;
        }

        if (!second_.block) {
            // second_.block == nullptr
            if (first_.merge(ref)) {
                return ;
            }

            second_ = ref;
            second_.inc_ref();
            return ;
        }

        if (second_.merge(ref)) {
            return ;
        }
        // transfer to array
        transfor_to_array();
    }

    array_append_ref(ref);
}

inline void ZBuffer::append_ref(BlockRef&& ref) {
    if (!array()) {
        if (!first_.block) {
            first_ = ref;
            return ;
        }

        if (!second_.block) {
            // second_.block == nullptr
            if (first_.merge(ref)) {
                ref.dec_ref();
                return ;
            }

            second_ = ref;
            return ;
        }

        if (second_.merge(ref)) {
            ref.dec_ref();
            return ;
        }
        // transfer to array
        transfor_to_array();
    }

    array_append_ref(ref);
}

int ZBuffer::append(const char* buf, size_t count) {
    size_t copied = 0;
    while (copied < count) {
        ZBuffer::Block* block = tls_block_cache.acquire_block();

        size_t left_space = block->left_space();
        size_t left = count - copied;
        if (left > left_space) {
            left = left_space;
        }
        ::memcpy(block->data + block->offset, buf + copied, left);

        const ZBuffer::BlockRef ref = {(uint32_t)block->offset, (uint32_t)left, block};
        append_ref(ref);
        block->offset += left;
        copied += left;
    }
    return 0;
}

#if 0
int ZBuffer::popn() {
        if (!array()) {
            if (!first_.length) {
                return 0;
            }

            if (first_.length > count) {
                first_.offset += count;
                first_.length -= count;
                ::memcpy(buf, count)
                return count;
            }

            size_t copied = first_.length;
            count -= copied;
            ::memcpy(buf, , copied);
            first_.release();
            if (!second_.block) {
                return copied;
            }
            buf += copied;

            if (count > second_.length) {
                count = second_.length;
                ::memcpy(buf, , count);
                second_.release();
                return copied + count;
            }

            ::memcpy(buf, , count);
            second_.offset += count;
            second_.length -= count;
            first_ = second_;
            second_.reset();
            return copied + count;
        }
}
#endif

} // end namespace base
} // end namespace p

