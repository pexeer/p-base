// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <assert.h>
#include "p/base/stack.h"
#include "p/base/logging.h"

namespace p {
namespace base {

template <typename T> struct ObjectGroupItemSize {
    constexpr static int   kValue = 256;
};

template <typename T> class ObjectPool {
public:
    T* get() {
        return tls_local_object_group_.get();
    }

    void put(T* obj) {
        tls_local_object_group_.put(obj);
    }

private:
    constexpr static int kObjectGroupItemSize = ObjectGroupItemSize<T>::kValue;

    struct ObjectGroup {
        ObjectGroup*        next;
        int                 size;
        T*                  items[kObjectGroupItemSize];
    };

    class LocalObjectGroup {
    public:
        T * get() {
            if (object_group_ptr_) {
                if (object_group_ptr_->size > 0) {
                    return object_group_ptr_->items[--object_group_ptr_->size];
                }
                global_free_object_group_stack_.push(object_group_ptr_);
            }

            object_group_ptr_= global_object_group_stack_.pop();
            if (object_group_ptr_) {
                assert(object_group_ptr_->size > 0 &&
                        object_group_ptr_->size <= kObjectGroupItemSize);
                --(object_group_ptr_->size);
                return object_group_ptr_->items[object_group_ptr_->size];
            }

            return T::NewThis();
        }

        void put(T* obj) {
            if (object_group_ptr_) {
                if (object_group_ptr_->size < kObjectGroupItemSize) {
                    object_group_ptr_->items[(object_group_ptr_->size)++] = obj;
                    return ;
                } else {
                    global_object_group_stack_.push(object_group_ptr_);
                }
            }

            object_group_ptr_ = global_free_object_group_stack_.pop();
            if (!object_group_ptr_) {
                object_group_ptr_ = new(std::nothrow) ObjectGroup;
            }
            object_group_ptr_->size = 1;
            object_group_ptr_->items[0] = obj;
        }

        ~LocalObjectGroup() {
            if (object_group_ptr_) {
                if (object_group_ptr_->size > 0) {
                    global_object_group_stack_.push(object_group_ptr_);
                } else {
                    global_free_object_group_stack_.push(object_group_ptr_);
                }
            }
        }

    private:
        ObjectGroup*    object_group_ptr_ = nullptr;
    };

private:
    thread_local static LocalObjectGroup     tls_local_object_group_;
    static p::base::LinkedStack<ObjectGroup>    global_object_group_stack_;
    static p::base::LinkedStack<ObjectGroup>    global_free_object_group_stack_;
};

template<typename T>
thread_local typename ObjectPool<T>::LocalObjectGroup
ObjectPool<T>::tls_local_object_group_;

template<typename T>
p::base::LinkedStack<typename ObjectPool<T>::ObjectGroup>
ObjectPool<T>::global_object_group_stack_;

template<typename T>
p::base::LinkedStack<typename ObjectPool<T>::ObjectGroup>
ObjectPool<T>::global_free_object_group_stack_;

} // end namespace base
} // end namespace p

