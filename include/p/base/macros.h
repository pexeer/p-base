// Copyright (c) 2015, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#define DISALLOW_COPY(TypeName)                 \
    TypeName(const TypeName&)  = delete;        \
    TypeName(const TypeName&&) = delete;        \
    void operator=(const TypeName&) = delete;   \
    void operator=(const TypeName&&) = delete
