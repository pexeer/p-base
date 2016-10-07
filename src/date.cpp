// Copyright (c) 2016, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/base/date.h"
#include "p/base/utils.h"

namespace p {
namespace base {

// 20160912-13:34:59.000Z
// 01234567890123456789012345
class DateImpl {
public:
  constexpr static uint32_t kOneDayMS = 24 * 3600 * 1000;
  constexpr static uint32_t kOneDaySec = 24 * 3600;
  constexpr static uint32_t kOneMinMs = 60 * 1000;

  DateImpl() {
    uint64_t ts = gettimeofday_us() / 1000L;
    time_days_ = ts / kOneDayMS;
    time_ms_ = ts % kOneDayMS;
    format_all();
  }

  friend class Date;

private:
  void format_all() {
    time_t time_utc = kOneDaySec * time_days_ + time_ms_ / 1000;
    struct tm local_tm;
    localtime_r(&time_utc, &local_tm);
    snprintf(formatted_, sizeof(formatted_),
             "%04d%02d%02d-%02d:%02d:%02d.%03dZ ", local_tm.tm_year + 1900,
             local_tm.tm_mon, local_tm.tm_mday, local_tm.tm_hour,
             local_tm.tm_min, local_tm.tm_sec, time_ms_ % 1000);
  }

  void update() {
    uint64_t ts = gettimeofday_us() / 1000L;
    if ((ts / kOneDayMS) == time_days_) {
      uint32_t ts_ms = time_ms_;
      time_ms_ = ts % kOneDayMS;
      uint32_t ts_min = time_ms_ / kOneMinMs;
      uint32_t ts_sec = (time_ms_ / 1000) % 60;
      formatted_[15] = ts_sec / 10 + '0';
      formatted_[16] = ts_sec % 10 + '0';

      formatted_[20] = time_ms_ % 10 + '0';
      uint32_t tmp = time_ms_ / 10;
      formatted_[19] = tmp % 10 + '0';
      tmp /= 10;
      formatted_[18] = tmp % 10 + '0';
      if ((ts_ms / kOneMinMs) == ts_min) {
        uint32_t ts_hour = ts_min / 60;
        formatted_[9] = ts_hour / 10 + '0';
        formatted_[10] = ts_hour % 10 + '0';
        ts_min = ts_min % 60;
        formatted_[12] = ts_min / 10 + '0';
        formatted_[13] = ts_min % 10 + '0';
        return;
      }
    } else {
      time_days_ = ts / kOneDayMS;
      time_ms_ = ts % kOneDayMS;
      format_all();
    }
  }

private:
  uint32_t time_days_;
  uint32_t time_ms_;
  char formatted_[24];

  DISALLOW_COPY(DateImpl);
};

// thread cached DateImpl
thread_local static DateImpl tls_date_impl;

const char *Date::c_str() { return tls_date_impl.formatted_; }

int Date::size() { return 23; }

void Date::update() { return tls_date_impl.update(); }

} // end namespace base
} // end namespace p
