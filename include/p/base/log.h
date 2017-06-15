// Copyright (c) 2016, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once
#include "p/base/macros.h"

namespace p {
namespace base {

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

constexpr int SizeOfLogLevelName = 7;
constexpr const char *LogLevelName[] = {" TRACE ", " DEBUG ", " INFO  ",
                                        " WARN  ", " ERROR ", " FATAL "};

extern LogLevel g_log_level;

namespace LogDate {
const char* get_log_date_str();

#ifdef LOG_TIME_TO_US
constexpr int kLogDateStrLen = 24;
#else
constexpr int kLogDateStrLen = 21;
#endif
} // end of namespace LogDate

} // end namespace base
} // end namespace p
