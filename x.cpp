#include <stdio.h>
#include "p/base/logging.h"

int main(void) {
    double x = 1.0;
    long double y = 32.0;
    const char* zz = "fuckz";
    const void* z = zz;
    printf("z=%p\n", z);
    p::base::Logger(0).stream() << " x=" << x << " y=" << y << " z=" << z;
    p::base::Logger(1).stream() << " x=" << x << " y=" << y << " z=" << z;
    p::base::Logger(2).stream() << " x=" << x << " y=" << y << " z=" << z;
    p::base::Logger(3).stream() << " x=" << x << " y=" << y << " z=" << z;
    p::base::Logger(4).stream() << " x=" << x << " y=" << y << " z=" << z;
    p::base::Logger(5).stream() << " x=" << x << " y=" << y << " z=" << z;
    p::base::Logger(6).stream() << " x=" << x << " y=" << y << " z=" << z;
    return 0;
}
