// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "p/base/logging.h"
#include "p/base/object_arena.h"
#include "p/base/stack.h"
#include <assert.h>

namespace p {
namespace base {

template <class T> class LockFreeStack {
public:
    LockFreeStack() { head_ = 0; }

    void push(uint64_t value) {
        value += 0x100000000ULL; // version + 1
        T *node = ObjectArena<T>::find(value);

        uint64_t p;
        do {
            p = head_.load(std::memory_order_acquire);
            node->next = p;
        } while (head_.compare_exchange_weak(p, value, std::memory_order_release,
                                             std::memory_order_relaxed) == false);
    }

    uint64_t pop() {
        uint64_t p;
        uint64_t next;
        do {
            p = head_.load(std::memory_order_acquire);
            if (!p) {
                return 0;
            }
            next = ObjectArena<T>::find(p)->next;
        } while (head_.compare_exchange_weak(p, next, std::memory_order_release,
                                             std::memory_order_relaxed) == false);
        return p;
    }

private:
    std::atomic<uint64_t> head_;
};

template <typename T> struct ObjectGroupItemSize { constexpr static int kValue = 256; };

template <typename T> class ObjectPool {
public:
    static T *acquire() { return tls_local_object_group_.acquire(); }

    static T *acquire(void *arg) { return tls_local_object_group_.acquire(arg); }

    static void release(T *obj) { tls_local_object_group_.release(obj); }

private:
    constexpr static int kObjectGroupItemSize = ObjectGroupItemSize<T>::kValue;

    struct ObjectGroup {
        uint64_t next;
        int size;
        T *items[kObjectGroupItemSize];
    };

    class LocalObjectGroup {
    public:
        T *acquire() {
            if (object_group_ptr_) {
                if (object_group_ptr_->size > 0) {
                    return object_group_ptr_->items[--object_group_ptr_->size];
                }
                global_free_object_group_stack_.push(object_group_id_);
            }

            object_group_id_ = global_object_group_stack_.pop();
            if (object_group_id_) {
                object_group_ptr_ = ObjectArena<ObjectGroup>::find(object_group_id_);
                assert(object_group_ptr_->size > 0 &&
                       object_group_ptr_->size <= kObjectGroupItemSize);
                --(object_group_ptr_->size);
                return object_group_ptr_->items[object_group_ptr_->size];
            }
            object_group_ptr_ = nullptr;

            return T::NewThis();
        }

        T *acquire(void *arg) {
            if (object_group_ptr_) {
                if (object_group_ptr_->size > 0) {
                    return object_group_ptr_->items[--object_group_ptr_->size];
                }
                global_free_object_group_stack_.push(object_group_id_);
            }

            object_group_id_ = global_object_group_stack_.pop();
            if (object_group_id_) {
                object_group_ptr_ = ObjectArena<ObjectGroup>::find(object_group_id_);
                assert(object_group_ptr_->size > 0 &&
                       object_group_ptr_->size <= kObjectGroupItemSize);
                --(object_group_ptr_->size);
                return object_group_ptr_->items[object_group_ptr_->size];
            }
            object_group_ptr_ = nullptr;

            return T::NewThis(arg);
        }

        void release(T *obj) {
            if (object_group_ptr_) {
                if (object_group_ptr_->size < kObjectGroupItemSize) {
                    object_group_ptr_->items[(object_group_ptr_->size)++] = obj;
                    return;
                } else {
                    global_object_group_stack_.push(object_group_id_);
                }
            }

            new_free_object_group();

            object_group_ptr_->size = 1;
            object_group_ptr_->items[0] = obj;
        }

        ~LocalObjectGroup() {
            if (object_group_ptr_) {
                if (object_group_ptr_->size > 0) {
                    global_object_group_stack_.push(object_group_id_);
                } else {
                    global_free_object_group_stack_.push(object_group_id_);
                }
            }
        }

        void new_free_object_group() {
            object_group_id_ = global_free_object_group_stack_.pop();
            if (object_group_id_) {
                object_group_ptr_ = ObjectArena<ObjectGroup>::find(object_group_id_);
                return;
            }

            object_group_ptr_ = ObjectArena<ObjectGroup>::acquire(&object_group_id_);

            uint64_t free_id = object_group_id_;
            for (size_t i = 1; i < ObjectArena<ObjectGroup>::N; ++i) {
                global_free_object_group_stack_.push(++free_id);
            }
        }

    private:
        ObjectGroup *object_group_ptr_ = nullptr;
        uint64_t object_group_id_ = 0;
    };

private:
    thread_local static LocalObjectGroup tls_local_object_group_;
    static p::base::LockFreeStack<ObjectGroup> global_object_group_stack_;
    static p::base::LockFreeStack<ObjectGroup> global_free_object_group_stack_;
};

template <typename T>
thread_local typename ObjectPool<T>::LocalObjectGroup ObjectPool<T>::tls_local_object_group_;

template <typename T>
p::base::LockFreeStack<typename ObjectPool<T>::ObjectGroup>
    ObjectPool<T>::global_object_group_stack_;

template <typename T>
p::base::LockFreeStack<typename ObjectPool<T>::ObjectGroup>
    ObjectPool<T>::global_free_object_group_stack_;

template <typename T> struct ArenaObjectGroupItemSize {
    constexpr static int kValue = ObjectArena<T>::N;
};

template <typename T> class ArenaObjectPool {
public:
    static T *acquire(uint64_t *obj_id) { return tls_local_object_group_.acquire(obj_id); }

    static void release(uint64_t obj_id) { tls_local_object_group_.release(obj_id); }

    static T *find(uint64_t obj_id) { return ObjectArena<T>::find(obj_id); }

private:
    constexpr static int kObjectGroupItemSize = ArenaObjectGroupItemSize<T>::kValue;

    struct ObjectGroup {
        uint64_t next;
        int size;
        uint64_t items[kObjectGroupItemSize];
    };

    class LocalObjectGroup {
    public:
        T *acquire(uint64_t *obj_id) {
            if (object_group_ptr_) {
                if (object_group_ptr_->size > 0) {
                    *obj_id = object_group_ptr_->items[--(object_group_ptr_->size)];
                    return ObjectArena<T>::find(*obj_id);
                }
                global_free_object_group_stack_.push(object_group_id_);
            }

            object_group_id_ = global_object_group_stack_.pop();
            if (object_group_id_) {
                object_group_ptr_ = ObjectArena<ObjectGroup>::find(object_group_id_);
                assert(object_group_ptr_->size > 0 &&
                       object_group_ptr_->size <= kObjectGroupItemSize);
                --(object_group_ptr_->size);
                *obj_id = object_group_ptr_->items[object_group_ptr_->size];
                return ObjectArena<T>::find(*obj_id);
            }

            // get a free object group
            new_free_object_group();

            T *items = ObjectArena<T>::acquire(obj_id);
            uint64_t tmp_id = *obj_id;
            object_group_ptr_->size = kObjectGroupItemSize - 1;
            for (size_t i = 1; i < kObjectGroupItemSize; ++i) {
                object_group_ptr_->items[i - 1] = tmp_id + i;
            }
            return items;
        }

        void release(uint64_t obj_id) {
            if (object_group_ptr_) {
                if (object_group_ptr_->size < kObjectGroupItemSize) {
                    object_group_ptr_->items[(object_group_ptr_->size)++] = obj_id;
                    return;
                } else {
                    global_object_group_stack_.push(object_group_id_);
                }
            }

            new_free_object_group();

            object_group_ptr_->size = 1;
            object_group_ptr_->items[0] = obj_id;
        }

        ~LocalObjectGroup() {
            if (object_group_ptr_) {
                if (object_group_ptr_->size > 0) {
                    global_object_group_stack_.push(object_group_id_);
                } else {
                    global_free_object_group_stack_.push(object_group_id_);
                }
            }
        }

        void new_free_object_group() {
            object_group_id_ = global_free_object_group_stack_.pop();
            if (object_group_id_) {
                object_group_ptr_ = ObjectArena<ObjectGroup>::find(object_group_id_);
                return;
            }

            object_group_ptr_ = ObjectArena<ObjectGroup>::acquire(&object_group_id_);

            uint64_t free_id = object_group_id_;
            for (size_t i = 1; i < ObjectArena<ObjectGroup>::N; ++i) {
                global_free_object_group_stack_.push(++free_id);
            }
        }

    private:
        ObjectGroup *object_group_ptr_ = nullptr;
        uint64_t object_group_id_ = 0;
    };

private:
    thread_local static LocalObjectGroup tls_local_object_group_;
    static p::base::LockFreeStack<ObjectGroup> global_object_group_stack_;
    static p::base::LockFreeStack<ObjectGroup> global_free_object_group_stack_;
};

template <typename T>
thread_local
    typename ArenaObjectPool<T>::LocalObjectGroup ArenaObjectPool<T>::tls_local_object_group_;

template <typename T>
p::base::LockFreeStack<typename ArenaObjectPool<T>::ObjectGroup>
    ArenaObjectPool<T>::global_object_group_stack_;

template <typename T>
p::base::LockFreeStack<typename ArenaObjectPool<T>::ObjectGroup>
    ArenaObjectPool<T>::global_free_object_group_stack_;

} // end namespace base
} // end namespace p

