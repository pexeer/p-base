// Copyright (c) 2016, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/base/logging.h"
#include "p/base/date.h"
#include <sys/syscall.h> // syscall
#include <thread>        // sth::thread
#include <unistd.h>      // SYS_gettid

namespace p {
namespace base {

thread_local static LogStream tls_log_stream;

class ThreadNumberImpl {
public:
  ThreadNumberImpl() {
    // ::gettid()
    tid_ = static_cast<pid_t>(::syscall(SYS_gettid));
    size_ = ConvertInteger(name_, tid_);
  }

  friend class ThreadNumber;

private:
  int tid_;
  int size_;
  char name_[24];
};

// thread cached ThreadNumber
thread_local static ThreadNumberImpl tls_thread_number;

inline const char *ThreadNumber::c_str() { return tls_thread_number.name_; }

inline int ThreadNumber::size() { return tls_thread_number.size_; }

inline uint64_t ThreadNumber::tid() { return tls_thread_number.tid_; }

constexpr const char *LogLevelName[] = {" TRACE ", " DEBUG ", " INFO  ",
                                        " WARN  ", " ERROR ", " FATAL "};

// Loggger
LogStream &Logger::stream(LogLevel i, const char *file, int line) {
  if (tls_log_stream.empty()) {
    tls_log_stream.Reset(LogLevelName[static_cast<int>(i)], 6);

    // append time
    Date::update();
    tls_log_stream.append(Date::c_str(), Date::size());

    // append thread_number
    tls_log_stream.append(ThreadNumber::c_str(), ThreadNumber::size());

    // append file:line

    tls_log_stream << ' ';
    tls_log_stream.append(file);
    tls_log_stream << ':' << line << '-';
  }

  return tls_log_stream;
}

inline void LogGenerator::Sink() {
  if (auto_flush_) {
    auto_flush_ = true;
    *cur_++ = '\n';
    log_->data_len = cur_ - log_->data;
    // submit current log: log_
    g_sinking_queue.push_back(log_);
    cur_ = (char *)(PTR_CACHELINE_ALIGNMENT(cur_));

    // prepare for next log
    log_ = reinterpret_cast<LogEntry *>(cur_);
    cur_ = log_->data;
    return;
  }
  auto_flush_ = true;
}

Logger::~Logger() { tls_log_stream.Sink(); }

// class LogBufferManager
class LogBufferManager {
public:
  struct BufferEntry {
    struct BufferEntry *next;
    int buffer_len;
    int retry_times;
  };
  static constexpr size_t kBufferLen = 128 * 1024L;
  static constexpr int kMaxBufferNum = 1024;
  LogBufferManager() : total_buffer_number_(0) {
    add_buffer(100);
    alloc_buffer(); // pop dummy Node
  }

  BufferEntry *alloc_buffer() {
    BufferEntry *p;
    int retry_times = 0;
    while ((p = free_buffer_queue_.pop_front()) == nullptr) {
      if (total_buffer_number_ < kMaxBufferNum) {
        add_buffer(10);
      } else {
        // yield this thread
        ++retry_times;
        std::this_thread::yield();
      }
    }
    p->buffer_len = kBufferLen;
    p->retry_times = retry_times;
    return p;
  }

  void free_buffer(BufferEntry *p) { free_buffer_queue_.push_back(p); }

  void add_buffer(int num) {
    char *buffer = (char *)malloc(kBufferLen * num);
    for (int i = 0; i < num; ++i) {
      BufferEntry *node =
          reinterpret_cast<BufferEntry *>(buffer + i * kBufferLen);
      // node.data_len = kBufferLen;
      free_buffer_queue_.push_back(node);
    }
    total_buffer_number_ += num;
  }

private:
  std::atomic<int> total_buffer_number_;
  LinkedQueue<BufferEntry> free_buffer_queue_;
};

LinkedQueue<LogGenerator::LogEntry> LogGenerator::g_sinking_queue;
LogBufferManager g_log_buffer_mgr;

// char *buffer = static_cast<char *>(malloc(kBufferLen));
int LogGenerator::check_buffer() {
  constexpr int kMinLogBufferSize = 1024;
  if (sentry_) {
    if (cur_ == log_->data) {
      if (cur_ + kMinLogBufferSize < end_) {
        return 0;
      }
      g_sinking_queue.push_back(sentry_);
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

thread_local static LogGenerator tls_log_generator;

LogGenerator &FastLogger::log_generator(LogLevel i, const char *file,
                                        int line) {
  int retry_times;
  if ((retry_times = tls_log_generator.check_buffer()) >= 0) {
    // add log head information
    Date::update();
    tls_log_generator.append(Date::c_str(), Date::size());
    tls_log_generator.append(ThreadNumber::c_str(), ThreadNumber::size());

    tls_log_generator.append(LogLevelName[static_cast<int>(i)], 7);
    tls_log_generator.append("main:", 5);
    tls_log_generator.append(file, __builtin_strlen(file));
    tls_log_generator << ":" << line << "] ";
    if (retry_times > 1) {
      tls_log_generator << "retry=" << retry_times << "] ";
    }
  }
  return tls_log_generator;
}

FastLogger::~FastLogger() {
  // printf("sink\n");
  tls_log_generator.Sink();
}

class LogSinkThread {
public:
  LogSinkThread() : log_sinker_(work) {
    // g_fd = fopen("x.log", "wa");
    g_fd = stdout;
  }

  ~LogSinkThread() {
    finished_ = true;
    log_sinker_.join();
  }

private:
  static bool finished_;
  static FILE *g_fd;
  static void work() {
    LinkedQueue<LogGenerator::LogEntry> &g_sinking_queue =
        LogGenerator::g_sinking_queue;
    LogGenerator::LogEntry *p;
    LogGenerator::LogEntry *cur;
    while (finished_ == false) {
      while ((p = g_sinking_queue.pop_front())) {
        cur = p->next;
        // printf("%s\tlen=%d\n", cur->data, cur->data_len);
        fwrite(cur->data, 1, cur->data_len, g_fd);
        if (p->data_len == 0) {
          LogBufferManager::BufferEntry *node =
              reinterpret_cast<LogBufferManager::BufferEntry *>(p);
          g_log_buffer_mgr.free_buffer(node);
        }
      }
      // sleep(1);
      std::this_thread::yield();
    }
    while ((p = g_sinking_queue.pop_front())) {
      cur = p->next;
      // printf("%s\tlen=%d\n", cur->data, cur->data_len);
      fwrite(cur->data, 1, cur->data_len, g_fd);
    }
    fprintf(g_fd, "Logging Stop Success.\n");
  }

  std::thread log_sinker_;
} g_log_sinker;

bool LogSinkThread::finished_ = false;
FILE *LogSinkThread::g_fd = 0;

} // end namespace base
} // end namespace p
