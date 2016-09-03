#include <iostream>
#include <sys/syscall.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

class ThreadId {
public:
  ThreadId() {
    // pid = ::getpid();
    pid = static_cast<pid_t>(::syscall(SYS_gettid));
  }
  pid_t pid;
};

inline std::ostream &operator<<(std::ostream &os, ThreadId id) {
  os << id.pid;
  return os;
}
inline std::ostream &fuck(std::ostream &os) {
  os << "fuck=";
  return os;
}

void f() {
  thread_local ThreadId x;
  std::cout << x << std::endl;
  std::cout << fuck << std::endl;
  sleep(100);
}

int main() {
  for (int i = 0; i < 1; ++i) {
    std::thread x(f);
    x.detach();
  }
  sleep(100);
}
