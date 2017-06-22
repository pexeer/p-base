#include "p/base/time.h"
#include <stdio.h>


unsigned long long f() {
    return p::base::clock_gettime_us();
}

unsigned long long g() {
    return p::base::gettimeofday_us();
}


int chenzongjia_afjk() {
    int i = 0;
    for (long long j = 0; j < 1000000000; ++j) {
        i += f();
    }
    return i;
}

int main() {
    printf("start_abc_defgllu\n");
    printf("%d\n", chenzongjia_afjk());
    printf("end_fuck_you\n");
    return 0;
}
