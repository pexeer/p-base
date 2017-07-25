// Copyright (c) 2015, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <chrono>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>

namespace p {
namespace base {

inline uint64_t timespec_to_us(const struct timespec &tp) {
    return tp.tv_sec * 1000000ULL + tp.tv_nsec / 1000ULL;
}

inline uint64_t timespec_to_ns(const struct timespec &tp) {
    return tp.tv_sec * 1000000000ULL + tp.tv_nsec;
}

inline struct timespec ns_to_timespec(uint64_t ts) {
    struct timespec tp;
    tp.tv_sec = ts / 1000000000ULL;
    tp.tv_nsec = ts - 1000000000ULL * tp.tv_sec;
    return tp;
}

inline struct timespec us_to_timespec(uint64_t ts) { return ns_to_timespec(ts * 1000ULL); }

inline uint64_t gettimeofday_us() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000000ULL + tv.tv_usec;
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

