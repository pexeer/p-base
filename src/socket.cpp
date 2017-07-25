// Copyright (c) 2016, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/base/socket.h"
#include <errno.h>  // errno
#include <fcntl.h>  // fctnl
#include <string.h> // memset

#include <netinet/in.h>  // IPPROTO_TCP
#include <netinet/tcp.h> // TCP_NODELAY
#include <sys/socket.h>  // setsockopt

namespace p {
namespace base {

// set non-block flag for fd_
inline int Socket::set_non_block() {
    int flags = ::fcntl(fd_, F_GETFL, 0);
    if (flags > 0) {
        flags |= O_NONBLOCK;
        flags = ::fcntl(fd_, F_SETFL, flags);
    }
    return flags;
}

// set close-on-exec flag for fd_
inline int Socket::set_close_on_exec() {
    int flags = ::fcntl(fd_, F_GETFD, 0);
    if (flags > 0) {
        flags |= FD_CLOEXEC;
        flags = ::fcntl(fd_, F_SETFD, flags);
    }
    return flags;
}

inline int Socket::set_no_delay() {
    int flag = 1;
    return setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag));
}

int Socket::Connect(const EndPoint &endpoint) {
    fd_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd_ < 0) {
        return errno;
    }
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(endpoint.port());
    servaddr.sin_addr.s_addr = endpoint.ip();

    set_non_block();
    set_close_on_exec();
    if (::connect(fd_, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) < 0) {
        return errno;
    }
    return 0;
}

int Socket::Listen(const EndPoint &endpoint) {
    fd_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd_ < 0) {
        return errno;
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
        return errno;
    }
    return 0;
}

int Socket::Accept(Socket *new_s) {
    struct sockaddr_in new_addr;
    memset(&new_addr, 0, sizeof(new_addr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(new_addr));

    int fd = -1;
#ifdef P_OS_MACOSX
    fd = ::accept(fd_, (struct sockaddr *)&new_addr, &addrlen);
    if (fd >= 0) {
        set_non_block();
        set_close_on_exec();
    }
#else
    fd = ::accept4(fd_, (struct sockaddr *)&new_addr, &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
    if (fd < 0) {
        return errno;
    }

    if (new_s->fd_ >= 0) {
        ::close(new_s->fd_);
    }
    new_s->fd_ = fd;
    return 0;
}

} // end namespace base
} // end namespace p
