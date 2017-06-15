// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/base/log.h"
#include "p/base/logger.h"

namespace p {
namespace base {

thread_local static LogStream tls_log_stream;

// Loggger
Logger::~Logger() { tls_log_stream.Sink(); }

LogStream &Logger::stream(const char* log_level_name, const char *file, int line) {
  if (tls_log_stream.empty()) {
    tls_log_stream.Reset(log_level_name, 6);

    // append time
    //Date::update();
    //tls_log_stream.append(Date::c_str(), Date::size());

    // append thread_number
    //tls_log_stream.append(ThreadNumber::c_str(), ThreadNumber::size());

    // append file:line

    tls_log_stream << ' ';
    tls_log_stream.append(file);
    tls_log_stream << ':' << line << '-';
  }

  return tls_log_stream;
}

} // end namespace base
} // end namespace p
