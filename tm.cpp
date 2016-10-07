#include <stdio.h>
#include <time.h>
#include <sys/time.h>

//20160912-13:34:59.000Z 30578 DEBUG [main:test.cpp:3647] dlfjd
//20160912-13:34:59.000Z 30578 WARN  [main:test.cpp:3647] dlfjd
//
//DEBUG 20160912-13:34:59.000Z 30578 [main:test.cpp:3647] dlfjd
//DEBUG 20160912-13:34:59.000Z 30578 [main:test.cpp:3647] dlfjd
//DEBUG 20160912-13:34:59.000Z 30578 [main:test.cpp:3647] dlfjd
//DEBUG 20160912-13:34:59.000Z 30578 [main:test.cpp:3647] dlfjd
//DEBUG 20160912-13:34:59.000Z 30578 [main:test.cpp:3647] dlfjd
//DEBUG 20160912-13:34:59.000Z 30578 [main:test.cpp:3647] dlfjd
//DEBUG 20160912-13:34:59.000Z 30578 [main:test.cpp:3647] dlfjd
//      0123456789abcdef089012
class DateFormat {

};

int main(void) {
  struct timeval ts;
  gettimeofday(&ts, nullptr);
  char buff[100];
  strftime(buff, sizeof buff, "%D %T", gmtime(&ts.tv_sec));
  printf("Current time: %s.%09ld UTC\n", buff, ts.tv_usec);

  time_t time_utc = ts.tv_sec;
  struct tm local_tm;
  localtime_r(&time_utc, &local_tm);

  printf("Local time: %s\n", asctime(&local_tm));

  return 0;
}
