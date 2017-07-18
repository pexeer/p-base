#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "p/base/endpoint.h"
#include "p/base/socket.h"
#include <string>
#include <signal.h>
#include <assert.h>

int main() {
    p::base::EndPoint endpoint;
    printf("sizeof(EndPoint) == %lu\n", sizeof(endpoint));
    if (endpoint) {
        printf("endpoint true\n");
    } else {
        printf("endpoint false\n");
    }

    p::base::EndPoint edx("   www.baidu.com:8001 ");
    if (edx) {
        printf("ed0 true\n");
    } else {
        printf("ed0 false\n");
    }

    p::base::EndPoint ed0("www.baidu.com:8001");
    if (ed0) {
        printf("ed0 true\n");
    } else {
        printf("ed0 false\n");
    }

    p::base::EndPoint ed1("www.baidu.com:8001  ");
    if (ed1) {
        printf("ed1 true\n");
    } else {
        printf("ed1 false\n");
    }

    p::base::EndPoint ed2("www.baidu.com 8001");
    if (ed2) {
        printf("ed2 true\n");
    } else {
        printf("ed2 false\n");
    }

    p::base::EndPoint ed3("www.baidu.com:8001  9");
    if (ed3) {
        printf("ed3 true\n");
    } else {
        printf("ed3 false\n");
    }

    p::base::EndPoint ed4("www.baidu.com:");
    if (ed4) {
        printf("ed4 true\n");
    } else {
        printf("ed4 false\n");
    }

    p::base::EndPoint ed5("192.34:104");
    if (ed5) {
        printf("ed5 true\n");
    } else {
        printf("ed5 false\n");
    }

    p::base::Socket x;

    p::base::EndPoint tmp("127.0.0.1", 9099);

        // Ignore SIGPIPE.~
    struct sigaction oldact;
    if (sigaction(SIGPIPE, NULL, &oldact) != 0 ||
            (oldact.sa_handler == NULL && oldact.sa_sigaction == NULL)) {
        assert(nullptr == signal(SIGPIPE, SIG_IGN));
    }
#if 1
    if (tmp) {
        if (0 == x.Connect(tmp)) {
            printf("conect ok\n");
        } else {
            printf("conect failed\n");
        }
    } else {
        printf("failed endpoint.\n");
        return 1;
    }

    std::string buf(1024000, 'a');

    while (true) {
        ssize_t ret = x.Write(buf.data(), buf.size());
        printf("%ld\n", ret);
        sleep(1);
    }

#endif
    return 0;
}
