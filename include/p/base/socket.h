// Copyright (c) 2016, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "p/base/endpoint.h"
#include "p/base/macros.h" // DISALLOW_COPY
#include <sys/socket.h>
#include <unistd.h>

namespace p {
namespace base {

class Socket {
public:
  Socket() {}

  bool Connect(const EndPoint &endpoint);

  bool Listen(const EndPoint &endpoint);

  bool Accept(Socket &new_s);

  ssize_t Write(const void *buf, size_t count) {
    return ::write(fd_, buf, count);
  }

  ssize_t Read(void *buf, size_t count) { return ::read(fd_, buf, count); }

  ~Socket() {
    if (fd_ >= 0) {
      ::close(fd_);
    }
  }

  explicit operator bool() const { return fd_ >= 0; }

private:
  int fd_ = -1;
  int st_;

private:
  DISALLOW_COPY(Socket);
};

} // end namespace base
} // end namespace p
