// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <stdarg.h>
#include <string.h>

#include "p/base/utils.h"
#include "p/base/log.h"
//#include "p/base/fixed_buffer.h"

namespace p {
namespace base {

class LogStream {
public:
  struct LogEntry;

  LogStream()
      : sentry_(nullptr), log_(nullptr), cur_(nullptr),
      end_(nullptr), source_file_(__FILE__, __LINE__) {}

  LogStream &operator<<(short v) {
    _AppendInteger(v);
    return *this;
  }

  LogStream &operator<<(unsigned short v) {
    _AppendInteger(v);
    return *this;
  }

  LogStream &operator<<(int v) {
    _AppendInteger(v);
    return *this;
  }

  LogStream &operator<<(unsigned int v) {
    _AppendInteger(v);
    return *this;
  }

  LogStream &operator<<(long v) {
    _AppendInteger(v);
    return *this;
  }

  LogStream &operator<<(unsigned long v) {
    _AppendInteger(v);
    return *this;
  }

  LogStream &operator<<(long long v) {
    _AppendInteger(v);
    return *this;
  }

  LogStream &operator<<(unsigned long long v) {
    _AppendInteger(v);
    return *this;
  }

  LogStream &operator<<(float v) {
    *this << static_cast<double>(v);
    return *this;
  }

  LogStream &operator<<(long double v) {
    appendf("%LF", v);
    return *this;
  }

  LogStream &operator<<(double v) {
    appendf("%F", v);
    return *this;
  }

  LogStream &operator<<(const char *str) {
    if (str) {
      append(str);
    } else {
      append("null");
    }
    return *this;
  }

  LogStream &operator<<(const void *v) {
    constexpr int kMaxPointerSize = 2 * sizeof(const void *) + 2;
    if (avial() >= kMaxPointerSize) {
      cur_ += ConvertPointer(cur_, v);
    }
    return *this;
  }

  LogStream &operator<<(bool v) {
    append(v ? '1' : '0');
    return *this;
  }

  LogStream &operator<<(char v) {
    append(v);
    return *this;
  }

  LogStream& operator<<(LogStream &(*func)(LogStream &)) {
    return (*func)(*this);
  }

  LogStream& noflush() {
      auto_flush_ = false;
      return *this;
  }

private:
  // check buffer is enaugh for a log and check buffer is using by a log
  // return -1, buffer is using, write log is not finished;
  // return >=0, buffer is ready and empty
  int check_log_buffer();

  void Sink();

  int just_append(const char* buf, int len) {
      ::memcpy(cur_, buf, len);
      cur_ += len;
      return len;
  }


  int append(const char *buf, int len) {
      if (UNLIKELY(len > avial())) {
          len = avial();
      }

      if (UNLIKELY(len <= 0)) {
          return 0;
      }
      return just_append(buf, len);
  }

  int append(char ch) {
    if (avial() > 0) {
      *cur_++ = ch;
      return 1;
    }
    return 0;
  }

  int append(const char *str) { return append(str, ::strlen(str)); }

  int appendf(const char *fmt, ...) {
    va_list argptr;
    va_start(argptr, fmt);
    const int ret = appendf(fmt, argptr);
    va_end(argptr);
    return ret;
  }

  int appendf(const char *fmt, va_list argptr) {
    int n = avial();
    if (UNLIKELY(n <= 0)) { return 0; }
    int ret = ::vsnprintf(cur_, n + 1, fmt, argptr);
    if (UNLIKELY(ret < 0)) { return 0; }
    if (ret > n) {
      ret = n;
    }
    cur_ += ret;
    return ret;
  }

  const int avial() const { return static_cast<int>(end_ - cur_); }

  template <typename T> void _AppendInteger(T v) {
    if (avial() >= kMaxNumericSize) {
      cur_ += ConvertInteger(cur_, v);
    }
  }

  friend class LogMessage;

private:
  LogEntry *sentry_;
  LogEntry *log_;
  char *cur_;
  char *end_;
  bool auto_flush_  = true;
  LogLevel  log_level_;
  SourceFile    source_file_;
  P_DISALLOW_COPY(LogStream);
};

typedef LogStream LogStream;

inline LogStream& noflush(LogStream& ls) { return ls.noflush(); }

class LogMessage {
public:
    static bool set_wf_log_min_level(LogLevel wf_log_min_level);

    typedef void (*OutputFunc)(const char* log_msg, int len);

    static void set_output_func(OutputFunc output_func);

    static void set_wf_output_func(OutputFunc output_func);

public:
  LogMessage() {}

  //LogStream &log_stream(LogLevel log_level, const char *file, int line);

  LogStream &log_stream(LogLevel log_level, const SourceFile &source_file);

  ~LogMessage();

private:
  P_DISALLOW_COPY(LogMessage);
};

#if 0
class LogStream : public SmallFixedBuffer {
public:
  typedef LogStream self_type;

  LogStream() {}

  self_type &operator<<(short v) {
    _AppendInteger(v);
    return *this;
  }

  self_type &operator<<(unsigned short v) {
    _AppendInteger(v);
    return *this;
  }

  self_type &operator<<(int v) {
    _AppendInteger(v);
    return *this;
  }

  self_type &operator<<(unsigned int v) {
    _AppendInteger(v);
    return *this;
  }

  self_type &operator<<(long v) {
    _AppendInteger(v);
    return *this;
  }

  self_type &operator<<(unsigned long v) {
    _AppendInteger(v);
    return *this;
  }

  self_type &operator<<(long long v) {
    _AppendInteger(v);
    return *this;
  }

  self_type &operator<<(unsigned long long v) {
    _AppendInteger(v);
    return *this;
  }

  self_type &operator<<(float v) {
    *this << static_cast<double>(v);
    return *this;
  }

  self_type &operator<<(long double v) {
    appendf("%LF", v);
    return *this;
  }

  self_type &operator<<(double v) {
    appendf("%F", v);
    return *this;
  }

  self_type &operator<<(const char *str) {
    if (str != nullptr) {
      append(str);
    } else {
      append("null");
    }
    return *this;
  }

  self_type &operator<<(const void *v) {
    constexpr int kMaxPointerSize = 2 * sizeof(const void *) + 2;
    if (avial() >= kMaxPointerSize) {
      add(ConvertPointer(cur(), v));
    }
    return *this;
  }

  self_type &operator<<(bool v) {
    append(v ? '1' : '0');
    return *this;
  }

  self_type &operator<<(char v) {
    append(v);
    return *this;
  }

  self_type &operator<<(self_type &(*func)(self_type &)) {
    return (*func)(*this);
  }

  void noflush() { auto_flush_ = false; }

  void Reset(const char *buf, int len) {
    ::memcpy(data_, buf, len);
    cur_ = data_ + len;
  }

  void Sink() {
    if (auto_flush_) {
      printf("%s\n", data_);
      reset();
    }
    auto_flush_ = true;
  }

private:
  template <typename T> void _AppendInteger(T v) {
    if (avial() >= kMaxNumericSize) {
      add(ConvertInteger(cur(), v));
    }
  }

private:
  bool auto_flush_ = true;
  P_DISALLOW_COPY(LogStream);
};

inline LogStream &noflush(LogStream &logstream) {
  logstream.noflush();
  return logstream;
}

class Logger {
public:
  Logger() {}

  LogStream &stream(const char* log_level_name, const char *file, int line);

  ~Logger();

private:
  P_DISALLOW_COPY(Logger);
};
#endif

} // end namespace base
} // end namespace p

