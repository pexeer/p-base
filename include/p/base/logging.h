// Copyright (c) 2015, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "p/base/fixed_buffer.h"
#include "p/base/macros.h"
#include "p/base/queue.h"
#include "p/base/utils.h"

namespace p {
namespace base {

// a simple logging implement
// copy code from muduo, https://github.com/chenshuo/muduo

//{"TRACE", "DEBUG", "INFO ", "WARN ", "ERROR", "FATAL"};
enum class LogLevel {
  kTrace,
  kDebug,
  kInfo,
  kWarn,
  kError,
  kFatal,
  kLogLevelCount,
};

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
  printf("fck\n");
  logstream.noflush();
  return logstream;
}

class ThreadNumber {
public:
  static const char *c_str();

  static int size();

  static uint64_t tid();

private:
  P_DISALLOW_COPY(ThreadNumber);
};

class Logger {
public:
  Logger() {}

  LogStream &stream(LogLevel i, const char *file, int line);

  ~Logger();

private:
  P_DISALLOW_COPY(Logger);
};

class LogGenerator {
public:
  typedef LogGenerator self_type;
  struct LogEntry {
    struct LogEntry *next;
    uint32_t data_len = 4;
    char data[4];
  };
  static LinkedQueue<LogEntry> g_sinking_queue;
  static_assert(sizeof(LogEntry::next) == 8, "sizeof(void*) must be 8");
  static_assert(sizeof(LogEntry) == 16, "sizeof(LogEntry) must be power of 8");

  LogGenerator()
      : sentry_(nullptr), log_(nullptr), cur_(nullptr), end_(nullptr) {}

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
      cur_ += ConvertPointer(cur_, v);
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

private:
  // check buffer is enaugh for a log
  // check buffer is using by a log
  // return -1, buffer is using, write log is not finished;
  // return >=0, buffer is ready and empty
  int check_buffer();

  void Sink();

  int append(const char *buf, int len) {
    if
      UNLIKELY(len >= avial()) {
        if (avial() > 0) {
          len = avial();
        } else {
          return 0;
        }
      }
    ::memcpy(cur_, buf, len);
    cur_ += len;
    return len;
  }

  int append(char ch) {
    if (avial() > 0) {
      *cur_++ = ch;
      return 1;
    }
    return 0;
  }

  int append(const char *str) { return append(str, strlen(str)); }

  int appendf(const char *fmt, ...) {
    va_list argptr;
    va_start(argptr, fmt);
    const int ret = appendf(fmt, argptr);
    va_end(argptr);
    return ret;
  }

  int appendf(const char *fmt, va_list argptr) {
    int n = avial();
    if
      UNLIKELY(n <= 0) { return 0; }
    int ret = ::vsnprintf(cur_, n + 1, fmt, argptr);
    if
      UNLIKELY(ret < 0) { return 0; }
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

  friend class FastLogger;

private:
  LogEntry *sentry_;
  LogEntry *log_;
  char *cur_;
  char *end_;
  bool auto_flush_ = true;
  P_DISALLOW_COPY(LogGenerator);
};

class FastLogger {
public:
  FastLogger() {}

  LogGenerator &log_generator(LogLevel i, const char *file, int line);

  ~FastLogger();

private:
  P_DISALLOW_COPY(FastLogger);
};

} // end namespace base
} // end namespace p
