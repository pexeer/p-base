#include <iostream>

#include "p/base/zbuffer.h"
#include "p/base/logging.h"

int main() {
    {
        p::base::ZBuffer buf0;
        p::base::ZBuffer buf1;

        for (int i = 0; i < 10090; ++i) {
            buf0.append("0123456789", 10);
            buf1.append("abcdefgh", 8);
            std::cout << buf0.size() << std::endl;
            std::cout << buf1.size() << std::endl;
        }
    }
    LOG_WARN << "number," << p::base::ZBuffer::total_block_number();
    LOG_WARN << "memory," << p::base::ZBuffer::total_block_memory();

    return 0;
}
