// Copyright (c) 2016, pexeer@gmail.com All rightimestamp reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/base/date.h"
#include "p/base/utils.h"

//#define LOG_TIME_TO_US

namespace p {
namespace base {

// 20160912-13:34:59.000Z
// 01234567890123456789012345
// 20160912-13:34:59.000000Z
class DateImpl {
public:
  static constexpr uint64_t kMinOneDay = 60L * 24;
  static constexpr uint64_t kUSOneMin = 1000000L * 60;
  DateImpl() { format(gettimeofday_us()); }

  friend class Date;

private:
  void format(uint64_t timestamp) {
    time_t time_utc_sec = timestamp / 1000000L;
    struct tm local_tm;
    localtime_r(&time_utc_sec, &local_tm);

#ifdef LOG_TIME_TO_US
    snprintf(formatted_, sizeof(formatted_),
             "%04d%02d%02d-%02d:%02d:%02d.%06luZ ", local_tm.tm_year + 1900,
             local_tm.tm_mon, local_tm.tm_mday, local_tm.tm_hour,
             local_tm.tm_min, local_tm.tm_sec, timestamp % 1000000L);
#else
    snprintf(formatted_, sizeof(formatted_),
             "%04d%02d%02d-%02d:%02d:%02d.%03luZ ", local_tm.tm_year + 1900,
             local_tm.tm_mon, local_tm.tm_mday, local_tm.tm_hour,
             local_tm.tm_min, local_tm.tm_sec, (timestamp % 1000000L) / 1000);
#endif
    time_us_ = timestamp % kUSOneMin;
    timestamp /= kUSOneMin;
    time_min_ = timestamp % kMinOneDay;
    time_day_ = timestamp / kMinOneDay;
  }

  void update() {
    uint64_t timestamp = gettimeofday_us();
    time_us_ = timestamp % kUSOneMin;
    timestamp /= kUSOneMin;
    uint32_t time_min = timestamp % kMinOneDay;
    uint32_t time_day = timestamp / kMinOneDay;

    if (time_day == time_day_) {
#ifdef LOG_TIME_TO_US
      uint32_t tmp = time_us_;
      formatted_[23] = tmp % 10 + '0';
      tmp /= 10;
      formatted_[22] = tmp % 10 + '0';
      tmp /= 10;
      formatted_[21] = tmp % 10 + '0';
      tmp /= 10;
      formatted_[20] = tmp % 10 + '0';
      tmp /= 10;
      formatted_[19] = tmp % 10 + '0';
      tmp /= 10;
      formatted_[18] = tmp % 10 + '0';
      tmp /= 10;
      formatted_[16] = tmp % 10 + '0';
      formatted_[15] = tmp / 10 + '0';
#else
      uint32_t tmp = time_us_ / 1000;
      formatted_[20] = tmp % 10 + '0';
      tmp /= 10;
      formatted_[19] = tmp % 10 + '0';
      tmp /= 10;
      formatted_[18] = tmp % 10 + '0';
      tmp /= 10;
      formatted_[16] = tmp % 10 + '0';
      formatted_[15] = tmp / 10 + '0';
#endif

      if (time_min == time_min_) {
        return;
      } else {
        time_min_ = time_min;
        tmp = time_min % 60;
        formatted_[13] = tmp % 10 + '0';
        formatted_[12] = tmp / 10 + '0';
        tmp = time_min / 60;
        formatted_[10] = tmp % 10 + '0';
        formatted_[9] = tmp / 10 + '0';
      }
    } else {
      format(timestamp);
    }
  }

private:
  uint32_t time_day_;
  uint32_t time_min_;
  uint32_t time_us_;
  char formatted_[28];

  DISALLOW_COPY(DateImpl);
};

// thread cached DateImpl
thread_local static DateImpl tls_date_impl;

const char *Date::c_str() { return tls_date_impl.formatted_; }

int Date::size() {
#ifdef LOG_TIME_TO_US
  return 26;
#else
  return 23;
#endif
}

void Date::update() { return tls_date_impl.update(); }

} // end namespace base
} // end namespace p
