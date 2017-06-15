// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

namespace p {
namespace base {

namespace ThisThread {
extern thread_local int thread_id;
extern thread_local int  thread_name_len;
extern thread_local char thread_name[32];
} // end namespace ThisThread

} // end namespace base
} // end namespace p
