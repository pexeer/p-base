#include "p/base/logging.h"
#include "p/base/this_thread.h"

#include <thread>
#include <iostream>


void f() {
    long long fuck = 1;
    for (int i = 1; i < 10240; ++i) {
        LOG_TRACE << "Hello, world " << i << ",countf=" << fuck;
        fuck += 2 * i;
    }
}

void g() {
    unsigned long long fuck = 1;
    for (int i = 1; i < 10240; ++i) {
        LOG_INFO << "Hello, world " << i << ",countg=" << fuck << fuck << 'x' << (double) 1.212;
        fuck += 3 * i;
    }
}

int main() {
    std::thread a1(f);
    std::thread a2(g);


    a1.join();
    a2.join();
    return 0;
}
