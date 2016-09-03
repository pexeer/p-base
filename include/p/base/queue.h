// Copyright (c) 2016, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <atomic>

namespace p {
namespace base {

template <class T> class LinkedQueue {
public:
  LinkedQueue() {
    dummy_.next = nullptr;
    tail_ = &dummy_;
    head_ = &dummy_;
    dummy_.next = nullptr;
  }

  void push_back(T *node) {
    node->next = nullptr;
    T *p = tail_.load(std::memory_order_relaxed);
    T *old_tail = p;
    T *dummy = nullptr;
    do {
      while (p->next) {
        p = p->next;
      }
      //} while (std::atomic_compare_exchange(p.next, nullptr, node));
    } while (p->next.compare_exchange_weak(dummy, node,
                                           std::memory_order_release,
                                           std::memory_order_relaxed) == false);
    tail_.compare_exchange_weak(old_tail, node, std::memory_order_release,
                                std::memory_order_relaxed);
  }

  T *pop_front() {
    T *p;
    do {
      p = head_.load(std::memory_order_relaxed);
      if (p->next == nullptr) {
        return nullptr;
      }
    } while (head_.compare_exchange_weak(p, p->next, std::memory_order_release,
                                         std::memory_order_relaxed) == false);
    return p;
  }

private:
  T dummy_;
  std::atomic<T *> head_;
  std::atomic<T *> tail_;
};

} // end namespace base
} // end namespace p
