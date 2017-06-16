// Copyright (c) 2015, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "p/base/fast_logger.h"

#define LOG_TRACE if (p::base::g_log_level <= p::base::LogLevel::kTrace) \
    p::base::FastLogMessage().log_stream(p::base::LogLevel::kTrace,\
    p::base::SourceFile(__FILE__, __LINE__))

#define LOG_DEBUG if (p::base::g_log_level <= p::base::LogLevel::kDebug) \
    p::base::FastLogMessage().log_stream(p::base::LogLevel::kDebug,\
    p::base::SourceFile(__FILE__, __LINE__))

#define LOG_INFO if (p::base::g_log_level <= p::base::LogLevel::kInfo) \
    p::base::FastLogMessage().log_stream(p::base::LogLevel::kInfo,\
    p::base::SourceFile(__FILE__, __LINE__))

#define LOG_WARN p::base::FastLogMessage().log_stream(\
        p::base::LogLevel::kWarn, p::base::SourceFile(__FILE__, __LINE__))

#define LOG_ERROR p::base::FastLogMessage().log_stream(\
        p::base::LogLevel::kError, p::base::SourceFile(__FILE__, __LINE__))

#define LOG_FATAL p::base::FastLogMessage().log_stream(\
        p::base::LogLevel::kFatal, p::base::SourceFile(__FILE__, __LINE__))
