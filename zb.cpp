#include <iostream>

#include "p/base/zbuffer.h"

int main() {
    p::base::ZBuffer buf;
    std::cout << buf.size() << std::endl;
    return 0;
}
