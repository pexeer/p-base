// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

// http://xoroshiro.di.unimi.it
// http://xoroshiro.di.unimi.it/xoroshiro128plus.c

#include "p/base/rand.h"
#include <random>
#include <limits>

namespace p {
namespace base {

// Written in 2016 by David Blackman and Sebastiano Vigna (vigna@acm.org)

/* This is the successor to xorshift128+. It is the fastest full-period
   generator passing BigCrush without systematic failures, but due to the
   relatively short period it is acceptable only for applications with a
   mild amount of parallelism; otherwise, use a xorshift1024* generator.

   Beside passing BigCrush, this generator passes the PractRand test suite
   up to (and included) 16TB, with the exception of binary rank tests,
   which fail due to the lowest bit being an LFSR; all other bits pass all
   tests. We suggest to use a sign test to extract a random Boolean value.
   
   Note that the generator uses a simulated rotate operation, which most C
   compilers will turn into a single instruction. In Java, you can use
   Long.rotateLeft(). In languages that do not make low-level rotation
   instructions accessible xorshift128+ could be faster.

   The state must be seeded so that it is not everywhere zero. If you have
   a 64-bit seed, we suggest to seed a splitmix64 generator and use its
   output to fill s. */

static inline uint64_t rotl(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

thread_local class FastRand {
public:
    FastRand() {
        std::random_device rd;
        std::mt19937 mt(rd());
        s[0] = mt();
        s[1] = mt();
    }

    uint64_t next() {
        const uint64_t s0 = s[0];
        uint64_t s1 = s[1];
        const uint64_t result = s0 + s1;

        s1 ^= s0;
        s[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14); // a, b
        s[1] = rotl(s1, 36); // c
        return result;
    }

    /* This is the jump function for the generator. It is equivalent
       to 2^64 calls to next(); it can be used to generate 2^64
       non-overlapping subsequences for parallel computations. */
    void jump(void) {
        static const uint64_t JUMP[] = { 0xbeac0467eba5facb, 0xd86b048b86aa9922 };

        uint64_t s0 = 0;
        uint64_t s1 = 0;
        for(size_t i = 0; i < (sizeof(JUMP) / sizeof(JUMP[0])); i++) {
            for(int b = 0; b < 64; b++) {
                if (JUMP[i] & UINT64_C(1) << b) {
                    s0 ^= s[0];
                    s1 ^= s[1];
                }
                next();
            }
        }

        s[0] = s0;
        s[1] = s1;
    }
private:
    uint64_t s[2];
} tls_fast_rand;

uint64_t fast_rand() {
    return tls_fast_rand.next();
}

uint64_t fast_rand(uint64_t rand_max) {
    if (rand_max) {
        uint64_t range = std::numeric_limits<uint64_t>::max() / rand_max;
        uint64_t ret;
        do {
            ret = tls_fast_rand.next() / range;
        } while (ret >= rand_max);
        return ret;
    }
    return 0;
}

} // end namespace base
} // end namespace p
