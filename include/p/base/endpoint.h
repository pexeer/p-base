// Copyright (c) 2016, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace p {
namespace base {

// convert hostname to structed ip
//  eg. 'www.baidu.com" to '123.125.114.144'
int hostname2ip(const char *hostname, in_addr *ip);

// convert a ip address in dotted-decimal fomart to binary
// eg. '10.10.98.98' to sturct in_addr
int str2ip(const char *ip_str, in_addr *ip);

// EndPoint is a combination of an IP address and a port number
// sizeof(EndPoint) == 8
class EndPoint {
public:
    EndPoint() {}

    // convert '10.10.98.98:8080' to EndPoint
    explicit EndPoint(const char *ip_port);

    EndPoint(const char *ip, short port) {
        port_ = port;
        if (str2ip(ip, &ip_)) {
            ip_ = { INADDR_NONE };
        }
    }

    explicit operator bool() const {
        return ip_.s_addr != INADDR_NONE;
    }

public:
    static struct in_addr   s_local_ip;
private:
    // typeof(in_addr.s_addr) is in_addr_t
    struct  in_addr         ip_ = { INADDR_NONE };
    unsigned short int      port_ = 0;
};

} // end namespace base
} // end namespace p
