#include "p/base/rand.h"
#include <iostream>

int buf[100];

constexpr int R_MAX = 17;
int main() {
    for (int i = 0; i < 1000; ++i) {
        ++buf[p::base::fast_rand(R_MAX)];
    }
    for (int i = 0; i < R_MAX; ++i) {
        std::cout << buf[i] << std::endl;
    }
    return 0;
}
