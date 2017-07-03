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
    //auto now = std::chrono::steady_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
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

#include <typeinfo>

int main() {
    std::cout << std::chrono::high_resolution_clock::period::num << std::endl;
    std::cout << std::chrono::high_resolution_clock::period::den << std::endl;

    auto now = std::chrono::steady_clock::now();
    std::cout << typeid(now).name() << std::endl;
    auto x = now.time_since_epoch();
    std::cout << typeid(x).name() << std::endl;

    std::cout << x.count() << std::endl;
    auto f0 = f();
    auto g0 = g();
    auto h0 = h();
    auto h110 = h11();

    auto a = p::base::steady_clock_us();
    auto b = p::base::high_resolution_clock_us();
    auto c = p::base::system_clock_us();

    std::cout << f0 << std::endl;
    std::cout << g0 << std::endl;
    std::cout << h0 << std::endl;
    std::cout << h110 << std::endl;

    std::cout << '-' << std::endl;
    std::cout << a << std::endl;
    std::cout << b << std::endl;
    std::cout << c << std::endl;

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
