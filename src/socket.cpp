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
inline int SocketFd::set_non_block() {
    int flags = ::fcntl(fd_, F_GETFL, 0);
    if (flags > 0) {
        flags |= O_NONBLOCK;
        flags = ::fcntl(fd_, F_SETFL, flags);
    }
    return flags;
}

// set close-on-exec flag for fd_
inline int SocketFd::set_close_on_exec() {
    int flags = ::fcntl(fd_, F_GETFD, 0);
    if (flags > 0) {
        flags |= FD_CLOEXEC;
        flags = ::fcntl(fd_, F_SETFD, flags);
    }
    return flags;
}

inline int SocketFd::set_no_delay() {
    int flag = 1;
    return setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag));
}

int SocketFd::Connect(const EndPoint &endpoint) {
    reset(-1);
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
    int err = 0;
    if (::connect(fd_, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) < 0) {
        err = errno;
    }

    switch (err)
    {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
        // success
        LOG_DEBUG << "connect " << endpoint << " success for error=" << strerror(errno);
        return 0;
    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
        // need retry
        LOG_DEBUG << "connect " << endpoint << " need retry for error=" << strerror(errno);
        return 1;
        break;
    }

    LOG_WARN << "connect " << endpoint << " failed for error=" << strerror(errno);
    return -1;
}

int SocketFd::GetSocketErr() {
    int opt;
    socklen_t len = sizeof(opt);
    if (getsockopt(fd_, SOL_SOCKET, SO_ERROR, &opt, &len) < 0) {
        LOG_DEBUG << this << " SocketFd getsockopt failed error=" << strerror(errno);
        return errno;
    }
    return opt;
}

int SocketFd::Listen(const EndPoint &endpoint) {
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
        LOG_WARN << "bind " << endpoint << " failed for error=" << strerror(errno);
        return false;
    }

    set_non_block();
    set_close_on_exec();

    if (::listen(fd_, SOMAXCONN) < 0) {
        LOG_WARN << "listen " << endpoint << " failed for error=" << strerror(errno);
        return errno;
    }
    return 0;
}

int SocketFd::Accept(SocketFd *new_s) {
    struct sockaddr new_addr;
    socklen_t addrlen = static_cast<socklen_t>(sizeof(new_addr));

    int fd = -1;
#ifdef P_OS_MACOSX
    fd = ::accept(fd_, &new_addr, &addrlen);
    if (fd >= 0) {
        set_non_block();
        set_close_on_exec();
    }
#else
    fd = ::accept4(fd_, &new_addr, &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
    if (fd < 0) {
        LOG_WARN << "accept4 " << fd_ << " failed for error=" << strerror(errno);
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
