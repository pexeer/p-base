#include "p/base/logging.h"
#include "p/base/this_thread.h"

#include <thread>
#include <iostream>
#include <unistd.h>


void f() {
    long long fuck = 1;
    for (int i = 1; i < 102400; ++i) {
        LOG_TRACE << "FHello, world " << i << ",countf=" << fuck;
        fuck += 2 * i;
    }
}

void g() {
    unsigned long long fuck = 1;
    for (int i = 1; i < 102400; ++i) {
        LOG_INFO << "GHello, world " << i << ",countg=" << fuck << fuck << 'x' << (double) 1.212;
        fuck += 3 * i;
    }
}

void f1() {
    unsigned long long fuck = 1;
    for (int i = 1; i < 102400; ++i) {
        LOG_ERROR << "f1Hello, world " << i << ",countg=" << fuck << fuck << 'x' << (double) 1.212;
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
        LOG_WARN << "f2Hello, world " << i << ",countg=" << fuck << fuck << 'x' << (double) 1.212;
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
        LOG_TRACE << "f3Hello, world " << i << ",countg=" << fuck << p::base::noflush();
        LOG_WARN << " i wiil flush";
        fuck += 3 * i;
        if (i % 100) {
            continue;
        }
        sleep(1);
    }
}

class Fuck {
public:
    Fuck(int x, int y) : a(x), b(y) {}

    p::base::LogStream& Logger(p::base::LogStream& l) const {
        return l << '[' << a << ',' << b << ']';
    }

private:
    int a;
    int b;
};

void fuck_me() {
    Fuck ffff(123, 345);
    LOG_DEBUG << ffff;
    LOG_DEBUG << Fuck(12, 3238293) << Fuck(399434, 12)
            << p::base::HexUint32(0x123455)
            << p::base::HexUint64(0x123455);
}

int main() {
    //fuck_me();
    p::base::Log::set_log_level(p::base::LogLevel::kTrace);
    LOG_DEBUG << "fuck me";


    fuck_me();

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
