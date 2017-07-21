// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <stdint.h>     // uint32_t, uint64_t
#include <string.h>     // size_t

namespace p {
namespace base {

// Zero-copy Buffer
class ZBuffer {
public:
    // 8kbytes for each normal block
    static constexpr size_t kNormalBlockSize = 16 * 1024Ul;
    static constexpr size_t kInitBlockRefArraySize = 16UL;
    static constexpr size_t kBlockCachedPerThread = 16UL;

    struct Block;

    struct BlockRef {
        void reset() {
            offset = 0;
            length = 0;
            block = nullptr;
        }

        bool merge(const BlockRef& rhs) {
            if ((block == rhs.block) && (offset + length == rhs.offset)) {
                length += rhs.length;
                return true;
            }
            return false;
        }

        const char* begin() const ;

        void inc_ref() const ;

        void dec_ref() const ;

        void release();

    public:
        uint32_t offset;
        uint32_t length;
        Block*   block;
    };
    static_assert(sizeof(BlockRef) == 16, "invalid sizeof BlockRef");

    struct BlockRefArray {
        BlockRefArray(uint32_t cap) : nbytes{0}, begin{0}, cap_mask(cap - 1) {}

        BlockRefArray(BlockRefArray* old, uint32_t cap) : nbytes{old->nbytes},
                begin{old->begin}, cap_mask{cap - 1} {
            for (uint32_t i = 0; i <= old->cap_mask; ++i) {
                refs[i] = old->refs[i];
            }
        }

        uint32_t capacity() const {
            return cap_mask + 1;
        }

        const BlockRef& ref_at(uint32_t i) const {
            return refs[(begin + i) & cap_mask];
        }

        BlockRef& ref_at(uint32_t i) {
            return refs[(begin + i) & cap_mask];
        }

    public:
        uint64_t    nbytes;
        uint32_t    begin;
        uint32_t    cap_mask;
        BlockRef    refs[0];
    };
    static_assert(sizeof(BlockRefArray) == 16, "invalid sizeof BlockRefArray");

public:
    ZBuffer() {
        first_.reset();
        second_.reset();
    }

    ~ZBuffer();

    // this is BlockRefArray
    bool array() const {
        return magic_num < 0;
    }

    size_t size() const {
        if (!array()) {
            return first_.length + second_.length;
        }

        return refs_array->nbytes;
    }

    uint32_t blockref_num() {
        if (!array()) {
            if (second_.block == nullptr) {
                return (uint32_t)(first_.block != nullptr);
            }
            return 1 + (first_.block != nullptr);
        }

        return 1 + refs_num;
    }

    int append(const char* buf, size_t count);

    size_t popn(char* buf, size_t count) {
        if (array()) {
            return array_popn(buf, count);
        }
        return simple_popn(buf, count);
    }

    void array_resize();

    static size_t total_block_number();

    static size_t total_block_memory();

protected:
    void append_ref(BlockRef&& ref);

    void append_ref(const BlockRef& ref);

    void array_append_ref(BlockRef&& ref);

    void array_append_ref(const BlockRef& ref);

    void transfor_to_array();

    size_t array_popn(char* buf, size_t count);

    size_t simple_popn(char* buf, size_t count);

private:
    union {
        struct {
            BlockRef    first_;
            BlockRef    second_;
        };

        struct {
            int32_t         magic_num;
            int32_t         refs_num;
            BlockRefArray*  refs_array;
            BlockRef        first_ref;
        };
    };
};
static_assert(sizeof(ZBuffer) == 32, "invalid sizeof ZBuffer");

} // end namespace base
} // end namespace p

