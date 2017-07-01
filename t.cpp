#include "p/base/time.h"
#include <stdio.h>
#include <iostream>
#include <chrono>


inline unsigned long long f() {
    return p::base::clock_gettime_ns();
}

inline unsigned long long g() {
    return p::base::gettimeofday_us();
}

inline unsigned long long h() {
    return p::base::steady_clock_gettime_ns();
}

inline unsigned long long h11() {
    auto now = std::chrono::steady_clock::now();
    return now.time_since_epoch().count();
}

inline unsigned long long bench_f() {
    unsigned long long i = 0;
    for (long long j = 0; j < 10000000; ++j) {
        i += f();
    }
    return i;
}

inline unsigned long long bench_g() {
    unsigned long long i = 0;
    for (long long j = 0; j < 10000000; ++j) {
        i += g();
    }
    return i;
}

inline unsigned long long bench_h() {
    unsigned long long i = 0;
    for (long long j = 0; j < 10000000; ++j) {
        i += h();
    }
    return i;
}

inline unsigned long long bench_h11() {
    unsigned long long i = 0;
    for (long long j = 0; j < 10000000; ++j) {
        i += h11();
    }
    return i;
}

int main() {
    auto f0 = f();
    auto g0 = g();
    auto h0 = h();
    auto h110 = h11();
    std::cout << f0 << std::endl;
    std::cout << g0 << std::endl;
    std::cout << h0 << std::endl;
    std::cout << h110 << std::endl;

    {
        auto start = h();
        auto ret = bench_f();
        auto end = h();
        std::cout <<  end - start << "\t" << ret << std::endl;
    }
    {
        auto start = h();
        auto ret = bench_g();
        auto end = h();
        std::cout <<  end - start << "\t" << ret << std::endl;
    }
    {
        auto start = h();
        auto ret = bench_h();
        auto end = h();
        std::cout <<  end - start << "\t" << ret << std::endl;
    }

    {
        auto start = h();
        auto ret = bench_h11();
        auto end = h();
        std::cout <<  end - start << "\t" << ret << std::endl;
    }

    return 0;
}
