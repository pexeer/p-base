// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/base/logger.h"

#include <stdlib.h>
#include <unistd.h>
#include <mutex>
#include <thread>
#include <condition_variable>

#include "p/base/queue.h"
#include "p/base/log.h"
#include "p/base/fixed_buffer.h"
#include "p/base/this_thread.h"

namespace p {
namespace base {
LogLevel    g_wf_log_min_level = LogLevel::kWarn;

void default_output_func(const char* msg, int len) {
    auto unused = write(1, msg, len);
    (void)unused;
}

void default_wf_output_func(const char* msg, int len) {
    auto unused = write(2, msg, len);
    (void)unused;
}

LogMessage::OutputFunc g_output_func = default_output_func;
LogMessage::OutputFunc g_wf_output_func = default_wf_output_func;

// struct LogEntry
struct LogStream::LogEntry {
    struct LogEntry*    next;
    uint32_t    data_len = 4;
    char data[4];
};

static_assert(sizeof(LogStream::LogEntry) == 16, "sizeof(LogEntry) must be power of 8");

LinkedQueue<LogStream::LogEntry> g_log_entry_queue;
/////////////////////////////////////////////////////////

// class LogBufferManager
class LogBufferManager {
public:
  struct BufferEntry {
    struct BufferEntry* next;
    int buffer_len;
    int retry_times;
  };

  static constexpr size_t kLogBufferSize = 1024 * 16;
  static constexpr int kLogBufferMaxNum = 1024 * 2;

  LogBufferManager() : total_buffer_number_(0), free_buffer_number_(0) {
    add_buffer(100);
    alloc_buffer(); // pop dummy Node
  }

  BufferEntry *alloc_buffer();

  void free_buffer(BufferEntry *p) {
    free_buffer_queue_.push_back(p);
    ++free_buffer_number_;
  }

  void notify_one() {
    std::lock_guard<std::mutex> lock_guard(buffer_mutex_);
    buffer_condition_.notify_one();
  }

  void wait_for() {
    std::unique_lock<std::mutex> unique_lock(buffer_mutex_);
    buffer_condition_.wait_for(unique_lock, std::chrono::milliseconds(300));
  }

private:
  void add_buffer(int num);

private:
  std::atomic<int> total_buffer_number_;
  std::atomic<int> free_buffer_number_;
  LinkedQueue<BufferEntry> free_buffer_queue_;

  std::mutex buffer_mutex_;
  std::condition_variable buffer_condition_;
};

LogBufferManager g_log_buffer_mgr;
/////////////////////////////////////////////////////////

// class LogStream
inline void LogStream::Sink() {
  if (auto_flush_) {
      if (avial() >= source_file_.name_size() + 32) {
          just_append(" - ", 3);
          just_append(source_file_.file_name(), source_file_.name_size());
          *cur_++ = ':';
          cur_ += ConvertInteger(cur_, source_file_.file_line());
      }
    *cur_++ = '\n';
    log_->data_len = cur_ - log_->data;
    // submit current log: log_
    if (UNLIKELY(log_level_ >= g_wf_log_min_level)) {
        g_wf_output_func(log_->data, log_->data_len);
    }

    g_log_entry_queue.push_back(log_);
    cur_ = (char *)(P_PTR_CACHELINE_ALIGNMENT(cur_));

    // prepare for next log
    log_ = reinterpret_cast<LogEntry *>(cur_);
    cur_ = log_->data;
  }
  auto_flush_ = true;
}

inline int LogStream::check_log_buffer() {
  constexpr int kMinLogBufferSize = 1024;
  if (sentry_) {
      if (UNLIKELY(cur_ != log_->data)) {
          return -1;
      }
      if (cur_ + kMinLogBufferSize < end_) {
          return 0;
      }
      g_log_entry_queue.push_back(sentry_);
  }

  LogBufferManager::BufferEntry *p = g_log_buffer_mgr.alloc_buffer();
  char *buffer = reinterpret_cast<char *>(p);
  end_ = buffer + p->buffer_len - 1;
  int retry_times = p->retry_times;
  sentry_ = reinterpret_cast<LogEntry *>(p);
  sentry_->next = nullptr;
  sentry_->data_len = 0;

  log_ = sentry_ + 1;
  cur_ = log_->data;
  return retry_times;
}

thread_local LogStream tls_log_stream;
/////////////////////////////////////////////////////////

bool LogMessage::set_wf_log_min_level(LogLevel wf_log_min_level) {
    if (wf_log_min_level > LogLevel::kLogLevelCount) {
        return false;
    }

    g_wf_log_min_level = wf_log_min_level;
    return true;
}

void LogMessage::set_output_func(OutputFunc output_func) {
    g_output_func = output_func;
}

void LogMessage::set_wf_output_func(OutputFunc output_func) {
    g_wf_output_func = output_func;
}

LogStream &LogMessage::log_stream(LogLevel log_level, const SourceFile &source_file) {
  tls_log_stream.log_level_ = log_level;
  int retry_times;
  if ((retry_times = tls_log_stream.check_log_buffer()) >= 0) {

    // add log head information
    tls_log_stream.just_append(LogDate::get_log_date_str(), LogDate::kLogDateStrLen);

    tls_log_stream.just_append(ThisThread::thread_name(), ThisThread::thread_name_len());

    tls_log_stream.just_append(
            LogLevelName[int(tls_log_stream.log_level_)], SizeOfLogLevelName);

    tls_log_stream.source_file_ = source_file;

#if 0
    if (retry_times > 1) {
        tls_log_stream << "retry=" << retry_times << ' ';
    }
#endif
  }
  return tls_log_stream;
}

LogMessage::~LogMessage() {
  tls_log_stream.Sink();
}

class LogSinkThread {
public:
  LogSinkThread() : log_sinker_(&LogSinkThread::work, this) {
      current_pos_ = flush_buffer_;
  }

  bool stop() {
      finished_ = true;
      if (log_sinker_.joinable()) {
          log_sinker_.join();
          return true;
      }

      return false;
  }


  ~LogSinkThread() {
      stop();
  }

private:
  void append(const char* message, int len) {
      if (current_pos_ < flush_buffer_end_) {
          ::memcpy(current_pos_, message, len);
          current_pos_ += len;
          return ;
      }

      flush();

      ::memcpy(current_pos_, message, len);
      current_pos_ += len;
      return ;
  }

  void flush() {
      if (current_pos_ > flush_buffer_) {
          g_output_func(flush_buffer_, current_pos_ - flush_buffer_);
          current_pos_ = flush_buffer_;
      }
  }


  void work() {
    LogStream::LogEntry *p;
    LogStream::LogEntry *cur;

    while (!finished_) {
      while ((p = g_log_entry_queue.pop_front())) {
        cur = p->next;
        append(cur->data, cur->data_len);
        if (UNLIKELY(p->data_len == 0)) {
          LogBufferManager::BufferEntry *node =
              reinterpret_cast<LogBufferManager::BufferEntry *>(p);
          g_log_buffer_mgr.free_buffer(node);
        }
      }

      flush();

      // try to sleep wait for more log entry
      g_log_buffer_mgr.wait_for();
    }

    while ((p = g_log_entry_queue.pop_front())) {
      cur = p->next;
      append(cur->data, cur->data_len);
    }

    const char stop_message[] = "Stop Logging Success.\n";
    append(stop_message, strlen(stop_message));

    // last flush before stop
    flush();
  }

private:
  constexpr static int kFlushBufferSize = 1024 * 1024 * 4;
  char      flush_buffer_[kFlushBufferSize];
  char      flush_buffer_end_[LogBufferManager::kLogBufferSize];
  char*     current_pos_;
  bool      finished_;
  std::thread log_sinker_;
} g_log_sinker;

inline void LogBufferManager::add_buffer(int num) {
    char *buffer = (char *)::malloc(kLogBufferSize * num);
    for (int i = 0; i < num; ++i) {
        BufferEntry *node =
            reinterpret_cast<BufferEntry *>(buffer + i * kLogBufferSize);
        // node.data_len = kLogBufferSize;
        free_buffer_queue_.push_back(node);
    }

    total_buffer_number_ += num;
    free_buffer_number_ += num;
}

inline LogBufferManager::BufferEntry* LogBufferManager::alloc_buffer() {
    //if (free_buffer_number_ < (kLogBufferMaxNum >> 3)) {
    //  notify_one();
    //}

    BufferEntry *p;
    int retry_times = 0;
    while ((p = free_buffer_queue_.pop_front()) == nullptr) {
      if (total_buffer_number_ < kLogBufferMaxNum) {
        add_buffer(10);
      } else {
        // signal log sink thread
        notify_one();
        // yield this thread
        ++retry_times;
        std::this_thread::yield();
      }
    }

    p->buffer_len = kLogBufferSize;
    p->retry_times = retry_times;
    --free_buffer_number_;
    return p;
}

bool stop_logging() {
    return p::base::g_log_sinker.stop();
}

#if 0
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
#endif

} // end namespace base
} // end namespace p

