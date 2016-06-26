// Copyright (c) 2016, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include "p/base/endpoint.h"

namespace p {
namespace base {

struct in_addr EndPoint::s_local_ip = { INADDR_NONE };

int hostname2ip(const char *hostname, in_addr *ip) {
    // skip heading space
    for (; isspace(*hostname); ++hostname) {}

    char buf[1024];
    int error = 0;
    struct hostent ent;
    struct hostent *result = NULL;
    if (gethostbyname_r(hostname, &ent, buf, sizeof(buf),
                        &result, &error) != 0 || result == NULL) {
        return -1;
    }
    // copy first ip
    memcpy((char*)ip, (char*)result->h_addr, result->h_length);
    return 0;
}

int str2ip(const char *ip_str, in_addr *ip) {
    for (; isspace(*ip_str); ++ip_str) {}
    if (inet_pton(AF_INET, ip_str, ip) > 0) {
        return 0;
    }
    return -1;
}

EndPoint::EndPoint(const char *ip_port) {
    for (; isspace(*ip_port); ++ip_port) {}
    char buf[512];
    char *ip = buf;
    while (*ip_port) {
        if (*ip_port == ':') {
            *ip = '\0';
            if (hostname2ip(buf, &ip_)) {
                break;
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
    ip_ = { INADDR_NONE };
}

class Dummy {
public:
    Dummy() {
        char buf[512];
        if (gethostname(buf, sizeof(buf)) < 0) {
            return ;
        }
        hostname2ip(buf, &EndPoint::s_local_ip);
    }
} dummy;

} // end namespace base
} // end namespace p

