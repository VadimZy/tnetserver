//
// Created by vadimzy on 6/1/25.
//

#pragma once
#include <iostream>
#include <thread>
#include <utility>

#include "Poco/Task.h"
#include "Poco/TaskManager.h"
#include "../include/tserver.h"


class HashEchoClient : public ConnClient {
public:
    HashEchoClient(int fd, ConnManager &m, std::unique_ptr<HashDigest> d) : connMgr(m), fd(fd), digest(std::move(d)) {}

    ~HashEchoClient() override = default;

    int start() override {
        pool().start(new connHandler([this]() { this->handleIO(); }));
        return 0;
    }

    void stop() override;

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
    std::unique_ptr<HashDigest> digest;
};

class HashEchoClientFactory: public ClientFactory {
public:
    ~HashEchoClientFactory() override = default;
    ConnClient *create(int fd, ConnManager &m) override;
};