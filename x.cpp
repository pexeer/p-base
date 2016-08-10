#include <stdio.h>
#include <arpa/inet.h>
#include "p/base/logging.h"
#include "p/base/buffer.h"
#include "p/base/endpoint.h"

int main(void) {
    in_addr_t ip = p::base::EndPoint::s_local_ip;
    char xbuf[512];
    in_addr real_ip;
    real_ip.s_addr = ip;
    const char * ret = inet_ntop(AF_INET, &real_ip, xbuf, sizeof(xbuf));
    printf("fuck\n%s\n", ret);
    double x = 1.0;
    long double y = 32.0;
    const char* zz = "fuckz";
    const void* z = zz;
    printf("z=%p\n", z);
    p::base::Logger(p::base::LogLevel::TRACE).stream() << " x=" << x << " y=" << y << " z=" << z;
    p::base::Logger(p::base::LogLevel::DEBUG).stream() << " x=" << x << " y=" << y << " z=" << z;
    p::base::Logger(p::base::LogLevel::INFO).stream() << " x=" << x << " y=" << y << " z=" << z;
    p::base::Logger(p::base::LogLevel::WARN).stream() << " x=" << x << " y=" << y << " z=" << z;
    p::base::Logger(p::base::LogLevel::ERROR).stream() << " x=" << x << " y=" << y << " z=" << z;
    p::base::Logger(p::base::LogLevel::FATAL).stream() << " x=" << x << " y=" << y << " z=" << z;
    p::base::Logger(p::base::LogLevel::FATAL).stream() << " x=" << x << " y=" << y << " z=" << z;


    printf("\n");

    p::base::Buffer buf;
    printf("%p\n", buf.c_str());
    for (int i = 0; i < 321; ++i) {
        buf.append("x", 1);
        printf("%s\n", buf.c_str());
    }

    buf.reset();
    for (int i = 0; i < 321; ++i) {
        buf.append("xyz", 3);
        printf("%s\n", buf.c_str());
    }

    if (ret) {
        p::base::Logger(p::base::LogLevel::FATAL).stream() << ret ;
    }
    return 0;
}
