// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/base/logger.h"

#include <condition_variable>
#include <mutex>
#include <stdlib.h>
#include <thread>
#include <unistd.h>
#include <deque>
#include <assert.h>

#include "p/base/queue.h"
#include "p/base/this_thread.h"
#include "p/base/logging.h"

namespace p {
namespace base {

////////////////////////////////////////
// for p/base/logging.h class Log

LogLevel g_log_level = LogLevel::kTrace;
LogLevel g_ef_log_level = LogLevel::kError;

size_t   g_max_length_per_log = 2 * 1024;
constexpr static int kFlushBufferSize = 32 * 1024;

void default_output_func(const char *msg, int len) {
    assert(len > 0);
    //fprintf(stderr, "backend flush bytes=%d\n", len);
    auto unused = write(1, msg, len);
    (void)unused;
}

void default_ef_output_func(const char *msg, int len) {
    auto unused = write(2, msg, len);
    (void)unused;
}

Log::OutputFunc g_output_func = default_output_func;
Log::OutputFunc g_ef_output_func = default_ef_output_func;


bool Log::set_log_level(LogLevel log_min_level) {
    if (log_min_level > LogLevel::kLogLevelCount) {
        return false;
    }

    g_log_level = log_min_level;
    return true;
}

bool Log::set_ef_log_level(LogLevel ef_log_min_level) {
    if (ef_log_min_level > LogLevel::kLogLevelCount) {
        return false;
    }

    g_ef_log_level = ef_log_min_level;
    return true;
}

bool Log::set_max_length_per_log(uint32_t max_len) {
    if (max_len <  1024 || max_len > 16 * 1024 * 1024) {
        return false;
    }
    g_max_length_per_log = max_len;
    return true;
}


void Log::set_output_func(Log::OutputFunc output_func) { g_output_func = output_func; }

void Log::set_ef_output_func(Log::OutputFunc output_func) { g_ef_output_func = output_func; }



////////////////////////////////////////////

class LogSinkThread {
public:
    LogSinkThread() : sinker_thread_(&LogSinkThread::work, this) {}

    bool stop() {
        finished_ = true;
        backend_condition_.notify_one();

        if (sinker_thread_.joinable()) {
            sinker_thread_.join();
            return true;
        }

        return false;
    }

    ~LogSinkThread() { stop(); }

    void append_buf(const char* p, uint64_t length) {
        std::unique_lock<std::mutex> lock_gaurd(mutex_front_);

        if (!cur_buffer_) {
            std::unique_lock<std::mutex> lock_gaurd(mutex_backend_);
            new_buffer();
        }

        if (cur_ptr_ + length > end_) {
            std::unique_lock<std::mutex> lock_gaurd(mutex_backend_);
            new_buffer();
        }

        ::memcpy(cur_ptr_, p, length);
        cur_ptr_ += length;
    }

private:
    void wait_for(int ms) {
        std::unique_lock<std::mutex> lock_guard(mutex_backend_);
        backend_condition_.wait_for(lock_guard, std::chrono::milliseconds(ms));
    }

    void work() {
        while (!finished_) {
            wait_for(200);  // wait interverl
            sync_with_front(); // get front data
            flush(); // flush all list
        }

        // last flush before stop
        LOG_FATAL << "Stop Logging Success.";
        {
            std::unique_lock<std::mutex> lock_gaurd(mutex_backend_);
            new_buffer();
        }

        // last flush before stop
        flush();
    }

    void flush() {
        while (true) {
            BufferRef ref = {nullptr, -1};
            {
                std::unique_lock<std::mutex> lock_gaurd(mutex_backend_);
                while (!ref_list_.empty()) {
                    ref = ref_list_.front();
                    ref_list_.pop_front();

                    if (ref.length <= 0) {
                        free_buffer_list_.push_back((char*)ref.ptr);
                        continue;
                    }

                    break;
                }
            }

            if (ref.length <= 0) {
                return ;
            }

            // do io
            g_output_func(ref.ptr, ref.length);
        }
    }

    void sync_with_front() {
        std::unique_lock<std::mutex> lock_gaurd(mutex_front_);
        if (cur_ptr_ > ptr_) {
            ref_list_.push_back(BufferRef{ptr_, cur_ptr_ - ptr_});
            cur_ptr_ = ptr_;
        }
    }

    void new_buffer() {
        if (cur_ptr_ > ptr_) {
            ref_list_.push_back(BufferRef{ptr_, cur_ptr_ - ptr_});
            cur_ptr_ = ptr_;
        }
        if (cur_buffer_) {
            ref_list_.push_back(BufferRef{cur_buffer_, 0});
            backend_condition_.notify_one();
        }

        if (free_buffer_list_.empty()) {
            cur_buffer_ = (char*)::malloc(kFlushBufferSize);
            //fprintf(stderr, "backend malloc buffer bytes=%d\n", kFlushBufferSize);
        } else {
            cur_buffer_ = free_buffer_list_.front();
            free_buffer_list_.pop_front();
        }

        ptr_ = cur_buffer_;
        end_ = cur_buffer_ + kFlushBufferSize;
        cur_ptr_ = ptr_;
    }

private:
    struct BufferRef {
        const char*   ptr;
        int64_t       length;
    };

    std::mutex      mutex_backend_;
    std::condition_variable backend_condition_;
    std::deque<char*>  free_buffer_list_;
    std::deque<BufferRef>   ref_list_;

    std::mutex      mutex_front_;
    char *cur_buffer_ = nullptr;
    char *ptr_ = nullptr;
    char *cur_ptr_ = nullptr;
    char *end_ = nullptr;

    bool finished_ = false;

    std::thread sinker_thread_;
} g_log_sinker;


// class LogStream
void LogStream::Sink() {
    if (auto_flush_) {
        if (avial() >= source_file_.name_size() + 32) {
            just_append(" - ", 3);
            just_append(source_file_.file_name(), source_file_.name_size());
            *cur_++ = ':';
            cur_ += ConvertInteger(cur_, source_file_.file_line());
        }
        *cur_++ = '\n';

        // submit current log if error and fatal log
        if (UNLIKELY(log_level_ >= g_ef_log_level)) {
            g_ef_output_func(buffer_, cur_ - buffer_);
        }

        g_log_sinker.append_buf(buffer_, cur_ - buffer_);
        cur_ = buffer_;

    }
    auto_flush_ = true;
}

bool LogStream::check_buffer() {
    if (buffer_ == nullptr) {
        buffer_ = (char*)::malloc(g_max_length_per_log);
        cur_ = buffer_;
        end_ = buffer_ + g_max_length_per_log;
    }

    return cur_ == buffer_;
}

thread_local LogStream tls_log_stream;
/////////////////////////////////////////////////////////

LogStream &LogMessage::log_stream(LogLevel log_level, const SourceFile &source_file) {
    tls_log_stream.log_level_ = log_level;

    if (tls_log_stream.check_buffer()) {
        // add log head information
        tls_log_stream.just_append(LogDate::get_log_date_str(), LogDate::kLogDateStrLen);

        tls_log_stream.just_append(ThisThread::thread_name(), ThisThread::thread_name_len());

        tls_log_stream.just_append(LogLevelName[int(tls_log_stream.log_level_)],
                SizeOfLogLevelName);

        tls_log_stream.source_file_ = source_file;
    }

    return tls_log_stream;
}

LogMessage::~LogMessage() { tls_log_stream.Sink(); }

} // end namespace base
} // end namespace p

