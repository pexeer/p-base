#include "p/base/rand.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <thread>
#include <vector>

constexpr int R_MAX = 17;

int f() {
    int buf[100];
    ::memset(buf, 0, sizeof(buf));
    for (int i = 0; i < 10000; ++i) {
        ++buf[p::base::fast_rand(R_MAX)];
    }
    char str[1000];
    char* p = str;
    for (int i = 0; i < R_MAX; ++i) {
        p += snprintf(p, 100, "%d ", buf[i]);
    }
    p+= snprintf(p, 100, "\n");
    ::write(0, str, p - str);
    return 0;
}

int main() {
    std::vector<std::thread> list;
    for (int i = 0; i < 100; ++i) {
        list.push_back(std::thread(f));
    }
    for (size_t i = 0; i < list.size(); ++i) {
        list[i].join();
    }
    return 0;
}
