// Copyright (c) 2015, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <sys/time.h>
#include <stdint.h>
#include <time.h>

#include "p/base/macros.h"

namespace p {
namespace base {

// TimerId is valid if (TimerId != 0)
typedef uint64_t TimerId;

class TimerControl {
public:
    TimerControl(uint32_t bucket_number);

    // return TimerId != 0 when add_timer success
    TimerId add_timer(void (*func)(void*), void* arg, const timespec& abstime);

    // return TimerId != 0 when success
    TimerId add_timer_us(void (*fn)(void*), void* arg, uint64_t delay_us);

    // caller must ensure TimerId is valid and returned by add_timer* interface
    // return = 0 -  cancel the TimerId success, the Timer not run yet.
    // return > 0 -  target Timer is running or finished.
    int cancel_timer(TimerId id);

    int stop_and_join();

private:

private:
    P_DISALLOW_COPY(TimerControl);
};

} // end namespace base
} // end namespace p

