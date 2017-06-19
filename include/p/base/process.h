// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

namespace p {
namespace base {

class Process {
public:
    static int pid();

    static int tid();

    static Slice tid_str();
};

}
}
