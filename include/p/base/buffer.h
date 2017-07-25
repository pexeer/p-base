// Copyright (c) 2015, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once
#include <stdlib.h>

namespace p {
namespace base {

class Buffer {
public:
    Buffer() {}

    ~Buffer() {
        if (buffer_) {
            free(buffer_);
        }
    }

    void reset() {
        free(buffer_);
        buffer_ = nullptr;
        cur_ = nullptr;
        end_ = nullptr;
    }

    size_t size() const { return (cur_ - buffer_); }

    size_t avial() const {
        if (buffer_) {
            return (end_ - cur_ - 1);
        }
        return 0;
    }

    size_t capacity() const { return end_ - buffer_; }

    void clear() { cur_ = buffer_; }

    const char *c_str() {
        if (buffer_) {
            *(cur_ + 1) = '\0';
        }
        return buffer_;
    }

    void append(const char *buf, size_t len) {
        if (len <= avial()) {
            ::memcpy(cur_, buf, len);
            cur_ += len;
            return;
        }

        size_t new_size = size() + len + 1;
        size_t tmp_size = buffer_ ? (end_ - buffer_) : 64;
        while (tmp_size < new_size) {
            tmp_size = tmp_size << 1;
        }
        _resize(tmp_size);
        ::memcpy(cur_, buf, len);
        cur_ += len;
    }

    bool resize() { return false; }

    void reserve(size_t cap) {}

private:
    void _resize(size_t new_size) {
        size_t now_size = size();
        char *tmp = (char *)malloc(new_size);
        ::memcpy(tmp, buffer_, now_size);
        free(buffer_);
        buffer_ = tmp;
        cur_ = buffer_ + now_size;
        end_ = buffer_ + new_size;
    }

private:
    char *buffer_ = nullptr;
    char *cur_ = nullptr;
    char *end_ = nullptr;

    P_DISALLOW_COPY(Buffer);
};

} // end namespace base
} // end namespace p

