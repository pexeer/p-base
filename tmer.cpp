#include "p/base/timer.h"
#include "p/base/logging.h"
#include "p/base/rand.h"
#include <atomic>
#include <unistd.h>

struct Test {
    P_CACHELINE_ALIGNMENT std::atomic<int>      z;
    std::atomic<int>                b;
};

p::base::TimerControl tc(10);

void func(void* x) {
    LOG_INFO << (uint64_t)x;
}

void f(uint64_t fuck) {
    for (int i = 0; i < 1000; ++i) {
        uint64_t id = fuck + i;
        uint64_t ts =  p::base::fast_rand(1000000);
        tc.add_timer_us(func, (void*)id, ts);
    }
}

void f2(uint64_t fuck) {
    for (int i = 0; i < 1000; ++i) {
        uint64_t id = fuck + i;
        uint64_t ts =  p::base::fast_rand(100000000);
        tc.add_timer_us(func, (void*)id, ts);
    }
}

int main() {
    std::vector<std::thread> thread_list;
    for (int i = 0; i < 40; ++i) {
        thread_list.push_back(std::thread(f, i * 10000));
    }

    for (int i = 0; i < 4; ++i) {
        thread_list.push_back(std::thread(f, i * 1000000));
    }

    for (size_t i = 0; i < thread_list.size(); ++i) {
        thread_list[i].join();
    }

    sleep(1);

    LOG_WARN << sizeof(Test);
    return 0;
}
