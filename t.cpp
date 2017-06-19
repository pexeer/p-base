#include "p/base/time.h"
#include <stdio.h>


unsigned long long f() {
    return p::base::clock_gettime_us();
}

unsigned long long g() {
    return p::base::gettimeofday_us();
}

int main() {
    printf("%llu\n", f());
    printf("%llu\n", g());
    return 0;
}
