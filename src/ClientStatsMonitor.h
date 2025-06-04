//
// Created by vadimzy on 6/3/25.
//

#pragma once
#include <atomic>


#include "../include/tserver.h"

class ClientStatsMonitor : public ConnMonitor {
public:
    ~ClientStatsMonitor() override = default;

    void clientCreated(int fd) override;
    void clientStatusChanged(int fd, ConnClient::State st, ConnClient::State old) override;
    void clientDeleted(int fd) override;
    void clientError(int fd, int errNo) override;
    void shutdown() override;

private:
    std::atomic<int> created{0};
    std::atomic<int> deleted{0};
    std::atomic<int> running{0};
    std::atomic<int> completed{0};
    std::atomic<int> failed{0};
    std::atomic<int> errors{0};
    std::atomic<bool> closed{};
};
