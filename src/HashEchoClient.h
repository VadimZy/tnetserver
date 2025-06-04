//
// Created by vadimzy on 6/1/25.
//

#pragma once
#include <iostream>
#include <thread>
#include <utility>

#include <condition_variable>
#include "../include/tserver.h"
#include "Poco/Task.h"

class HashEchoClient : public ConnClient {
public:
    HashEchoClient(int fd, std::shared_ptr<ConnMonitor> m, std::unique_ptr<StreamDigest> d);

    ~HashEchoClient() override;

    int start() override;

    void stop() override;


private:

    int fd{-1};

    std::shared_ptr<ConnMonitor> connMonitor;
    std::unique_ptr<StreamDigest> digest;

    std::atomic_bool terminate{false};
    std::atomic<State> mState{State::CREATED};

    static uint64_t nextId() {
        static std::atomic<uint64_t> _{0};
        return _.fetch_add(1);
    }
    uint64_t id{nextId()};
};

class HashEchoClientFactory : public ClientFactory {
public:
    ~HashEchoClientFactory() override = default;
    ConnClient *create(int fd, std::shared_ptr<ConnMonitor> m) override;
};
