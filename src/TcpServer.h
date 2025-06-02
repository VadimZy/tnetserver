#pragma once

#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include <netinet/in.h>
#include <thread>
#include <unordered_map>
#include <vector>

#include "../include/tserver.h"

class TcpServer : public ConnManager {
public:
    explicit TcpServer(int port, std::unique_ptr<ClientFactory> cf);

    ~TcpServer() override;

    void clientDisconnected(int fd) override{};

    int run();
    void shutdown();

private:
    void cleanup();
    sockaddr_in addr{};
    int server_fd{-1};
    int epoll_fd{-1};
    std::unordered_map<int, std::shared_ptr<ConnClient>> connMap{};
    std::list<std::shared_ptr<ConnClient>> connDone;

    std::atomic<bool> stop{false};
    std::unique_ptr<ClientFactory> clientFactory;
};
