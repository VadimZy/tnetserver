cmake_minimum_required(VERSION 3.24)
project(tnetserver)

#-Wno-deprecated

set(CMAKE_CXX_STANDARD 20)

add_executable(tnetserver main.cpp
        util/logger.cpp
        util/logger.h
        include/tserver.h
        server/TcpServer.cpp
        server/TcpServer.h
        client/HashEchoClient.cpp
        client/HashEchoClient.h
        util/util.cpp
        util/util.h)

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


target_link_libraries(tnetserver PRIVATE Poco::Foundation ssl crypto)

