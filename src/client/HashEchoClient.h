//
// Created by vadimzy on 6/1/25.
//

#pragma once
#include <iostream>
#include <thread>
#include <utility>

#include "Poco/Task.h"
#include "../../include/tserver.h"
#include <condition_variable>

class HashEchoClient : public ConnClient {
public:
    HashEchoClient(int fd, ConnManager &m, std::unique_ptr<StreamDigest> d) : connMgr(m), fd(fd), digest(std::move(d)) {}

    ~HashEchoClient() override = default;

    int start() override;

    void stop() override;

    void onData() override;

    ConnClient::State state() const override;

private:
    void handleIO();

    struct connHandler : public Poco::Task {
        explicit connHandler(std::function<void()> f) : Task("connection"), fn(std::move(f)) {}

        ~connHandler() override = default;

        void runTask() override { fn(); }

        std::function<void()> fn{nullptr};
    };

    static Poco::TaskManager &pool();

    ConnManager &connMgr;
    int fd{-1};
    std::atomic_bool terminate{false};
    std::condition_variable cv;
    std::mutex cvMutex;
    std::unique_ptr<StreamDigest> digest;
    std::atomic<State> mState{State::CREATED};
};

class HashEchoClientFactory: public ClientFactory {
public:
    ~HashEchoClientFactory() override = default;
    ConnClient *create(int fd, ConnManager &m) override;
};