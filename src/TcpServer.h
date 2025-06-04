#pragma once

#include <atomic>
#include <memory>
#include <netinet/in.h>

#include "../include/tserver.h"

namespace Poco {
    class TaskManager;
}

class TcpServer  {
public:
    TcpServer(int port, std::unique_ptr<ClientFactory> cf);

    ~TcpServer() ;

    int run();
    void shutdown();

private:
    sockaddr_in addr{};
    int server_fd{-1};
    int epoll_fd{-1};

    std::atomic<int> stop{0};
    std::unique_ptr<ClientFactory> clientFactory;
    std::unique_ptr<Poco::TaskManager> taskManager;

};
