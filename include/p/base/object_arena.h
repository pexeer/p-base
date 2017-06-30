// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <assert.h>
#include <unistd.h>
#include <atomic>
#include <mutex>
#include "p/base/logging.h"

namespace p {
namespace base {

template<typename T> class ObjectArena {
public:
    constexpr static size_t BASE = 10;
    constexpr static size_t BASE_MARK = 0x3FF;
    constexpr static size_t N = (1 << BASE);
    constexpr static size_t NG = (1 << (31 - BASE - BASE));

    struct ObjectBlock {
        T                       items[N];
    };

    static T* get(uint64_t* id) {
        return global_group_.add_block(id)->items;
    }

    static T* find(uint64_t obj_id) {
        return global_group_.find(obj_id);
    }

private:
    struct BlockGroup {
        std::atomic<ObjectBlock*>            block_list[N];
        std::atomic<size_t>                  block_size = {0};
    };

    class GlobalGroup {
    public:
        GlobalGroup() : group_size_(0) {
            add_group(0);
        }

        T* find(uint64_t id) {
            uint64_t obj_id = id & 0xFFFFFFFFULL;
            uint64_t block_id = obj_id >> BASE;
            uint64_t group_id = (block_id >> BASE) - 1;
            return group_list_[group_id].load(std::memory_order_acquire)->
                block_list[block_id & BASE_MARK].load(std::memory_order_acquire)->
                items + (obj_id & BASE_MARK);
        }

        ObjectBlock* add_block(uint64_t* block_id) {
            ObjectBlock* ret = new ObjectBlock;
            uint64_t group_size;
            do {
                group_size = group_size_.load(std::memory_order_acquire);
                BlockGroup* group_ptr = group_list_[group_size - 1].load(std::memory_order_acquire);

                const size_t block_size = group_ptr->block_size.fetch_add(1, std::memory_order_relaxed);
                if (block_size < N) {
                    group_ptr->block_list[block_size].store(ret, std::memory_order_release);
                    *block_id = ((group_size << BASE) + block_size) << BASE;
                    return ret;
                }

                group_ptr->block_size.fetch_sub(1, std::memory_order_relaxed);
            } while (add_group(group_size));

            assert(0);
            delete ret;
            *block_id = 0;
            return nullptr;
        }

        bool add_group(uint64_t old_group_size) {
            std::unique_lock<std::mutex>    mutex_gurad(group_mutex_);
            uint64_t group_size = group_size_.load(std::memory_order_acquire);
            if (group_size_ != old_group_size) {
                return true;
            }

            if (group_size < NG) {
                group_list_[group_size].store(new BlockGroup, std::memory_order_release);
                group_size_.store(group_size + 1, std::memory_order_release);
                return true;
            }
            return false;
        }

    private:
        std::atomic<BlockGroup*>           group_list_[NG];
        std::atomic<size_t>                group_size_;
        std::mutex                         group_mutex_;
    };

    static GlobalGroup              global_group_;
};

template<typename T>
typename ObjectArena<T>::GlobalGroup ObjectArena<T>::global_group_;

} // end namespace base
} // end namespace p

