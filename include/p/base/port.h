// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "p/base/macros.h"
#include <unistd.h>                 // SYS_gettid
#include <stdlib.h>
#include <sys/syscall.h>            // syscall

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

}
}

