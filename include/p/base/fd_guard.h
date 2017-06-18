// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <unistd.h>

namespace p {
namespace base {

class fd_guard {
public:
    fd_guard() : _fd(-1) {}

    explicit fd_guard(int fd) : _fd(fd) {}

    ~fd_guard() {
        if (_fd >= 0) {
            ::close(_fd);
            _fd = -1;
        }
    }

    void reset(int fd) {
        if (_fd >= 0) {
            ::close(_fd);
        }
        _fd = fd;
    }

    int release() {
        const int old_fd = _fd;
        _fd = -1;
        return old_fd;
    }

    operator int() const { return _fd; }

private:
    int _fd;
    P_DISALLOW_COPY(fd_guard);
};

} // end namespace base
} // end namespace p

