// Copyright (c) 2015, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#ifndef __x86_64__
static_assert(false, "only support for architecture x86_64.");
#endif

#ifdef __linux__
#define P_OS_LINUX // P_OS_LINUX
#elif __APPLE__ || __MACH__
#define P_OS_MACOSX // P_OS_MACOSX
#elif __unix || __unix__
#define P_OS_UNIX // P_OS_UNIX
#else
#define P_OS_UNKNOWN // P_OS_UNKNOWN
static_assert(false, "detect os type error.");
#endif

#if defined(__GNUC__) && __GNUC__ >= 4
#define LIKELY(x) (__builtin_expect((x), 1))
#define UNLIKELY(x) (__builtin_expect((x), 0))
#else
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#endif

#define P_DISALLOW_COPY(TypeName)                                                                  \
private:                                                                                           \
    TypeName(const TypeName &) = delete;                                                           \
    TypeName(const TypeName &&) = delete;                                                          \
    void operator=(const TypeName &) = delete;                                                     \
    void operator=(const TypeName &&) = delete

#define P_CACHELINE_SIZE 64U
#define P_CACHELINE_ALIGNMENT alignas(P_CACHELINE_SIZE)
#define P_PTR_ALIGNMENT(ptr) (((uintptr_t)(ptr) + (uintptr_t)(7)) & ~((uintptr_t)(7)))
#define P_PTR_CACHELINE_ALIGNMENT(ptr)                                                             \
    (((uintptr_t)(ptr) + (uintptr_t)(P_CACHELINE_SIZE - 1)) & ~((uintptr_t)(P_CACHELINE_SIZE - 1)))
