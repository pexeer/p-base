// Copyright (c) 2015, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <atomic>
#include <mutex>
#include <stdint.h>
#include <sys/time.h>
#include <thread>
#include <time.h>

#include "p/base/logging.h"
#include "p/base/macros.h"
#include "p/base/time.h"

namespace p {
namespace base {

// TimerId is valid if (TimerId != 0)
typedef uint64_t TimerId;

class TimerControl {
public:
    TimerControl(uint32_t bucket_number) : timer_thread_(bucket_number) {}

    // return TimerId != 0 when add_timer success
    TimerId add_timer(void (*func)(void *), void *arg, const timespec &abstime);

    // return TimerId != 0 when success
    TimerId add_timer_us(void (*func)(void *), void *arg, uint64_t delay_us) {
        timespec abstime = us_to_timespec(gettimeofday_us() + delay_us);
        return add_timer(func, arg, abstime);
    }

    // caller must ensure TimerId is valid and returned by add_timer* interface
    // return = 0 -  cancel the TimerId success, the Timer not run yet.
    // return > 0 -  target Timer is running.
    // return < 0 - target Timer is not added or just finished.
    int cancel_timer(const TimerId timer_id);

    void stop_and_join();

public:
    struct Timer;

    class P_CACHELINE_ALIGNMENT TimerBucket {
    public:
        TimerBucket();

        Timer *reset();

        bool add_timer(Timer *tm);

    private:
        std::mutex mutex_;
        Timer *head_;
        uint64_t min_timestamp_;
    };

    class TimerThread {
    public:
        TimerThread(uint32_t bucket_number);

        ~TimerThread();

        void add_timer(Timer *tm);

        void stop_and_join();

        void Run();

        void futex_wait(int signal_pending, const timespec &abstime);

        void futex_wake(int signal_pending);

    private:
        size_t bucket_number_;
        TimerBucket *bucket_array_;
        std::atomic<int> stop_;
        std::thread thread_;

        P_CACHELINE_ALIGNMENT
        std::atomic<uint64_t> min_timestamp_;
        std::atomic<int> signal_pending_;
#if !defined(P_OS_LINUX)
        P_CACHELINE_ALIGNMENT
        std::mutex mutex_;
        std::condition_variable condition_;
#endif
    };

private:
    TimerThread timer_thread_;

private:
    P_DISALLOW_COPY(TimerControl);
};

} // end namespace base
} // end namespace p

