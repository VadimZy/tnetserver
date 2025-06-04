cmake_minimum_required(VERSION 3.24)
project(tnetserver)

#-Wno-deprecated

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -ggdb")

set(ENABLE_PAGECOMPILER OFF)
set(ENABLE_PAGECOMPILER_FILE2PAGE OFF)
set(ENABLE_ACTIVERECORD OFF)
set(ENABLE_ACTIVERECORD_COMPILER OFF)

include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
        poco
        GIT_REPOSITORY https://github.com/pocoproject/poco.git
        GIT_TAG        poco-1.14.2-release
)
FetchContent_MakeAvailable(poco)

add_subdirectory(src)
add_subdirectory(test)
