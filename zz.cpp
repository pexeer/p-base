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
    p::base::EndPoint tmp("127.0.0.1", 9099);
    p::base::Socket x;

    int ret = x.Listen(tmp);
    printf("Listen ret=%s\n", strerror(ret));

    p::base::Socket s;

    if (x.Accept(&s) >= 0) {
        printf("accpet succsse\n");
        sleep(3);
        ssize_t len = s.Read(buf, 10240000);
        printf("%ld\n", len);
    }

    while (true) {
        sleep(5);
        ssize_t len = s.Read(buf, 10240000);
        printf("%ld\n", len);
    }

    return 0;
}
