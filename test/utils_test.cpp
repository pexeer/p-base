// Copyright (c) 2016, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "gtest/gtest.h"
#include "p/base/utils.h"

using ::testing::InitGoogleTest;

namespace {

TEST(utils, ConvertInteger) {
    char result[101];
    size_t len = 0;
    // for bool type
    bool i_bool = false;
    len = p::base::ConvertInteger(result, i_bool);
    EXPECT_EQ(len, strlen(result));
    EXPECT_STREQ("0", result);

    i_bool = true;
    len = p::base::ConvertInteger(result, i_bool);
    EXPECT_EQ(len, strlen(result));
    EXPECT_STREQ("1", result);

    // for int
    int i_int = 0;
    len = p::base::ConvertInteger(result, i_int);
    EXPECT_EQ(len, strlen(result));
    EXPECT_STREQ("0", result);

    i_int = -13478;
    len = p::base::ConvertInteger(result, i_int);
    EXPECT_EQ(len, strlen(result));
    EXPECT_STREQ("-13478", result);

    i_int = 18794;
    len = p::base::ConvertInteger(result, i_int);
    EXPECT_EQ(len, strlen(result));
    EXPECT_STREQ("18794", result);

    i_int = 1073741823;
    len = p::base::ConvertInteger(result, i_int);
    EXPECT_EQ(len, strlen(result));
    EXPECT_STREQ("1073741823", result);

    i_int = INT32_MAX;
    len = p::base::ConvertInteger(result, i_int);
    EXPECT_EQ(len, strlen(result));
    EXPECT_STREQ("2147483647", result);

    i_int = INT32_MIN;
    len = p::base::ConvertInteger(result, i_int);
    EXPECT_EQ(len, strlen(result));
    EXPECT_STREQ("-2147483648", result);

    // for uint64_t
    uint64_t i_uint64 = 0;
    len = p::base::ConvertInteger(result, i_uint64);
    EXPECT_EQ(len, strlen(result));
    EXPECT_STREQ("0", result);

    i_uint64 = UINT64_MAX;
    len = p::base::ConvertInteger(result, i_uint64);
    EXPECT_EQ(len, strlen(result));
    EXPECT_STREQ("18446744073709551615", result);
}

TEST(utils, ConvertPointer) {
    char result[101];
    size_t len = 0;

    int x = 1234;
    char tmp[101];
    int *x_addr = &x;
    len = p::base::ConvertPointer(result, x_addr);
    EXPECT_EQ(len, strlen(result));
    sprintf(tmp, "%p", x_addr);
    EXPECT_STREQ(result, tmp + 2);

    x_addr = nullptr;
    len = p::base::ConvertPointer(result, x_addr);
    EXPECT_EQ(len, strlen(result));
    EXPECT_STREQ("0", result);
}

} // anonymous namespace

int main(int argc, char **argv) {
    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
