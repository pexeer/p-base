// Copyright (c) 2015, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/base/timer.h"
#include "p/base/object_pool.h"

namespace p {
namespace base {

struct P_CACHELINE_ALIGNMENT Timer {
    Timer*          next;
    uint64_t        run_time;
    TimerId         timer_id;
    void            (*func)(void*);
    void*           arg;

    std::atomic<uint32_t>   version;
};

typedef base::ArenaObjectPool<Timer>   TimerFactory;

class P_CACHELINE_ALIGNMENT TimerBucket {
public:
    TimerBucket() : head_(nullptr), min_timestamp_(0) {
    }

    uint64_t min_timestamp() {
        std::unique_lock<std::mutex>     mutex_guard(mutex_);
        return min_timestamp_;
    }

    Timer* reset_header() {
        std::unique_lock<std::mutex>     mutex_guard(mutex_);
        Timer* ret = head_;
        head_ = nullptr;
        return ret;
    }

private:
    std::mutex          mutex_;
    Timer*              head_;
    uint64_t            min_timestamp_;
};

class TimerThread {
public:
    TimerThread(uint32_t bucket_number) : bucket_number_(bucket_number) {
        if (bucket_number_ == 0) {
            bucket_number_ = 1;
        }
        bucket_array_ = new TimerBucket[bucket_number_];
    }

    ~TimerThread() {
        stop_and_join();
        delete [] bucket_array_;
    }

    void stop_and_join();

private:
    size_t              bucket_number_;
    TimerBucket*        bucket_array_;

    P_CACHELINE_ALIGNMENT std::atomic<uint64_t>       min_timestamp_;
};


} // end namespace base
} // end namespace p

