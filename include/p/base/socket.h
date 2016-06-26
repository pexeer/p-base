// Copyright (c) 2016, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "p/base/endpoint.h"

namespace p {
namespace base {

class Socket {
public:
    Socket() {}

    Socket(EndPoint endpoint) {
        fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    }

    ~Socket() {
        if (fd_ > 0) {
            ::close(fd_);
        }
    }

    int fd() { return fd_; }

 private:
    const int fd_ = -1;
private:
    DISALLOW_COPY(Socket);
};

} // end namespace base
} // end namespace p
