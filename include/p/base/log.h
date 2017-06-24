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
constexpr const char *LogLevelName[] = {" TRACE ", " DEBUG ", "  INFO ",
                                        "  WARN ", " ERROR ", " FATAL "};
extern LogLevel g_log_level;

namespace LogDate {
const char* get_log_date_str();

#define LOG_TIME_TO_US

#ifdef LOG_TIME_TO_US
constexpr int kLogDateStrLen = 25;
#else
constexpr int kLogDateStrLen = 22;
#endif
} // end of namespace LogDate

#if 0
class SourceFile {
public:
    template<int N>
    constexpr  SourceFile(const char(&file_name)[N], int line) :
        file_name_(file_name), name_size_(N - 1), file_line_(line) {
        constexpr const char* slash = strrchr(file_name_, '/');
        file_name_ = slash ? (slash + 1) : file_name_;
        name_size_ = slash ? (name_size_ - (file_name - slash)) : name_size_;
    }

    constexpr int name_size() const { return name_size_; }

    constexpr const char* file_name() const {
        return file_name_;
    }

    constexpr int file_line() const { return file_line_; }

private:
    const char* file_name_;
    int     name_size_;
    int     file_line_;
};
#endif

class SourceFile {
public:
    template<int N>
    inline SourceFile(const char(&file_name)[N], int line) :
        file_name_(file_name), name_size_(N - 1), file_line_(line) {
        char* slash = __builtin_strrchr(file_name_, '/');
        if (slash) {
            file_name_ = slash + 1;
            name_size_ -= static_cast<int>(file_name_ - file_name);
            //name_size_ = __builtin_strlen(file_name_);
        }
    }

    int name_size() const { return name_size_; }

    const char* file_name() const {
        return file_name_;
    }

    int file_line() const { return file_line_; }

private:
    const char* file_name_;
    int     name_size_;
    int     file_line_;
};

class LogSink {
public:
    virtual ~LogSink() = 0;

    virtual int sink(const char* msg, int len) = 0;
};

bool stop_logging();

} // end namespace base
} // end namespace p
