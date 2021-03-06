// Copyright (c) 2015, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.
//

#pragma once
#include "p/base/macros.h" // DISALLOW_COPY
#include <stdarg.h>        // va_list
#include <stdio.h>         // vsnprintf
#include <string.h>        // memcpy, strlen

namespace p {
namespace base {

// fixed buffer class, mostly used to allocate a buffer from
// stack or global static space. if want to allocate a buffer
// from heap, p::base::Buffer or std::string is better
template <int N> class FixedBuffer {
public:
    FixedBuffer() {}

    void reset() { cur_ = data_; }

    int append(const char *buf, int len) {
        if (UNLIKELY(len > avial())) {
            len = avial();
        }

        if (UNLIKELY(len <= 0)) {
            return 0;
        }

        ::memcpy(cur_, buf, len);
        cur_ += len;
        return len;
    }

    int append(char ch) {
        if (avial() > 0) {
            *cur_++ = ch;
            return 1;
        }
        return 0;
    }

    int append(const char *str) { return append(str, strlen(str)); }

    int appendf(const char *fmt, ...) {
        va_list argptr;
        va_start(argptr, fmt);
        const int ret = appendf(fmt, argptr);
        va_end(argptr);
        return ret;
    }

    int appendf(const char *fmt, va_list argptr) {
        int n = avial();
        if (n <= 0) {
            return 0;
        }
        int ret = ::vsnprintf(cur_, n + 1, fmt, argptr);
        if (ret < 0) {
            return 0;
        }
        if (ret > n) {
            ret = n;
        }
        cur_ += ret;
        return ret;
    }

    char *cur() { return cur_; }

    void add(size_t len) { cur_ += len; }

    const char *data() const { return data_; }

    const char *begin() const { return data_; }

    const char *end() const { return cur_; }

    bool empty() const { return cur_ == data_; }

    const int length() const { return static_cast<int>(cur_ - data_); }

    const int avial() const {
        // reserve a byte for NULL
        return static_cast<int>(reinterpret_cast<const char *>(&cur_) - cur_ - 1);
    }

    const char *c_str() const {
        // has reversed a byte for this NULL
        *cur_ = 0;
        return data_;
    }

    int max_size() const { return N; }

protected:
    char data_[N];
    char *cur_ = data_;
    P_DISALLOW_COPY(FixedBuffer);
};

static constexpr int kSmallBuffer = 4000;
static constexpr int kLargeBuffer = 4000 * 1024;
typedef FixedBuffer<kSmallBuffer> SmallFixedBuffer;
typedef FixedBuffer<kLargeBuffer> LargeFixedBuffer;

} // end namespace base
} // end namespace p
