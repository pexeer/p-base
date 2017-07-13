// Copyright (c) 2015, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <sys/time.h>
#include <stdint.h>
#include <time.h>
#include <chrono>

namespace p {
namespace base {

inline uint64_t gettimeofday_us() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return tv.tv_sec * 1000000UL + tv.tv_usec;
}

inline uint64_t clock_gettime_us() {
  struct timespec tp;
  ::clock_gettime(CLOCK_REALTIME, &tp);
  return tp.tv_sec * 1000000ULL + tp.tv_nsec / 1000ULL;
}

inline uint64_t clock_gettime_ns() {
  struct timespec tp;
  ::clock_gettime(CLOCK_REALTIME, &tp);
  return tp.tv_sec * 1000000000ULL + tp.tv_nsec;
}

inline uint64_t steady_clock_gettime_ns() {
  struct timespec tp;
  ::clock_gettime(CLOCK_MONOTONIC, &tp);
  return tp.tv_sec * 1000000000ULL + tp.tv_nsec;
}

inline uint64_t system_clock_us() {
    auto now = std::chrono::system_clock::now();
    return now.time_since_epoch().count();
}

inline uint64_t steady_clock_us() {
    auto now = std::chrono::steady_clock::now();
    return now.time_since_epoch().count();
}

inline uint64_t high_resolution_clock_us() {
    auto now = std::chrono::high_resolution_clock::now();
    return now.time_since_epoch().count();
}

} // end namespace base
} // end namespace p

