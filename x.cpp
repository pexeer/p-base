#include <iostream>
#include <sys/syscall.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

#include "p/base/port.h"

class ThreadId {
public:
  ThreadId() {
    pid = p::base::gettid();
    printf("new ThreadId=%d\n", pid);
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
  for (int i = 0; i < 10; ++i) {
    std::thread x(f);
    x.detach();
    sleep(3);
  }
  sleep(100);
}
