// Copyright (c) 2015, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

namespace p {
namespace base {

class String {
public:
    String():data_(nullptr), len_(0) {}

    String(const char* buf) {

    }

private:
    char*   data_;
    size_t  len_;
  DISALLOW_COPY(String);
};

} // end namespace base
} // end namespace p
