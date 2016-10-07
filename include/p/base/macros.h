// Copyright (c) 2015, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#if defined(__GNUC__) && __GNUC__ >= 4
#define LIKELY(x) (__builtin_expect((x), 1))
#define UNLIKELY(x) (__builtin_expect((x), 0))
#else
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#endif

#define DISALLOW_COPY(TypeName)                                                \
private:                                                                       \
  TypeName(const TypeName &) = delete;                                         \
  TypeName(const TypeName &&) = delete;                                        \
  void operator=(const TypeName &) = delete;                                   \
  void operator=(const TypeName &&) = delete

#ifdef __linux__
#define OS_LINUX
#elif __unix || __unix__
#define OS_UNIX
#elif __APPLE__ || __MACH__
#define OS_MAC
#elif __FreeBSD__
#define OS_FREEBSD
#else
#define OS_UNKNOWN
static_assert(false, "detect os type error.");
#endif

#define CACHELINE_SIZE 64
#define CACHELINE_ALIGNMENT __attribute__((aligned(CACHELINE_SIZE)))
#define PTR_ALIGNMENT(ptr) (((uintptr_t)(ptr) + (uintptr_t)(7)) & ~((uintptr_t)(7)))
#define PTR_CACHELINE_ALIGNMENT(ptr) (((uintptr_t)(ptr) + (uintptr_t)(CACHELINE_SIZE - 1))\
    & ~((uintptr_t)(CACHELINE_SIZE - 1)))

