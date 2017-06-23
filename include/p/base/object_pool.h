// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "p/base/stack.h"
#include "p/base/logging.h"

namespace p {
namespace base {

template <typename T> struct ObjectChunkItemSize {
    constexpr static int   kValue = 256;
};

template <typename T> struct ObjectPoolTraits {
    constexpr static size_t kObjectGroupNumber = 1024;
    constexpr static size_t kObject = 1;
};

template <typename T> class ObjectPool {
public:
    constexpr static int kObjectChunkItemSize = ObjectChunkItemSize<T>::kValue;

    struct ObjectChunk {
        ObjectChunk*        next;
        int                 object_size;
        T*                  object_list[kObjectChunkItemSize];
    };

    T* allocate() {
        if (tls_free_chunk_) {
            if (tls_free_chunk_->object_size > 0) {
                return tls_free_chunk_->object_list[--tls_free_chunk_->object_size];
            }
            global_free_chunk_queue_.push(tls_free_chunk_);
        }

        tls_free_chunk_ = global_full_chunk_queue_.pop();

        if (tls_free_chunk_) {
            tls_free_chunk_->object_size = kObjectChunkItemSize - 1;
            return tls_free_chunk_->object_list[tls_free_chunk_->object_size];
        }

        auto ret = new T;
        return ret;
    }

    void free(T* obj) {
        if (tls_free_chunk_) {
            if (tls_free_chunk_->object_size < kObjectChunkItemSize) {
                tls_free_chunk_->object_list[(tls_free_chunk_->object_size)++] = obj;
                return ;
            } else {
                global_full_chunk_queue_.push(tls_free_chunk_);
            }
        }

        tls_free_chunk_ = allocate_free_object_chunk();
        tls_free_chunk_->object_size = 1;
        tls_free_chunk_->object_list[0] = obj;
    }

private:
    ObjectChunk* allocate_free_object_chunk() {
        ObjectChunk* ret = global_free_chunk_queue_.pop();
        if (ret) {
            return ret;
        }
        return new ObjectChunk;
    }

private:
    thread_local static ObjectChunk*     tls_free_chunk_;
    p::base::LinkedStack<ObjectChunk>    global_full_chunk_queue_;
    p::base::LinkedStack<ObjectChunk>    global_free_chunk_queue_;
};

template<typename T>
thread_local typename ObjectPool<T>::ObjectChunk* ObjectPool<T>::tls_free_chunk_ = nullptr;

} // end namespace base
} // end namespace p

