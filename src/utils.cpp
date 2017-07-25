// Copyright (c) 2015, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/base/utils.h"
#include "p/base/status.h"

namespace p {
namespace base {

size_t ConvertPointer(char *buf, const void *value) {
    static const char s_digitsHex[] = "0123456789abcdef";
    uint64_t i = reinterpret_cast<uint64_t>(value);
    char *p = buf;
    int32_t lsd;

    do {
        lsd = static_cast<int>(i % 16);
        i /= 16;
        *p++ = s_digitsHex[lsd];
    } while (i != 0);

    *p = '\0';
    std::reverse(buf, p);
    return p - buf;
}

} // end namespace base
} // end namespace p
