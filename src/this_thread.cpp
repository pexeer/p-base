// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/base/this_thread.h"

#include "p/base/port.h"
#include <stdio.h>

namespace p {
namespace base {

ThisThread::ThisThread() {
    thread_id_ = gettid();
    thread_name_len_ = snprintf(thread_name_, sizeof(thread_name_), "%5d ", thread_id_) - 1;
}

thread_local ThisThread tls_this_thread;

} // end namespace base
} // end namespace p
