// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/base/zbuffer.h"
#include "p/base/logging.h"
#include <algorithm> // std::min
#include <atomic>    // std::atomic<>
#include <new>       // new
#include <stdlib.h>  // ::malloc,::free
#include <sys/uio.h> // ::preadv
#include <assert.h>  // ::assert
#include <algorithm> // std::min

namespace p {
namespace base {

std::atomic<size_t> g_block_number;
std::atomic<size_t> g_block_memory;

std::atomic<size_t> g_huge_block_number;
std::atomic<size_t> g_huge_block_memory;

size_t ZBuffer::total_block_number() { return g_block_number; }

size_t ZBuffer::total_block_memory() { return g_block_memory; }

class ThreadLocalBlockCache {
public:
    ThreadLocalBlockCache() : head_{nullptr}, block_number_(0) {}

    ~ThreadLocalBlockCache();

    ZBuffer::Block *acquire_block();

    ZBuffer::Block *pop_front();

    void push_front(ZBuffer::Block* block);

    void release_block(ZBuffer::Block *block);

    void *acquire_block_ref_array(ZBuffer::BlockRef *ref, uint32_t ref_size);

    friend class ZBuffer;
private:
    ZBuffer::Block *head_;
    uint32_t block_number_;
};

static thread_local ThreadLocalBlockCache tls_block_cache;

ZBuffer::Block::Block(uint16_t cap) : next{nullptr}, ref_num{1}, offset{0}, capacity{cap} {
    capacity -= (uint16_t)sizeof(Block);
    g_block_number.fetch_add(1, std::memory_order_relaxed);
    g_block_memory.fetch_add(cap, std::memory_order_relaxed);
}

void ZBuffer::Block::dec_ref() {
    if (ref_num.fetch_sub(1, std::memory_order_release) <= 1) {
        LOG_DEBUG << "try return a Block, ptr=" << this << ",size=" << capacity;

        next = nullptr;
        offset = 0;

        tls_block_cache.release_block(this);
    }
}

static_assert(sizeof(ZBuffer::Block) == 24, "invalid sizeof ZBuffer::Block");

constexpr size_t kNormalBlockPayloadSize = ZBuffer::kNormalBlockSize - sizeof(ZBuffer::Block);

inline ZBuffer::Block *ZBuffer::Block::create_block(uint32_t size) {
    void *ptr = ::malloc((size + 0x7) & ~0x7);
    ZBuffer::Block *block = new (ptr) ZBuffer::Block(size);
    LOG_DEBUG << "new a Block, ptr=" << block << ",size=" << block->capacity;
    return block;
}

inline void ZBuffer::Block::free_block(Block* block) {
    LOG_DEBUG << "free a Block, ptr=" << block << ", size=" << block->capacity;
    g_block_number.fetch_sub(1, std::memory_order_relaxed);
    g_block_memory.fetch_sub(block->capacity + sizeof(Block), std::memory_order_relaxed);
    ::free(block);
}

ThreadLocalBlockCache::~ThreadLocalBlockCache() {
    ZBuffer::Block *block;
    while (head_) {
        block = head_;
        head_ = head_->next;
        block->dec_ref();
    }
}

inline ZBuffer::Block *ThreadLocalBlockCache::acquire_block() {
    ZBuffer::Block *block = nullptr;
    while (head_) {
        if (!head_->full()) {
            return head_;
        }

        block = head_->next;
        head_->dec_ref();

        head_ = block;
        --block_number_;
    }

    block = ZBuffer::Block::create_block(ZBuffer::kNormalBlockSize);
    ++block_number_;

    head_ = block;
    return block;
}

inline ZBuffer::Block * ThreadLocalBlockCache::pop_front() {
    ZBuffer::Block *block = nullptr;
    while (head_) {
        block = head_;
        head_ = head_->next;

        if (!block->full()) {
            return block;
        }

        block->dec_ref();
        --block_number_;
    }

    block = ZBuffer::Block::create_block(ZBuffer::kNormalBlockSize);
    ++block_number_;

    return block;
}

inline void ThreadLocalBlockCache::push_front(ZBuffer::Block *block) {
    if (block->full()) {
        block->dec_ref();
        return;
    }

    // insert
    block->next = head_;
    head_ = block;
    ++block_number_;
}

inline void ThreadLocalBlockCache::release_block(ZBuffer::Block *block) {
    if (block_number_ < ZBuffer::kBlockCachedPerThread) {
        ++block_number_;
        block->inc_ref();
        block->next = head_;
        head_ = block;
        return;
    }

    ZBuffer::Block::free_block(block);
}

inline void *ThreadLocalBlockCache::acquire_block_ref_array(ZBuffer::BlockRef *ref,
                                                            uint32_t ref_size) {
    ZBuffer::Block *block = nullptr;
    size_t array_bytes = ref_size * sizeof(ZBuffer::BlockRef);
    array_bytes += (sizeof(ZBuffer::BlockRefArray) + sizeof(ZBuffer::BlockRef));
    if (array_bytes < kNormalBlockPayloadSize) {
        LOG_DEBUG << "acquire_block_ref_array need size=" << array_bytes;
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
        LOG_WARN << "acquire_block_ref_array big size=" << array_bytes;
        block = ZBuffer::Block::create_block(array_bytes + sizeof(ZBuffer::Block));
        block->next = head_;
        head_ = block;
        ++block_number_;
    }

    LOG_DEBUG << this << " ZBuffer found block=" << (void*)block << ",offset=" << block->offset
        << ",length=" << array_bytes;

    uintptr_t offset = (uintptr_t)(block->data + block->offset);
    offset += (sizeof(ZBuffer::BlockRefArray) - 1);
    offset &= ~(sizeof(ZBuffer::BlockRefArray) - 1);

    block->inc_ref();
    ref->offset = block->offset;
    ref->block = block;
    ref->length = array_bytes;

    block->offset += array_bytes;

    LOG_DEBUG << "Block " << this << " offset=" << block->offset;

    return (void *)offset;
}

void ZBuffer::acquire_block_ref(ZBuffer::BlockRef* ref) {
    ZBuffer::Block* block = tls_block_cache.pop_front();
    ref->block = block;
    ref->offset = block->offset;
    ref->length = block->left_space();
}

void ZBuffer::return_block_ref(BlockRef* ref) {
    ZBuffer::Block* block = ref->block;
    block->offset += ref->length;
    block->inc_ref();
    tls_block_cache.push_front(block);
}

ZBuffer::~ZBuffer() {
    if (!array()) {
        first_.release();
        second_.release();
        return;
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

    void *ptr = tls_block_cache.acquire_block_ref_array(&first_ref, kInitBlockRefArraySize);
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
    BlockRef tmp;
    uint64_t array_size = (refs_array->cap_mask + 1) << 1;
    void *ptr = tls_block_cache.acquire_block_ref_array(&tmp, array_size);
    refs_array = new (ptr) BlockRefArray(refs_array, array_size);

    magic_num = -1;
    first_ref.dec_ref();
    first_ref = tmp;
}

inline void ZBuffer::array_append_ref(BlockRef &&ref) {
    // assert(ref_num > 0);
    BlockRef &last = refs_array->ref_at(refs_num - 1);
    refs_array->nbytes += ref.length;

    if (last.merge(ref)) {
        ref.release();
        return;
    }

    if (UNLIKELY(refs_num > refs_array->cap_mask)) { array_resize(); }

    refs_array->ref_at(refs_num) = ref;
    ++refs_num;
    ref.reset();
}

inline void ZBuffer::array_append_ref(const BlockRef &ref) {
    // assert(ref_num > 0);
    BlockRef &last = refs_array->ref_at(refs_num - 1);
    refs_array->nbytes += ref.length;

    if (last.merge(ref)) {
        return;
    }

    if (UNLIKELY(refs_num > refs_array->cap_mask)) { array_resize(); }

    (refs_array->ref_at(refs_num) = ref).inc_ref();
    ++refs_num;
}

inline void ZBuffer::append_ref(const BlockRef &ref) {
    if (!array()) {
        if (!first_.block) {
            first_ = ref;
            first_.inc_ref();
            return;
        }

        if (!second_.block) {
            // second_.block == nullptr
            if (first_.merge(ref)) {
                return;
            }

            second_ = ref;
            second_.inc_ref();
            return;
        }

        if (second_.merge(ref)) {
            return;
        }
        // transfer to array
        transfor_to_array();
    }

    array_append_ref(ref);
}

inline void ZBuffer::append_ref(BlockRef &&ref) {
    if (!array()) {
        if (!first_.block) {
            first_ = ref;
            ref.reset();
            return;
        }

        if (!second_.block) {
            // second_.block == nullptr
            if (first_.merge(ref)) {
                ref.release();
                return;
            }

            second_ = ref;
            ref.reset();
            return;
        }

        if (second_.merge(ref)) {
            ref.release();
            return;
        }
        // transfer to array
        transfor_to_array();
    }

    array_append_ref(std::move(ref));
}

int ZBuffer::append(const char *buf, size_t count) {
    size_t copied = 0;
    while (copied < count) {
        ZBuffer::Block *block = tls_block_cache.acquire_block();
        uint32_t left_space = block->left_space();
        uint32_t left = count - copied;
        if (left > left_space) {
            left = left_space;
        }
        ::memcpy(block->data + block->offset, buf + copied, left);

        ZBuffer::BlockRef ref = {block->offset, left, block};
        block->offset += left; // must befor append_ref
        block->offset += 7;
        block->offset = block->offset & ~0x7;
        append_ref(ref);
        copied += left;
    }
    return 0;
}

int ZBuffer::append(ZBuffer&& rh) {
    int nr = 0;
    if (!rh.array()) {
        if (rh.first_.length) {
            append_ref(std::move(rh.first_));
            ++nr;
        } else {
            rh.first_.release();
        }

        if (rh.second_.length) {
            append_ref(std::move(rh.second_));
            ++nr;
        } else {
            rh.second_.release();
        }
        return 0;
    }

    int ct = rh.refs_num;
    for (int i = 0; i < ct; ++i) {
        BlockRef &last = rh.refs_array->ref_at(i);
        append_ref(std::move(last));
        ++nr;
    }
    rh.refs_num = 0;
    rh.refs_array->reset();

    return nr;
}

int ZBuffer::append(const ZBuffer& rh) {
    int nr = 0;
    if (!rh.array()) {
        if (rh.first_.length) {
            append_ref(rh.first_);
            ++nr;
        }

        if (rh.second_.length) {
            append_ref(rh.second_);
            ++nr;
        }
        return 0;
    }

    int ct = rh.refs_num;
    for (int i = 0; i < ct; ++i) {
        append_ref(rh.refs_array->ref_at(i));
        ++nr;
    }

    return nr;
}

size_t ZBuffer::simple_popn(char *buf, size_t count) {
    if (!first_.length) {
        return 0;
    }

    if (first_.length > (int64_t)count) {
        ::memcpy(buf, first_.begin(), count);
        first_.offset += count;
        first_.length -= count;
        return count;
    }

    size_t copied = first_.length;
    ::memcpy(buf, first_.begin(), copied);
    count -= copied;
    first_.release();

    if (!second_.block) {
        return copied;
    }

    if ((int64_t)count >= second_.length) {
        count = second_.length;
        ::memcpy(buf + copied, second_.begin(), count);
        second_.release();
        return copied + count;
    }

    ::memcpy(buf + copied, second_.begin(), count);
    second_.offset += count;
    second_.length -= count;
    first_ = second_;
    second_.reset();
    return copied + count;
}

size_t ZBuffer::array_popn(char *buf, size_t count) {
    size_t copied = 0;

    uint32_t i = 0;
    for (; i < (uint32_t)refs_num; ++i) {
        BlockRef &ref = refs_array->ref_at(i);
        if (ref.length > count) {
            ::memcpy(buf + copied, ref.begin(), count);
            copied += count;
            ref.offset += count;
            ref.length -= count;
            break;
        }
        ::memcpy(buf + copied, ref.begin(), ref.length);
        copied += ref.length;
        count -= ref.length;
        ref.release();
    }

    refs_array->nbytes -= copied;
    refs_array->begin += i;
    refs_num -= i;
    return copied;
}

size_t ZBuffer::copy_front(char*buf, size_t count) {
    size_t copied = 0;

    if (!array()) {
        {
            BlockRef &ref = first_;
            if (ref.length >= count) {
                ::memcpy(buf + copied, ref.begin(), count);
                copied += count;
                return copied;
            }

            ::memcpy(buf + copied, ref.begin(), ref.length);
            copied += ref.length;
            count -= ref.length;
        }
        {
            BlockRef &ref = second_;
            if (ref.length >= count) {
                ::memcpy(buf + copied, ref.begin(), count);
                copied += count;
                return copied;
            }

            ::memcpy(buf + copied, ref.begin(), ref.length);
            copied += ref.length;
            count -= ref.length;
        }
        return copied;
    }

    uint32_t i = 0;
    for (; i < refs_num; ++i) {
        BlockRef &ref = refs_array->ref_at(i);
        if (ref.length >= count) {
            ::memcpy(buf + copied, ref.begin(), count);
            copied += count;
            return copied;
        }

        ::memcpy(buf + copied, ref.begin(), ref.length);
        copied += ref.length;
        count -= ref.length;
    }
    return copied;

}

int64_t ZBuffer::read_from_fd(int fd, int64_t offset, size_t count) {
    constexpr int kMaxIov = 64;
    iovec iov[kMaxIov];

    ZBuffer::Block *p = tls_block_cache.acquire_block();
    ZBuffer::Block *prev = nullptr;
    assert(p);
    assert(p == tls_block_cache.head_);
    ZBuffer::Block *tmp = p;

    int i = 0;
    size_t space = 0;

    do {
        if (p == nullptr) {
            p = ZBuffer::Block::create_block(ZBuffer::kNormalBlockSize);
            prev->next = p;
            ++tls_block_cache.block_number_;
        }
        iov[i].iov_base = p->data + p->offset;
        iov[i].iov_len = std::min((size_t)p->left_space(), count - space);
        space += iov[i].iov_len;

        ++i;
        prev = p;
        p = p->next;

        if (space >= count || i >= kMaxIov) {
            break;
        }
    } while (1);

    int nr = 0;
    if (offset < 0) {
        nr = ::readv(fd, iov, i);
    } else {
        nr = ::preadv(fd, iov, i, offset);
    }

    if (nr <= 0) {
        return nr;
    }

    size_t total = nr;
    p = tmp;
    assert(tmp == tls_block_cache.head_);
    prev = nullptr;
    int j = 0;
    ZBuffer::BlockRef   tmp_save[kMaxIov];
    do {
        uint32_t len = std::min(total, (size_t)p->left_space());
        if (total != len) {
            assert(iov[j].iov_len == len);
        }
        assert(iov[j].iov_base == (p->data + p->offset));
        total -= len;
        tmp_save[j] = { p->offset, len, p};
        p->offset += len;
        ++j;

        if (p->full()) {
            --tls_block_cache.block_number_;
            prev = p;
            p = p->next;
        } else {
            p->offset = (p->offset + 0x7) & ~0x7;
            p->inc_ref();
            assert(total == 0);
        }
    } while (total);

    assert(i >= j);

    tls_block_cache.head_ = p;

    for (i = 0; i < j; ++i) {
        append_ref(std::move(tmp_save[i]));
    }

    return nr;
}

int ZBuffer::dump_refs_to(std::deque<ZBuffer::BlockRef>* queue) {
    int nr = 0;
    if (!array()) {
        if (first_.length) {
            queue->push_back(first_);
            first_.reset();
            ++nr;
        } else {
            first_.release();
        }

        if (second_.length) {
            queue->push_back(second_);
            second_.reset();
            ++nr;
        } else {
            second_.release();
        }
        return nr;
    }

    int ct = refs_num;
    for (int i = 0; i < ct; ++i) {
        BlockRef &last = refs_array->ref_at(i);
        if (last.length) {
            queue->push_back(last);
            ++nr;
        } else {
            last.release();
        }
    }

    first_ref.release();

    first_.reset();
    second_.reset();
    return nr;
}

int ZBuffer::map(void (*f)(char* buf, int len, void* arg), void* arg) {
    int nr = 0;
    if (!array()) {
        if (first_.block) {
            f(first_.begin(), first_.length, arg);
            ++nr;
        }

        if (second_.block) {
            f(second_.begin(), second_.length, arg);
            ++nr;
        }
        return nr;
    }

    int ct = refs_num;
    for (int i = 0; i < ct; ++i) {
        BlockRef &last = refs_array->ref_at(i);
        f(last.begin(), last.length, arg);
        ++nr;
    }

    return nr;
}

} // end namespace base
} // end namespace p

