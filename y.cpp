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

void ff(int k) {
  for (int i = 0; i < k; ++i) {
    for (int j = 0; j < 10; ++j) {
      FastLogger().log_generator(LogLevel::kDebug, __FILE__, __LINE__)
          << "xxxxx, i = " << i << ": i*j=" << i * j << ":i+j=" << i + j
          << "j=" << j
          << "ts=\t" << p::base::gettimeofday_us();
    }
  }
}

int main() {
  /*
  std::vector<std::thread*> threads;
  for (int i = 100; i < 132; ++i) {
    threads.push_back(new std::thread(f, i + 100));
  }

  for (int i = 0; i < 32; ++i) {
    threads[i]->join();
  }
  */

  ff(10);
  FastLogger().log_generator(LogLevel::kDebug, __FILE__, __LINE__)
    << "fuck0";
  std::cout << "fuck0" << std::endl;
  sleep(10);

  FastLogger().log_generator(LogLevel::kDebug, __FILE__, __LINE__)
    << "fuck1";

  std::cout << "fuck1" << std::endl;
  ff(10);

  FastLogger().log_generator(LogLevel::kDebug, __FILE__, __LINE__)
    << "fuck2";
  std::cout << "fuck2" << std::endl;
  sleep(10);

  FastLogger().log_generator(LogLevel::kDebug, __FILE__, __LINE__)
    << "fuck3";
  std::cout << "fuck3" << std::endl;
  ff(1);


  return 0;
}
