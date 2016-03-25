// Copyright (c) 2015, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "p/base/macros.h"
#include "p/base/utils.h"
#include "p/base/fixed_buffer.h"

namespace p {
namespace base {

// a simple logging implement
// copy code from muduo, https://github.com/chenshuo/muduo

enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    NUM_LOG_LEVELS
};

class LogStream : public SmallFixedBuffer {
public:
    typedef LogStream   self;
    static const int kMaxNumericSize = 32;

    self& operator<<(bool v) {
        append(v ? '1' : '0');
        return *this;
    }

    self& operator<<(char v) {
        append(v);
        return *this;
    }

    self& operator<<(short v) {
        _AppendInteger(v);
        return *this;
    }

    self& operator<<(unsigned short v) {
        _AppendInteger(v);
        return *this;
    }

    self& operator<<(int v) {
        _AppendInteger(v);
        return *this;
    }

    self& operator<<(unsigned int v) {
        _AppendInteger(v);
        return *this;
    }

    self& operator<<(long v) {
        _AppendInteger(v);
        return *this;
    }

    self& operator<<(unsigned long v) {
        _AppendInteger(v);
        return *this;
    }

    self& operator<<(long long v) {
        _AppendInteger(v);
        return *this;
    }

    self& operator<<(unsigned long long v) {
        _AppendInteger(v);
        return *this;
    }

    self& operator<<(const void* v) {
        const int kMaxPointerSize = 2 * sizeof(const void*) + 2;
        if (avial() >= kMaxPointerSize) {
            add(Utils::ConvertPointer(cur(), v));
        }
        return *this;
    }

    self& operator<<(float v) {
        *this << static_cast<double>(v);
        return *this;
    }

    self& operator<<(long double v) {
        appendf("%LF", v);
        return *this;
    }

    self& operator<<(double v) {
        appendf("%F", v);
        return *this;
    }

    self& operator<<(const char* str) {
        if (str != nullptr) {
            append(str);
        } else {
            append("null");
        }
        return *this;
    }

private:
    template<typename T>
    void _AppendInteger(T v) {
        if (avial() >= kMaxNumericSize) {
            add(Utils::ConvertInteger(cur(), v));
        }
    }

private:
    LogLevel    log_level_;
    const char* file_name_;
    int         file_line_;
    bool        flush_ = true;
};

class Logger {
public:
    Logger(int i) {
        thread_local static LogStream tls_log_stream;
        const char* const LogLevelName[] =
            {"TRACE ", "DEBUG ", "INFO  ", "WARN  ", "ERROR ", "FATAL "};
        tls_log_stream.append(LogLevelName[i]);
        log_stream_ = &tls_log_stream;
    }

    LogStream& stream() {
        return *log_stream_;
    }

    ~Logger() {
        printf("%s\n", log_stream_->c_str());
    }
private:
    LogStream*  log_stream_;
};

} // end namespace base
} // end namespace p
