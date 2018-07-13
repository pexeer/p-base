// Copyright (c) 2016, pexeer@gmail.com All rightimestamp reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/base/log.h"
#include "p/base/time.h"
#include "p/base/utils.h"
#include <stdio.h>
//#define LOG_TIME_TO_US

namespace p {
namespace base {

LogLevel g_log_level = LogLevel::kTrace;

// 20160912-13:34:59.000Z
// 01234567890123456789012345
// 20160912-13:34:59.000000Z
class LogDateImpl {
public:
    static constexpr uint64_t kMinOneDay = 60UL * 24;
    static constexpr uint64_t kUSOneMin = 1000000UL * 60;

    LogDateImpl() { format_to_string(gettimeofday_us()); }

    const char *update_date() {
        uint64_t timestamp = gettimeofday_us();
        time_us_ = timestamp % kUSOneMin;
        timestamp /= kUSOneMin;
        uint32_t time_min = timestamp % kMinOneDay;
        uint32_t time_day = timestamp / kMinOneDay;

        if (UNLIKELY(time_day != time_day_)) {
            format_to_string(timestamp);
            return formatted_;
        }

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

        if (UNLIKELY(time_min != time_min_)) {
            time_min_ = time_min;
            tmp = time_min % 60;
            formatted_[13] = tmp % 10 + '0';
            formatted_[12] = tmp / 10 + '0';
            tmp = time_min / 60;
            formatted_[10] = tmp % 10 + '0';
            formatted_[9] = tmp / 10 + '0';
        }
        return formatted_;
    }

private:
    void format_to_string(uint64_t timestamp) {
        time_t time_utc_sec = timestamp / 1000000UL;
        struct tm local_tm;
        localtime_r(&time_utc_sec, &local_tm);

// std::strftime
#ifdef LOG_TIME_TO_US
        snprintf(formatted_, sizeof(formatted_), "%04d%02d%02d-%02d:%02d:%02d:%06lu ",
                 local_tm.tm_year + 1900, local_tm.tm_mon + 1, local_tm.tm_mday, local_tm.tm_hour,
                 local_tm.tm_min, local_tm.tm_sec, (unsigned long)(timestamp % 1000000UL));
#else
        snprintf(formatted_, sizeof(formatted_), "%04d%02d%02d-%02d:%02d:%02d:%03lu ",
                 local_tm.tm_year + 1900, local_tm.tm_mon + 1, local_tm.tm_mday, local_tm.tm_hour,
                 local_tm.tm_min, local_tm.tm_sec, (unsigned long)((timestamp % 1000000UL) / 1000));
#endif
        time_us_ = timestamp % kUSOneMin;
        timestamp /= kUSOneMin;
        time_min_ = timestamp % kMinOneDay;
        time_day_ = timestamp / kMinOneDay;
    }

private:
    uint32_t time_day_;
    uint32_t time_min_;
    uint32_t time_us_;
    char formatted_[28];

    P_DISALLOW_COPY(LogDateImpl);
};

// thread cached LogDateImpl
thread_local LogDateImpl tls_date_impl;

const char *LogDate::get_log_date_str() { return tls_date_impl.update_date(); }

} // end namespace base
} // end namespace p
