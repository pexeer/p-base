// Copyright (c) 2016, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/base/socket.h"
#include <string.h>

namespace p {
namespace base {

bool Socket::connect(const EndPoint &endpoint) {
  fd_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd_ < 0) {
    return false;
  }
  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(endpoint.port());
  servaddr.sin_addr.s_addr = endpoint.ip();

  if (::connect(fd_, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) <
      0) {
    return false;
  }

  return true;
}

bool Socket::listen(const EndPoint &endpoint) {
  fd_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd_ < 0) {
    return false;
  }

  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(endpoint.port());
  servaddr.sin_addr.s_addr = endpoint.ip();
  if (::bind(fd_, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) < 0) {
    return false;
  }

  if (::listen(fd_, SOMAXCONN) < 0) {
    return false;
  }
  return true;
}

} // end namespace base
} // end namespace p
