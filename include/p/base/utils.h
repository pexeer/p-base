// Copyright (c) 2015, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <cstdint>
#include <algorithm>

namespace p {
namespace base {

// Efficient Integer to String Conversions, by Matthew Wilson.
// copy code from muduo, https://github.com/chenshuo/muduo
template<typename T>
size_t ConvertInteger(char *buf, T value);

size_t ConvertPointer(char *buf, const void *value);

} // end namespace base
} // end namespace p
