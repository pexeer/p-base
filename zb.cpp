#include <iostream>

#include "p/base/zbuffer.h"
#include "p/base/logging.h"

int main() {
    for (int j = 0; j < 100; ++j) {
    {
        p::base::ZBuffer buf0;
        p::base::ZBuffer buf1;

        for (int i = 0; i < 10; ++i) {
            buf0.append("0123456789", 10);
            buf1.append("abcdefgh", 7);
            std::cout << buf0.size() << std::endl;
            std::cout << buf1.size() << std::endl;
        }

         char buf[1000];
         for (int i = 0; i < 100; ++i) {
            int ii = buf0.popn(buf, j);
            std::string tmp(buf, ii);
            std::cout << "jj=" << ii << "," << tmp << std::endl;
         }

         for (int i = 0; i < 100; ++i) {
            int ii = buf1.popn(buf, j);
            std::string tmp(buf, ii);
            std::cout << "jj=" << ii << "," << tmp << std::endl;
        }
        std::cout << std::endl;
        std::cout << "====================================" << std::endl;
    }
    }


    LOG_WARN << "number," << p::base::ZBuffer::total_block_number();
    LOG_WARN << "memory," << p::base::ZBuffer::total_block_memory();

    return 0;
}
