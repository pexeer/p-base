// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <stdint.h> // uint32_t, uint64_t
#include <string.h> // size_t
#include <atomic>   // std::atomic<>
#include <deque>

namespace p {
namespace base {

// Zero-copy Buffer
class ZBuffer {
public:
    // 8kbytes for each normal block
    static constexpr size_t kNormalBlockSize = 8 * 1024Ul;
    static constexpr size_t kInitBlockRefArraySize = 32UL;
    static constexpr size_t kBlockCachedPerThread = 32UL;

    struct Block {
        Block(uint16_t cap);

        bool full() const { return (offset + 7) >= capacity; }

        uint32_t left_space() const { return capacity - offset; }

        void inc_ref() { ref_num.fetch_add(1, std::memory_order_release); }

        void dec_ref();

        static Block *create_block(uint32_t size);

        static void free_block(Block* block);

    public:
        struct Block            *next;
        std::atomic<int64_t>    ref_num;
        uint32_t                 offset;
        uint32_t                 capacity;
        char                    data[0];
    };

    struct BlockRef {
        void reset() {
            offset = 0;
            length = 0;
            block = nullptr;
        }

        bool merge(const BlockRef &rhs) {
            if ((block == rhs.block) && (offset + length == rhs.offset)) {
                length += rhs.length;
                return true;
            }
            return false;
        }

        char *begin() { return block->data + offset; }

        void inc_ref() const {
            block->inc_ref();
        }

        void dec_ref() const {
            block->dec_ref();
        }

        void release() {
            if (block) {
                block->dec_ref();
            }
            offset = 0;
            length = 0;
            block = nullptr;
        }

    public:
        uint32_t offset;
        uint32_t length;
        Block *block;
    };

    static void acquire_block_ref(BlockRef* ref);
    static void return_block_ref(BlockRef* ref);

    static_assert(sizeof(BlockRef) == 16, "invalid sizeof BlockRef");

    struct BlockRefArray {
        BlockRefArray(uint32_t cap) : nbytes{0}, begin{0}, cap_mask(cap - 1) {}

        BlockRefArray(BlockRefArray *old, uint32_t cap)
            : nbytes{old->nbytes}, begin{old->begin}, cap_mask{cap - 1} {
            for (uint32_t i = 0; i <= old->cap_mask; ++i) {
                refs[i] = old->refs[i];
            }
        }

        uint32_t capacity() const { return cap_mask + 1; }

        const BlockRef &ref_at(uint32_t i) const { return refs[(begin + i) & cap_mask]; }

        BlockRef &ref_at(uint32_t i) { return refs[(begin + i) & cap_mask]; }

        void reset() {
            nbytes = 0;
            begin = 0;
        }

    public:
        uint64_t nbytes;
        uint32_t begin;
        uint32_t cap_mask;
        BlockRef refs[0];
    };
    static_assert(sizeof(BlockRefArray) == 16, "invalid sizeof BlockRefArray");

public:
    ZBuffer() {
        first_.reset();
        second_.reset();
    }

    ~ZBuffer();

    // this is BlockRefArray
    bool array() const { return magic_num < 0; }

    size_t size() const {
        if (!array()) {
            return first_.length + second_.length;
        }

        return refs_array->nbytes;
    }

    int32_t blockref_num() {
        if (!array()) {
            if (second_.block == nullptr) {
                return first_.block ? 1 : 0;
            }
            return 1 + (first_.block ? 1 : 0);
        }

        return 1 + refs_num;
    }

    int append(const char *buf, size_t count);

    int append(ZBuffer&& rh);

    int append(const ZBuffer& rh);

    size_t popn(char *buf, size_t count) {
        if (array()) {
            return array_popn(buf, count);
        }
        return simple_popn(buf, count);
    }

    size_t copy_front(char*buf, size_t count);

    void array_resize();

    static size_t total_block_number();

    static size_t total_block_memory();

    void append_ref(BlockRef &&ref);

    void append_ref(const BlockRef &ref);

    int64_t read_from_fd(int fd, int64_t offset, size_t count);

    int dump_refs_to(std::deque<ZBuffer::BlockRef>* queue);

    int map(void (*f)(char* buf, int len, void* arg), void* arg);

protected:
    void array_append_ref(BlockRef &&ref);

    void array_append_ref(const BlockRef &ref);

    void transfor_to_array();

    size_t array_popn(char *buf, size_t count);

    size_t simple_popn(char *buf, size_t count);

private:
    union {
        struct {
            BlockRef first_;
            BlockRef second_;
        };

        struct {
            int32_t magic_num;
            uint32_t refs_num;
            BlockRefArray *refs_array;
            BlockRef first_ref;
        };
    };
};

static_assert(sizeof(ZBuffer) == 32, "invalid sizeof ZBuffer");

} // end namespace base
} // end namespace p

