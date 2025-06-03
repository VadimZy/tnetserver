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
    HashEchoClient(int fd, ConnManager &m, std::unique_ptr<StreamDigest> d);

    ~HashEchoClient() override;

    int start() override;

    void stop() override;

    void onData() override;

    ConnClient::State state() const override;

private:
    void handleIO();

    static Poco::TaskManager &pool();

    ConnManager &connMgr;
    int fd{-1};
    std::atomic_bool terminate{false};
    std::unique_ptr<StreamDigest> digest;
    std::atomic<State> mState{State::CREATED};
    static uint64_t nextId() {
        static std::atomic<uint64_t> _{0};
        return _.fetch_add(1);
    }
    uint64_t id{0};
};

class HashEchoClientFactory : public ClientFactory {
public:
    ~HashEchoClientFactory() override = default;
    ConnClient *create(int fd, ConnManager &m) override;
};
