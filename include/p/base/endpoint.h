// Copyright (c) 2016, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <netinet/in.h> // in_addr, INADDR_NONE, INADDR_ANY

namespace p {
namespace base {

// convert hostname to structed ip
//  eg. 'www.baidu.com" to in_addr_t, from dns server
//      '10.10.98.98' to in_addr_t, equals to str2ip
in_addr_t hostname2ip(const char *hostname);

// convert a ip address in dotted-decimal fomart to binary
// eg. '10.10.98.98' to in_addr_t
in_addr_t str2ip(const char *ip_str);

// EndPoint is a combination of an IP address and a port number
// sizeof(EndPoint) == 8
class EndPoint {
public:
    EndPoint() { ip_ = INADDR_NONE; }

    // parse 'www.google.com:8080' to EndPoint
    explicit EndPoint(const char *ip_port);

    explicit EndPoint(uint64_t node) : node_(node) {}

    uint64_t node() const { return node_; }

    EndPoint(const char *ip, short port) {
        port_ = port;
        if (ip) {
            ip_ = hostname2ip(ip);
        } else {
            ip_ = s_local_ip;
        }
    }

    EndPoint(in_addr_t ip, unsigned short port) : ip_(ip), port_(port) {}

    explicit operator bool() const { return ip_ != INADDR_NONE; }

    unsigned short port() const { return port_; }

    in_addr_t ip() const { return ip_; }

    static in_addr_t local_ip() { return s_local_ip; }

protected:
    static in_addr_t s_local_ip;

private:
    union {
        uint64_t node_;
        struct {
            // typeof(in_addr.s_addr) is in_addr_t
            in_addr_t ip_;            // net byte order
            unsigned short port_ = 0; // host byte order
        };
    };
};

} // end namespace base
} // end namespace p
