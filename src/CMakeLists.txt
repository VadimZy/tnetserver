
add_executable(tnetserver main.cpp
        ../include/tserver.h
        TcpServer.cpp
        TcpServer.h
        HashEchoClient.cpp
        HashEchoClient.h
        DigestGenerators.cpp
        DigestGenerators.h
        ../util/logger.cpp
        ../util/logger.h
        ../util/util.cpp
        ../util/util.h
        Configuration.cpp
        Configuration.h
        ClientStatsMonitor.h
        ClientStatsMonitor.cpp)

target_link_libraries(tnetserver PRIVATE Poco::Foundation Poco::Util ssl crypto)
