// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/base/process.h"
#include <stdlib.h>
#include <unistd.h>

namespace p {
namespace base {

namespace Process {
const int kPid = ::getpid();

const char *const kProcName = "TODO";

const uint64_t kPageSize = ::getpagesize();
};

} // end namespace base
} // end namespace p
