#pragma once

#include <atomic>
#include <list>
#include <memory>
#include <netinet/in.h>
#include <unordered_map>

#include "../include/tserver.h"

namespace Poco {
    class TaskManager;
}

class TcpServer : public ConnManager {
public:
    explicit TcpServer(int port, std::unique_ptr<ClientFactory> cf);

    ~TcpServer() override;

    void clientDisconnected(int fd) override{};

    int run();
    void shutdown();

private:
    sockaddr_in addr{};
    int server_fd{-1};
    int epoll_fd{-1};

    std::atomic<bool> stop{false};
    std::unique_ptr<ClientFactory> clientFactory;
    std::unique_ptr<Poco::TaskManager> taskManager;
};
