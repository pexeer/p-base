// Copyright (c) 2016, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once
#include "p/base/macros.h"

namespace p {
namespace base {

class Date {
public:
  static void update();

  static const char *c_str();

  static int size();

private:
  DISALLOW_COPY(Date);
};

} // end namespace base
} // end namespace p
