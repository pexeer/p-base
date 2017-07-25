// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "p/base/macros.h"

#if defined(P_OS_LINUX)
#include <error.h>       // strerror
#include <sys/syscall.h> // syscall
#include <unistd.h>      // SYS_gettid
#else
#include <thread> // pthread_threadid_np
#endif

namespace p {
namespace base {

inline int gettid() {
#if defined(P_OS_LINUX)
    // gettid()
    return static_cast<int>(::syscall(SYS_gettid));
#else
    // pthread_t pthread_self()
    uint64_t thread_id;
    ::pthread_threadid_np(nullptr, &thread_id);
    return static_cast<int>(thread_id);
#endif
}

#if defined(P_OS_LINUX)
#include <linux/futex.h>
#include <sys/syscall.h>

inline long sys_futex(void *addr1, int op, int val1, struct timespec *timeout, void *addr2,
                      int val3) {
    return syscall(SYS_futex, addr1, op, val1, timeout, addr2, val3);
}

inline int futex_wait(int *uaddr, int expected, const struct timespec *timeout) {
    return syscall(SYS_futex, uaddr, FUTEX_WAIT_PRIVATE, expected, timeout, nullptr, 0);
}

inline int futex_wake(int *uaddr, int wake_num) {
    return syscall(SYS_futex, uaddr, FUTEX_WAKE_PRIVATE, wake_num, nullptr, nullptr, 0);
}

#endif
}
}

