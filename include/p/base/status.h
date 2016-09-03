// Copyright (c) 2016, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

namespace p {
namespace base {

class Status {
  enum Code {
    kOk = 0,
    kNotFound = 1,
    kCorruption = 2,
    kNotSupported = 3,
    kInvalidArgument = 4,
    kTimedOut = 5,
    kIOError = 6,
    kInProgress = 7,
    kBusy = 8,
    kTryAgain = 9,
  };

  Status() {}

  Status(Code code, const char *msg) {}

  ~Status() { delete[] msg_; }

  Status(const Status &s) {}

  Status(Status &&s) {
    msg_ = s.msg_;
    s.msg_ = nullptr;
  }

  Status &operator=(const Status &s);

  Status &operator=(Status &&s) {
    delete[] msg_;
    msg_ = s.msg_;
    s.msg_ = nullptr;
    return *this;
  }

  Code code() const {
    return msg_ == nullptr ? kOk : static_cast<Code>(msg_[4]);
  }

private:
  // A nullptr msg_ (only for OK) means the message is empty.
  // Otherwise msg_ is a buffer of following form:
  //   msg_[0..3] = message length
  //   msg_[4] = status code
  //   msg_[5..] = msg
  const char *msg_ = nullptr;
};

} // end namespace base
} // end namespace p
