#include "p/base/logging.h"
#include "p/base/this_thread.h"

#include <thread>
#include <iostream>
#include <unistd.h>


void f() {
    long long fuck = 1;
    for (int i = 1; i < 102400; ++i) {
        LOG_TRACE << "Hello, world " << i << ",countf=" << fuck;
        fuck += 2 * i;
    }
}

void g() {
    unsigned long long fuck = 1;
    for (int i = 1; i < 102400; ++i) {
        LOG_INFO << "Hello, world " << i << ",countg=" << fuck << fuck << 'x' << (double) 1.212;
        fuck += 3 * i;
    }
}

void f1() {
    unsigned long long fuck = 1;
    for (int i = 1; i < 102400; ++i) {
        LOG_ERROR << "Hello, world " << i << ",countg=" << fuck << fuck << 'x' << (double) 1.212;
        fuck += 3 * i;
        if (i % 100) {
            continue;
        }
        sleep(1);
    }
}

void f2() {
    unsigned long long fuck = 1;
    for (int i = 1; i < 102400; ++i) {
        LOG_WARN << "Hello, world " << i << ",countg=" << fuck << fuck << 'x' << (double) 1.212;
        fuck += 3 * i;
        if (i % 100) {
            continue;
        }
        sleep(1);
    }
}

void f3() {
    unsigned long long fuck = 1;
    for (int i = 1; i < 102400; ++i) {
        LOG_TRACE << "Hello, world " << i << ",countg=" << fuck << p::noflush;
        LOG_WARN << " i wiil flush";
        fuck += 3 * i;
        if (i % 100) {
            continue;
        }
        sleep(1);
    }
}

int main() {
    std::thread a1(f);
    std::thread a2(g);

    std::thread b1(f1);
    std::thread b2(f2);
    std::thread b3(f3);

    b3.join();
    b1.join();
    b2.join();

    a1.join();
    a2.join();
    return 0;
}