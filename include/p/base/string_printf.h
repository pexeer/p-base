// Copyright (c) 2015, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <cstdarg> // va_list
#include <cstdio>  // vsnprintf
#include <string>  // std::string

namespace p {
namespace base {

// append formmatted ouput to a std::string
inline int StringAppendf(std::string *dst, const char *format, ...)
    __attribute__((format(printf, 2, 3)));

// assign formatted output to a std::string
inline int StringPrintf(std::string *dst, const char *format, ...)
    __attribute__((format(printf, 2, 3)));

// lower-level interface
// append formatted data from variable argument list to a string.
inline int StringAppendV(std::string *dst, const char *format, va_list argptr)
    __attribute__((format(printf, 2, 0)));

// lower-level interface
// write formatted data from variable argument list to a string.
int StringPrintV(std::string *dst, const char *format, va_list argptr)
    __attribute__((format(printf, 2, 0)));

// implements
inline int StringAppendV(std::string *dst, const char *format, va_list argptr) {
  const size_t len = dst->size();
  size_t capacity = dst->capacity();
  // must to resize the string to hold the buffer befor do sprintf
  dst->resize(capacity);
  capacity -= len;

  va_list copied_argptr;
  // copy arguments
  va_copy(copied_argptr, argptr);
  int ret = vsnprintf(&(*dst)[len], capacity + 1, format, copied_argptr);
  // free arguments
  va_end(copied_argptr);

  if (ret > 0) {
    dst->resize(len + ret);
    if ((size_t)ret > capacity) {
      // buffer overflow, do sprintf again
      vsnprintf(&(*dst)[len], ret + 1, format, argptr);
    }
  } else {
    // error when sprintf, resize to original length
    dst->resize(len);
  }

  return ret;
}

int StringPrintV(std::string *dst, const char *format, va_list argptr) {
  dst->clear();
  return StringAppendV(dst, format, argptr);
}

inline int StringAppendf(std::string *dst, const char *format, ...) {
  va_list argptr;

  va_start(argptr, format);
  const int ret = StringAppendV(dst, format, argptr);
  va_end(argptr);

  return ret;
}

inline int StringPrintf(std::string *dst, const char *format, ...) {
  va_list argptr;
  va_start(argptr, format);
  dst->clear();
  const int ret = StringAppendV(dst, format, argptr);
  va_end(argptr);

  return ret;
}

} // end namespace base
} // end namespace p
