// Copyright (c) 2016, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/base/logging.h"
#include <sys/syscall.h> // syscall
#include <unistd.h>      // SYS_gettid

namespace p {
namespace base {

thread_local static LogStream tls_log_stream;

class ThreadNumberImpl : public ThreadNumber {
public:
  ThreadNumberImpl() {
    // ::gettid()
    tid_ = static_cast<pid_t>(::syscall(SYS_gettid));
    ConvertInteger(name_, tid_);
  }

public:
  uint64_t tid_;
  char name_[24];
};

// thread cached ThreadNumber
thread_local static ThreadNumberImpl tls_thread_number;

const char *ThreadNumber::c_str() { return tls_thread_number.name_; }

uint64_t ThreadNumber::tid() { return tls_thread_number.tid_; }

LogStream &Logger::stream(LogLevel i, const char *file, int line) {
  constexpr const char *LogLevelName[] = {"TRACE ", "DEBUG ", "INFO  ",
                                          "WARN  ", "ERROR ", "FATAL "};
  if (tls_log_stream.empty()) {
    tls_log_stream.Reset(LogLevelName[static_cast<int>(i)], 6);

    // append time
    tls_log_stream.append("0918 12:23:34.439874 ");

    // append thread_number
    tls_log_stream.append(ThreadNumber::c_str());

    // append file:line

    tls_log_stream << ' ';
    tls_log_stream.append(file);
    tls_log_stream << ':' << line << '-';
  }

  return tls_log_stream;
}

Logger::~Logger() { tls_log_stream.Sink(); }

} // end namespace base
} // end namespace p
