cmake_minimum_required(VERSION 3.24)
project(ut-tserver)

add_executable(ut-tserver
        src/main.cpp
        src/tests.cpp
        ../../src/TcpServer.cpp
        ../../src/HashEchoClient.cpp
        ../../src/DigestGenerators.cpp
        ../../util/logger.cpp
)

#include(FetchContent)
#
#FetchContent_Declare(
#        googletest
#        GIT_REPOSITORY https://github.com/google/googletest.git
#        GIT_TAG        v1.14.0
#)
#FetchContent_MakeAvailable(googletest)
#
#FetchContent_Declare(
#        poco
#        GIT_REPOSITORY https://github.com/pocoproject/poco.git
#        GIT_TAG        poco-1.14.2-release
#)
#FetchContent_MakeAvailable(poco)


target_link_libraries(ut-tserver PRIVATE gtest Poco::Foundation ssl crypto)
