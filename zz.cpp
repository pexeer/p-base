#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "p/base/endpoint.h"
#include "p/base/socket.h"
#include <string>
#include <error.h>

char buf[10240001];

int main() {
    p::base::EndPoint tmp(nullptr, 9099);
    p::base::Socket x;

    int ret = x.Listen(tmp);
    printf("Listen ret=%s\n", strerror(ret));

    p::base::Socket s;

    ret = x.Accept(&s);
    if (ret) {
        printf("Accept failed,%s\n", strerror(ret));
    } else {
        printf("accpet succsse\n");
        sleep(3);
        ssize_t len = s.Read(buf, 10240000);
        printf("%ld\n", len);
    }

    while (true) {
        sleep(5);
        ssize_t len = s.Read(buf, 10240000);
        if (len >= 0) {
            printf("%ld\n", len);
        } else {
            printf("Read Error,%s\n", strerror(len));
        }
    }

    return 0;
}
