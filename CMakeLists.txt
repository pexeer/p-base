cmake_minimum_required(VERSION 2.8)
#set(CMAKE_VERBOSE_MAKEFILE ON)

project(p-base)

enable_language(CXX C ASM)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
###################################################################################

# file(WRITE ${PROJECT_BINARY_DIR}/a.c "void a(void){}\n")

include_directories("include")

#aux_source_directory(src P_BASE_SRC)
#file(GLOB P_BASE_SRC "src/*.cpp")
list(APPEND P_BASE_SRC
    src/arena.cpp
    src/endpoint.cpp
    src/log.cpp
    src/logger.cpp
    src/process.cpp
    src/rand.cpp
    src/socket.cpp
    src/this_thread.cpp
    src/timer.cpp
    src/utils.cpp
    src/zbuffer.cpp
    )


add_library(p-base STATIC ${P_BASE_SRC})
#target_link_libraries(P-base base)

