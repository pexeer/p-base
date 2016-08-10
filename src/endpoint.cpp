// Copyright (c) 2016, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/base/endpoint.h"

#include <stdio.h>
#include <unistd.h>     // gethostname
#include <stdlib.h>     // strtol
#include <string.h>     // memcpy
#include <ctype.h>      // isspace
#include <netdb.h>      // gethostbyname_r
#include <arpa/inet.h>  // inet_pton

namespace p {
namespace base {

in_addr_t EndPoint::s_local_ip = INADDR_NONE;

in_addr_t hostname2ip(const char *hostname) {
    // skip heading space
    for (; isspace(*hostname); ++hostname) {}

#ifdef OS_LINUX
    char buf[1024];
    int error = 0;
    struct hostent ent;
    struct hostent *result = NULL;
    if (gethostbyname_r(hostname, &ent, buf, sizeof(buf),
                        &result, &error) != 0 || result == NULL) {
        return INADDR_NONE;
    }
    // get first ip
    return ((struct in_addr *)(result->h_addr))->s_addr;
#else
    struct hostent *result = gethostbyname(hostname);
    if (result) {
        // get first ip
        return ((struct in_addr*)(result->h_addr))->s_addr;
    }
#endif
    return INADDR_NONE;
}

in_addr_t str2ip(const char *ip_str) {
    for (; isspace(*ip_str); ++ip_str) {}
    struct in_addr ip;
    if (inet_pton(AF_INET, ip_str, &ip) > 0) {
        return ip.s_addr;
    }
     return INADDR_NONE;
}

EndPoint::EndPoint(const char *ip_port) {
    for (; isspace(*ip_port); ++ip_port) {}
    char buf[512];
    char *ip = buf;
    while (*ip_port) {
        if (*ip_port == ':') {
            *ip = '\0';
            ip_ = hostname2ip(buf);
            if (INADDR_NONE == ip_) {
                return ;
            }
            const char *port_str = ++ip_port;
            while (*ip_port) { ++ip_port; }
            char *endptr = nullptr;
            long port = strtol(port_str, &endptr, 10);
            if (endptr == port_str) {
                break;
            } else if (*endptr) {
                for (++endptr; isspace(*endptr); ++endptr) {}
                if (*endptr) {
                    break;
                }
            }
            if (port < 0 || port > 65535) {
                break;
            }
            port_ = port;
            return ;
        }
        *ip = *ip_port;
        ip++;
        ip_port++;
    }
    ip_ = INADDR_NONE;
}

class Dummy {
public:
    Dummy() {
        char buf[512];
        if (gethostname(buf, sizeof(buf)) < 0) {
            return ;
        }
        printf("get ip = %s\n", buf);
        EndPoint::s_local_ip = hostname2ip("");
    }
} dummy;

} // end namespace base
} // end namespace p

