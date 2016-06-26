// Copyright (c) 2015, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#if defined(__GNUC__) && __GNUC__ >= 4
    #define LIKELY(x)   (__builtin_expect((x), 1))
    #define UNLIKELY(x) (__builtin_expect((x), 0))
#else
    #define LIKELY(x)   (x)
    #define UNLIKELY(x) (x)
#endif

#define DISALLOW_COPY(TypeName)                 \
    private:                                    \
    TypeName(const TypeName&)  = delete;        \
    TypeName(const TypeName&&) = delete;        \
    void operator=(const TypeName&) = delete;   \
    void operator=(const TypeName&&) = delete
