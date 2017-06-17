#include <string.h>
#include <stdio.h>

/*
constexpr const char* str_end(const char *str) {
    return *str ? str_end(str + 1) : str;
}
constexpr bool has_slash(const char *str) {
    return *str == '/' ? true : ( *str ? has_slash(str + 1) : false);
}
constexpr const char* rfind_slash(const char* str) {
    return *str == '/' ? (str + 1) : rfind_slash(str - 1);
}
constexpr const char* file_name(const char* str) {
    return has_slash(str) ? rfind_slash(str_end(str)) : str;
}
*/

#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

int main() {
    //constexpr const char* const_file = file_name(__FILE__);

    constexpr const char* const tmp = __FILENAME__;
    puts(tmp);
    return 0;
}

