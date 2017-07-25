// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "p/base/logging.h"
#include <atomic>
#include <mutex>

namespace p {
namespace base {

#if 0
template <class T> class LinkedStack {
public:
  LinkedStack() {
    head_ = nullptr;
  }

  void push(T *node) {
    T *p;
    do {
        p = head_.load(std::memory_order_acquire);
        node->next = p;
    } while (head_.compare_exchange_weak(p, node, std::memory_order_release,
                                         std::memory_order_relaxed) == false);
  }

  T *pop() {
    T *p;
    do {
      p = head_.load(std::memory_order_acquire);
      if (p == nullptr) {
        return nullptr;
      }
    } while (head_.compare_exchange_weak(p, p->next, std::memory_order_release,
                                            std::memory_order_relaxed) == false);
    return p;
  }

private:
  std::atomic<T *> head_;
};
#endif

template <class T> class LinkedStack {
public:
    LinkedStack() { head_ = nullptr; }

    void push(T *node) {
        std::lock_guard<std::mutex> lock_guard(mutex_);
        node->next = head_;
        head_ = node;
    }

    T *pop() {
        std::lock_guard<std::mutex> lock_guard(mutex_);
        if (head_) {
            T *tmp = head_;
            head_ = head_->next;
            return tmp;
        }
        return nullptr;
    }

private:
    T *head_;
    std::mutex mutex_;
};

} // end namespace base
} // end namespace p
