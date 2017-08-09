// Copyright (c) 2016, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "p/base/endpoint.h"
#include "p/base/macros.h" // DISALLOW_COPY
#include <atomic>
#include <sys/socket.h>
#include <unistd.h>

namespace p {
namespace base {

class SocketFd {
public:
    SocketFd() {}

    int Connect(const EndPoint &endpoint);

    int Listen(const EndPoint &endpoint);

    int set_non_block();

    int set_close_on_exec();

    int set_no_delay();

    int Accept(SocketFd *new_s);

    ssize_t Write(const void *buf, size_t count) { return ::write(fd_, buf, count); }

    ssize_t Read(void *buf, size_t count) { return ::read(fd_, buf, count); }

    ~SocketFd() {
        if (fd_ >= 0) {
            ::close(fd_);
        }
    }

    explicit operator bool() const { return fd_ >= 0; }

    int release() {
        int ret = fd_;
        fd_ = -1;
        return ret;
    }

protected:
    int fd_ = -1;

private:
    P_DISALLOW_COPY(SocketFd);
};

} // end namespace base
} // end namespace p
