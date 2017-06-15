// Copyright (c) 2015, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "p/base/fast_logger.h"

#define P_LOG_FILENAME (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#define LOG_TRACE if (p::base::g_log_level <= p::base::LogLevel::kTrace) \
    p::base::FastLogger().log_generator(p::base::LogLevel::kTrace, P_LOG_FILENAME, __LINE__)

#define LOG_DEBUG if (p::base::g_log_level <= p::base::LogLevel::kDebug) \
    p::base::FastLogger().log_generator(p::base::LogLevel::kDebug, P_LOG_FILENAME, __LINE__)

#define LOG_INFO if (p::base::g_log_level <= p::base::LogLevel::kInfo) \
    p::base::FastLogger().log_generator(p::base::LogLevel::kInfo, P_LOG_FILENAME, __LINE__)

#define LOG_WARN p::base::FastLogger().log_generator(\
        p::base::LogLevel::kWarn, P_LOG_FILENAME, __LINE__)

#define LOG_ERROR p::base::FastLogger().log_generator(\
        p::base::LogLevel::kError, P_LOG_FILENAME, __LINE__)

#define LOG_FATAL p::base::FastLogger().log_generator(\
        p::base::LogLevel::kFatal, P_LOG_FILENAME, __LINE__)
