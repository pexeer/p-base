
#include "p/base/fixed_buffer.h"

// a simple logging implement
// copy code from muduo, https://github.com/chenshuo/muduo

class LogStream : public SmallFixedBuffer {
public:
  typedef LogStream self_type;

  LogStream() {}

  self_type &operator<<(short v) {
    _AppendInteger(v);
    return *this;
  }

  self_type &operator<<(unsigned short v) {
    _AppendInteger(v);
    return *this;
  }

  self_type &operator<<(int v) {
    _AppendInteger(v);
    return *this;
  }

  self_type &operator<<(unsigned int v) {
    _AppendInteger(v);
    return *this;
  }

  self_type &operator<<(long v) {
    _AppendInteger(v);
    return *this;
  }

  self_type &operator<<(unsigned long v) {
    _AppendInteger(v);
    return *this;
  }

  self_type &operator<<(long long v) {
    _AppendInteger(v);
    return *this;
  }

  self_type &operator<<(unsigned long long v) {
    _AppendInteger(v);
    return *this;
  }

  self_type &operator<<(float v) {
    *this << static_cast<double>(v);
    return *this;
  }

  self_type &operator<<(long double v) {
    appendf("%LF", v);
    return *this;
  }

  self_type &operator<<(double v) {
    appendf("%F", v);
    return *this;
  }

  self_type &operator<<(const char *str) {
    if (str != nullptr) {
      append(str);
    } else {
      append("null");
    }
    return *this;
  }

  self_type &operator<<(const void *v) {
    constexpr int kMaxPointerSize = 2 * sizeof(const void *) + 2;
    if (avial() >= kMaxPointerSize) {
      add(ConvertPointer(cur(), v));
    }
    return *this;
  }

  self_type &operator<<(bool v) {
    append(v ? '1' : '0');
    return *this;
  }

  self_type &operator<<(char v) {
    append(v);
    return *this;
  }

  self_type &operator<<(self_type &(*func)(self_type &)) {
    return (*func)(*this);
  }

  void noflush() { auto_flush_ = false; }

  void Reset(const char *buf, int len) {
    ::memcpy(data_, buf, len);
    cur_ = data_ + len;
  }

  void Sink() {
    if (auto_flush_) {
      printf("%s\n", data_);
      reset();
    }
    auto_flush_ = true;
  }

private:
  template <typename T> void _AppendInteger(T v) {
    if (avial() >= kMaxNumericSize) {
      add(ConvertInteger(cur(), v));
    }
  }

private:
  bool auto_flush_ = true;
  DISALLOW_COPY(LogStream);
};

inline LogStream &noflush(LogStream &logstream) {
  printf("fck\n");
  logstream.noflush();
  return logstream;
}

class Logger {
public:
  Logger() {}

  LogStream &stream(LogLevel i, const char *file, int line);

  ~Logger();

private:
  DISALLOW_COPY(Logger);
};
