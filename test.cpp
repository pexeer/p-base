
#include "p/base/string_printf.h"

#include <string.h>
#include <iostream>
#include <bitset>

constexpr const char* c_rfindchar(const char* src, size_t len, const char ch)
{
    return len ? (src[len - 1] == ch ? (src + len) : c_rfindchar(src, len - 1, ch)) : NULL;
}

constexpr const size_t c_strlen(const char* src)
{
    return (*src) ? 1 + c_strlen(src + 1) : 0;
}

constexpr const char* c_strrchr(const char* src, const char ch)
{
    return c_rfindchar(src, c_strlen(src), ch);
}

template<int N>
constexpr const char* findrchr(const char (&src)[N], const char ch)
{
    return c_strrchr(src, ch) ? c_strrchr(src, ch) : &(src[0]);
}



int test(int ret) {
    std::cout << "in test=" << ret << std::endl;
    return ret;
}

int main(void) {
    std::string z = "\1我\1的\1世\1界\1!";

    std::cout << "sizeof(wchar_t)=" << sizeof(wchar_t) << std::endl;
    std::cout << "sizeof(char32_t)=" << sizeof(char16_t) << std::endl;
    std::cout << "sizeof(char16_t)=" << sizeof(char16_t) << std::endl;
    std::cout << "sizeof(char)=" << sizeof(char) << std::endl;
    std::cout << "sizeof(bool)=" << sizeof(bool) << std::endl;
    struct Foo {
        bool a;
        bool b;
    };
    std::cout << "sizeof(Foo)=" << sizeof(struct Foo) << std::endl;

    constexpr const char* source_file = findrchr("/hodf/sldkjf/sdkfj/", '/');

    std::cout << "source_file:" << source_file << std::endl;
    std::cout << "source_file:" << findrchr(__FILE__, 's') << std::endl;

    for (auto &i : z) {
        if (i == 1) {
            std::cout << std::endl << "12345678" << std::endl;
            continue;
        }
        std::bitset<sizeof(i) * 8> bits(i);
        std::cout << bits << std::endl;
    }



    std::string x;
    int ret = p::base::StringAppendf(&x, " ");
    std::cout << ret << x << std::endl;
    x.reserve(4);
    ret = p::base::StringAppendf(&x, "abcd");
    std::cout << ret << x << std::endl;
    x.reserve(4);
    ret = p::base::StringAppendf(&x, "abcdef我");
    std::cout << ret << x << std::endl;
    x.reserve(4);
    ret = p::base::StringAppendf(&x, "a");
    std::cout << ret << x << std::endl;
    ret = p::base::StringAppendf(&x, "%f%p,", 1.23, "hello,fuck");
    std::cout << ret << x << std::endl;

    return 0;
}
