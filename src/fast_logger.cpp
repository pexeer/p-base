// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/base/fast_logger.h"

#include <stdlib.h>
#include <mutex>
#include <thread>

#include "p/base/queue.h"
#include "p/base/log.h"
#include "p/base/this_thread.h"

namespace p {
namespace base {

int g_wf_log_min_level = static_cast<int>(LogLevel::kWarn);

// struct LogEntry
struct FastLogGenerator::LogEntry {
    struct LogEntry *next;
    uint32_t data_len = 4;
    char data[4];
};

static_assert(sizeof(FastLogGenerator::LogEntry) == 16, "sizeof(LogEntry) must be power of 8");

LinkedQueue<FastLogGenerator::LogEntry> g_log_entry_queue;
/////////////////////////////////////////////////////////

// class LogBufferManager
class LogBufferManager {
public:
  struct BufferEntry {
    struct BufferEntry *next;
    int buffer_len;
    int retry_times;
  };

  static constexpr size_t kBufferLen = 1024 * 16;
  static constexpr int kMaxBufferNum = 1024 * 2;

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

// class FastLogGenerator
inline void FastLogGenerator::Sink() {
  if (auto_flush_) {
    *cur_++ = '\n';
    log_->data_len = cur_ - log_->data;
    // submit current log: log_
    if (UNLIKELY(log_level_ >= g_wf_log_min_level)) {
        // sync submit wf log
        ;
    }

    g_log_entry_queue.push_back(log_);
    cur_ = (char *)(P_PTR_CACHELINE_ALIGNMENT(cur_));

    // prepare for next log
    log_ = reinterpret_cast<LogEntry *>(cur_);
    cur_ = log_->data;
  }
  auto_flush_ = true;
}

inline int FastLogGenerator::check_buffer() {
  constexpr int kMinLogBufferSize = 1024;
  if (sentry_) {
    if (cur_ == log_->data) {
      if (cur_ + kMinLogBufferSize < end_) {
        return 0;
      }
      g_log_entry_queue.push_back(sentry_);
    } else {
      return -1;
    }
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

thread_local static FastLogGenerator tls_log_generator;
/////////////////////////////////////////////////////////

bool set_wf_log_min_level(LogLevel wf_log_min_level) {
    if (wf_log_min_level > LogLevel::kLogLevelCount) {
        return false;
    }

    g_wf_log_min_level = static_cast<int>(wf_log_min_level);
    return true;
}

FastLogGenerator &FastLogger::log_generator(LogLevel log_level, const char *file, int line) {
  tls_log_generator.log_level_ = static_cast<int>(log_level);
  int retry_times;
  if ((retry_times = tls_log_generator.check_buffer()) >= 0) {

    // add log head information
    tls_log_generator.just_append(LogDate::get_log_date_str(), LogDate::kLogDateStrLen);

    tls_log_generator.just_append(ThisThread::thread_name, ThisThread::thread_name_len);

    tls_log_generator.just_append(LogLevelName[tls_log_generator.log_level_], SizeOfLogLevelName);
    tls_log_generator.just_append(file, __builtin_strlen(file));
    tls_log_generator << ':' << line << ' ';

    //if (retry_times > 1) {
    //  tls_log_generator << "retry=" << retry_times << "] ";
    //}
  }
  return tls_log_generator;
}

FastLogger::~FastLogger() {
  tls_log_generator.Sink();
}

class FastLogSinkThread {
public:
  FastLogSinkThread() : log_sinker_(work) {
    s_fd_ = stdout;
  }

  ~FastLogSinkThread() {
    s_finished_ = true;
    log_sinker_.join();
  }

private:
  static bool s_finished_;
  static FILE *s_fd_;

  static void work() {
    FastLogGenerator::LogEntry *p;
    FastLogGenerator::LogEntry *cur;

    while (s_finished_ == false) {
      while ((p = g_log_entry_queue.pop_front())) {
        cur = p->next;
        fwrite(cur->data, 1, cur->data_len, s_fd_);
        if (p->data_len == 0) {
          LogBufferManager::BufferEntry *node =
              reinterpret_cast<LogBufferManager::BufferEntry *>(p);
          g_log_buffer_mgr.free_buffer(node);
        }
      }

      // try to sleep wait for more log entry
      g_log_buffer_mgr.wait_for();
    }

    while ((p = g_log_entry_queue.pop_front())) {
      cur = p->next;
      fwrite(cur->data, 1, cur->data_len, s_fd_);
    }

    fprintf(s_fd_, "Stop Logging Success.\n");
  }

  std::thread log_sinker_;
} g_log_sinker;

bool FastLogSinkThread::s_finished_ = false;
FILE *FastLogSinkThread::s_fd_ = nullptr;

inline void LogBufferManager::add_buffer(int num) {
    char *buffer = (char *)::malloc(kBufferLen * num);
    for (int i = 0; i < num; ++i) {
        BufferEntry *node =
            reinterpret_cast<BufferEntry *>(buffer + i * kBufferLen);
        // node.data_len = kBufferLen;
        free_buffer_queue_.push_back(node);
    }

    total_buffer_number_ += num;
    free_buffer_number_ += num;
}

inline LogBufferManager::BufferEntry* LogBufferManager::alloc_buffer() {
    //if (free_buffer_number_ < (kMaxBufferNum >> 3)) {
    //  notify_one();
    //}

    BufferEntry *p;
    int retry_times = 0;
    while ((p = free_buffer_queue_.pop_front()) == nullptr) {
      if (total_buffer_number_ < kMaxBufferNum) {
        add_buffer(10);
      } else {
        // signal log sink thread
        notify_one();
        // yield this thread
        ++retry_times;
        std::this_thread::yield();
      }
    }

    p->buffer_len = kBufferLen;
    p->retry_times = retry_times;
    --free_buffer_number_;
    return p;
}


} // end namespace base
} // end namespace p
