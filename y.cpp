#include <iostream>
#include <unistd.h>
#include <thread>

#include "p/base/logging.h"

using namespace p::base;

void f(int k) {
  for (int i = 0; i < k; ++i) {
    for (int j = 0; j < 10000; ++j) {
      FastLogger().log_generator(LogLevel::kDebug, __FILE__, __LINE__)
          << "xxxxx, i = " << i << ": i*j=" << i * j << ":i+j=" << i + j
          << "j=" << j
          << "ts=\t" << p::base::gettimeofday_us();
    }
  }
}

int main() {
  std::vector<std::thread*> threads;
  for (int i = 100; i < 110; ++i) {
    threads.push_back(new std::thread(f, i + 100));
  }

  for (int i = 0; i < 10; ++i) {
    threads[i]->join();
  }

  return 0;
}
