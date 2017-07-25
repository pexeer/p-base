// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

namespace p {
namespace base {

class ThisThread {
public:
    ThisThread();

    static int thread_id();

    static const char *thread_name();

    static int thread_name_len();

private:
    int thread_id_;
    int thread_name_len_;
    char thread_name_[24];
};

extern thread_local ThisThread tls_this_thread;

inline int ThisThread::thread_id() { return tls_this_thread.thread_id_; }

inline const char *ThisThread::thread_name() { return tls_this_thread.thread_name_; }

inline int ThisThread::thread_name_len() { return tls_this_thread.thread_name_len_; }

} // end namespace base
} // end namespace p
