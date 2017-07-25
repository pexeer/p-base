// Copyright (c) 2015, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/base/timer.h"
#include "p/base/object_pool.h"
#include "p/base/port.h"
#include "p/base/this_thread.h"
#include <chrono>
#include <vector>

namespace p {
namespace base {

typedef base::ArenaObjectPool<TimerControl::Timer> TimerFactory;

struct P_CACHELINE_ALIGNMENT TimerControl::Timer {
    Timer *next;
    uint64_t run_time;
    TimerId timer_id;
    void (*func)(void *);
    void *arg;

    std::atomic<uint32_t> version;

    bool try_delete() {
        uint32_t base_version = timer_id >> 32;
        if (version.load(std::memory_order_relaxed) != base_version) {
            assert(version == (base_version + 2));
            // timer_id += 0x300000000;
            TimerFactory::release(timer_id);
            return true;
        }

        return false;
    }

    bool run_and_delete() {
        uint32_t base_version = timer_id >> 32;
        if (version.compare_exchange_strong(base_version, base_version + 1,
                                            std::memory_order_release, std::memory_order_relaxed)) {
            // will run this Timer
            func(arg);
            version.store(base_version + 2, std::memory_order_relaxed);
            TimerFactory::release(timer_id);

            return true;
        }

        // Timer canceled by others
        assert(version.load() == (base_version + 2));
        TimerFactory::release(timer_id);

        return false;
    }

    bool operator<(const Timer &rls) { return run_time > rls.run_time; }
};

inline bool timer_compare(const TimerControl::Timer *a, const TimerControl::Timer *b) {
    return a->run_time > b->run_time;
}

TimerControl::TimerBucket::TimerBucket()
    : head_(nullptr), min_timestamp_(std::numeric_limits<int64_t>::max()) {}

inline TimerControl::Timer *TimerControl::TimerBucket::reset() {
    std::unique_lock<std::mutex> mutex_guard(mutex_);
    Timer *ret = head_;
    head_ = nullptr;
    min_timestamp_ = std::numeric_limits<int64_t>::max();
    return ret;
}

inline bool TimerControl::TimerBucket::add_timer(Timer *tm) {
    std::unique_lock<std::mutex> mutex_guard(mutex_);
    tm->next = head_;
    head_ = tm;
    if (tm->run_time < min_timestamp_) {
        min_timestamp_ = tm->run_time;
        return false;
    }
    return true;
}

TimerControl::TimerThread::TimerThread(uint32_t bucket_signal_pending)
    : bucket_number_(bucket_signal_pending) {
    if (bucket_number_ == 0) {
        bucket_number_ = 1;
    }

    bucket_array_ = new TimerBucket[bucket_number_];
    min_timestamp_.store(std::numeric_limits<int64_t>::max());

    stop_.store(0);
    thread_ = std::thread(&TimerThread::Run, this);
}

TimerControl::TimerThread::~TimerThread() {
    stop_and_join();
    Timer *tmp;
    for (size_t i = 0; i < bucket_number_; ++i) {
        Timer *tm_list = bucket_array_[i].reset();
        while (tm_list) {
            tmp = tm_list;
            tm_list = tm_list->next;
            TimerFactory::release(tmp->timer_id);
        }
    }

    delete[] bucket_array_;
}

void TimerControl::TimerThread::stop_and_join() {
    if (thread_.joinable()) {
        stop_.store(1);
        futex_wake(1);
        thread_.join();
    }
}

void TimerControl::TimerThread::Run() {
    // LOG_INFO << "TimerThread started.";
    std::vector<TimerControl::Timer *> tm_heap;
    tm_heap.reserve(4096);

    Timer *tmp;
    while (stop_.load(std::memory_order_relaxed) == 0) {
        // reset min_timestamp_ to max
        min_timestamp_.store(std::numeric_limits<int64_t>::max());

        // pull all task from all bucket
        for (size_t i = 0; i < bucket_number_; ++i) {
            Timer *tm_list = bucket_array_[i].reset();
            while (tm_list) {
                tmp = tm_list;
                tm_list = tm_list->next;

                if (!tmp->try_delete()) {
                    tm_heap.push_back(tmp);
                    std::push_heap(tm_heap.begin(), tm_heap.end(), timer_compare);
                }
            }
        }

        while (tm_heap.size()) {
            tmp = tm_heap[0];
            if (tmp->try_delete()) {
                std::pop_heap(tm_heap.begin(), tm_heap.end(), timer_compare);
                tm_heap.pop_back();
                continue;
            }

            if (tmp->run_time > base::gettimeofday_us()) {
                break;
            }

            // run this Timer
            std::pop_heap(tm_heap.begin(), tm_heap.end(), timer_compare);
            tm_heap.pop_back();
            tmp->run_and_delete();
        }

        int signal_pending = signal_pending_.load(std::memory_order_acquire);
        uint64_t min_sleep_point = min_timestamp_.load(std::memory_order_acquire);
        if (tm_heap.size()) {
            tmp = tm_heap[0];
            if (min_sleep_point > tmp->run_time) {
                min_sleep_point = tmp->run_time;
            }
        }

        uint64_t now = base::gettimeofday_us();
        if (min_sleep_point > now) {
            struct timespec abs_time;
            min_sleep_point -= now;
            abs_time.tv_sec = min_sleep_point / 1000000UL;
            abs_time.tv_nsec = min_sleep_point - abs_time.tv_sec * 1000000UL;
            futex_wait(signal_pending, abs_time);
        }
    }

    for (size_t i = 0; i < tm_heap.size(); ++i) {
        tmp = tm_heap[i];
        TimerFactory::release(tmp->timer_id);
    }

    // LOG_INFO << "TimerThread exited.";
    assert(stop_.load());
}

inline void TimerControl::TimerThread::futex_wait(int signal_pending, const timespec &abstime) {
#if !defined(P_OS_LINUX)
    std::unique_lock<std::mutex> lock_gaurd(mutex_);
    if (signal_pending == signal_pending_) {
        auto d = std::chrono::seconds{abstime.tv_sec} + std::chrono::nanoseconds{abstime.tv_nsec};
        std::chrono::system_clock::time_point tp(
            std::chrono::duration_cast<std::chrono::system_clock::duration>(d));
        condition_.wait_until(lock_gaurd, tp);
    }
#else
    p::base::futex_wait((int *)&signal_pending_, signal_pending, &abstime);
#endif
}

inline void TimerControl::TimerThread::futex_wake(int signal_pending) {
#if !defined(P_OS_LINUX)
    std::unique_lock<std::mutex> lock_gaurd(mutex_);
    signal_pending_ += signal_pending;
    condition_.notify_one();
#else
    signal_pending_.fetch_add(signal_pending, std::memory_order_release);
    p::base::futex_wake((int *)&signal_pending_, signal_pending);
#endif
}

inline void TimerControl::TimerThread::add_timer(Timer *tm) {
    uint64_t run_time = tm->run_time;
    if (bucket_array_[ThisThread::thread_id() % bucket_number_].add_timer(tm)) {
        return;
    }

    while (true) {
        uint64_t min_timestamp = min_timestamp_.load(std::memory_order_acquire);
        if (min_timestamp <= run_time) {
            return;
        }
        if (min_timestamp_.compare_exchange_weak(min_timestamp, run_time, std::memory_order_release,
                                                 std::memory_order_relaxed)) {
            break;
        }
    }
    futex_wake(1);
}

TimerId TimerControl::add_timer(void (*func)(void *), void *arg, const timespec &abstime) {
    TimerId timer_id;
    Timer *tm = TimerFactory::acquire(&timer_id);
    timer_id += 0x300000000; // add version
    tm->timer_id = timer_id;
    tm->next = nullptr;
    tm->run_time = timespec_to_us(abstime);
    tm->func = func;
    tm->arg = arg;
    tm->version.store(timer_id >> 32, std::memory_order_release);
    timer_thread_.add_timer(tm);
    // LOG_INFO << "add_run_time=" << tm->run_time;
    return timer_id;
}

int TimerControl::cancel_timer(const TimerId timer_id) {
    Timer *tm = TimerFactory::find(timer_id);
    if (timer_id != tm->timer_id) {
        return -1; // already finished
    }

    uint32_t base_version = timer_id >> 32;

    if (tm->version.compare_exchange_strong(base_version, base_version + 2,
                                            std::memory_order_release, std::memory_order_relaxed)) {
        return 0; // cancel timer success
    }

    if (tm->version.load(std::memory_order_acquire) == (base_version + 1)) {
        return 1; // timer is running
    }

    return -1;
}

void TimerControl::stop_and_join() { timer_thread_.stop_and_join(); }

} // end namespace base
} // end namespace p

