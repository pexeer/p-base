// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/base/this_thread.h"

#include "p/base/port.h"
#include <stdio.h>

namespace p {
namespace base {

namespace ThisThread {

thread_local int thread_id;
thread_local int  thread_name_len;
thread_local char thread_name[32];

class ThisThreadImpl {
public:
    ThisThreadImpl() {
        thread_id = gettid();
        thread_name_len = snprintf(thread_name, sizeof(thread_name), "%5d ", thread_id) - 1;
    }
} this_thread_impl_dummy;

} // end namespace ThisThread

} // end namespace base
} // end namespace p
