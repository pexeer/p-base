// Copyright (c) 2016, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <unistd.h>
#include <sys/socket.h>
#include "p/base/macros.h"      // DISALLOW_COPY
#include "p/base/endpoint.h"

namespace p {
namespace base {

class Socket {
public:
    Socket() {}

    bool connect(const EndPoint& endpoint);

    bool listen(const EndPoint& endpoint);

    bool accept(Socket& new_s);

    ssize_t write(const void *buf, size_t count) {
        return ::write(fd_, buf, count);
    }

    ssize_t read(void *buf, size_t count) {
        return ::read(fd_, buf, count);
    }

    ~Socket() {
        if (fd_ >= 0) {
            ::close(fd_);
        }
    }

    operator bool() const {
        return fd_ >= 0;
    }

 private:
    int fd_ = -1;
    int st_;
private:
    DISALLOW_COPY(Socket);
};

} // end namespace base
} // end namespace p
