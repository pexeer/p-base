#include <iostream>
#include <cstdint>
#include <cstdio>

struct LogEntry {
  struct LogEntry *next;
  uint32_t data_len;
  char data[4];
};

int main() {
  std::cout << "sizeof(LogEntry) = " << sizeof(LogEntry) << std::endl;
  LogEntry sentry;
  sentry.data[0] = 0;
  sentry.data[1] = 1;
  sentry.data[2] = 2;
  sentry.data[3] = 3;
  sentry.next = (LogEntry*)0x0A0B0C0D0E;
  sentry.data_len = -1;


  char G[18] ={"0123456789ABCDEFG"};
  char * buf = reinterpret_cast<char*>(&sentry);
  for (size_t i = 0; i < sizeof(LogEntry); ++i) {
    unsigned char tmp = buf[i];
    printf("%c%c ", G[tmp >> 4], G[tmp & 0xF]);
  }
  printf("\n");


  std::cout << "alignof(LogEntry) = " << alignof(LogEntry) << std::endl;

  typedef std::aligned_storage<alignof(LogEntry), sizeof(LogEntry)>::type X;

  std::cout << tmp << std::endl;
  return 0;
}
