// Copyright (c) 2015, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "p/base/logger.h"

namespace p {
namespace base {

class Log {
public:
    typedef void (*OutputFunc)(const char* log_msg, int len);

    static bool set_log_level(LogLevel log_level);

    static bool set_ef_log_level(LogLevel ef_log_level);

    static bool set_max_length_per_log(uint32_t max_len);

    static void set_output_func(OutputFunc output_func);

    static void set_ef_output_func(OutputFunc output_func);

    static bool stop_logging();
};
extern LogLevel g_log_level;

} // end namespace base
} // end namespace p

#define LOG_TRACE                                                                                  \
    if (p::base::g_log_level <= p::base::LogLevel::kTrace)                                         \
    p::base::LogMessage().log_stream(p::base::LogLevel::kTrace,                                    \
                                     p::base::SourceFile(__FILE__, __LINE__))

#define LOG_DEBUG                                                                                  \
    if (p::base::g_log_level <= p::base::LogLevel::kDebug)                                         \
    p::base::LogMessage().log_stream(p::base::LogLevel::kDebug,                                    \
                                     p::base::SourceFile(__FILE__, __LINE__))

#define LOG_INFO                                                                                   \
    if (p::base::g_log_level <= p::base::LogLevel::kInfo)                                          \
    p::base::LogMessage().log_stream(p::base::LogLevel::kInfo,                                     \
                                     p::base::SourceFile(__FILE__, __LINE__))

#define LOG_WARN                                                                                   \
    p::base::LogMessage().log_stream(p::base::LogLevel::kWarn,                                     \
                                     p::base::SourceFile(__FILE__, __LINE__))

#define LOG_ERROR                                                                                  \
    p::base::LogMessage().log_stream(p::base::LogLevel::kError,                                    \
                                     p::base::SourceFile(__FILE__, __LINE__))

#define LOG_FATAL                                                                                  \
    p::base::LogMessage().log_stream(p::base::LogLevel::kFatal,                                    \
                                     p::base::SourceFile(__FILE__, __LINE__))
