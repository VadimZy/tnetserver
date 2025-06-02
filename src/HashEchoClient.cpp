//
// Created by vadimzy on 6/1/25.
//
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include <iomanip>
#include <openssl/evp.h>

#include <Poco/ThreadPool.h>
#include <condition_variable>
#include <cstring>
#include <fcntl.h>
#include <sys/epoll.h>

#include "../include/tserver.h"
#include "../util/logger.h"
#include "Poco/DigestEngine.h"
#include "Poco/Task.h"
#include "Poco/TaskManager.h"

#include "HashEchoClient.h"

#include "DigestGenerators.h"
#include "Poco/MD5Engine.h"


COMMON_LOGGER();


int HashEchoClient::start() {

    auto c = pool().count();
    try {
        pool().start(new connHandler([this]() { this->handleIO(); }));
        mState =  State::RUNNING;
    } catch (const std::exception &e) {
        LOG_ERROR("Exception while starting hash echo client: %s", e.what());
        close(fd);
        mState = State::FAILED;
        return -1;
    }

    LOG_INFO("fd: %d, starting client task, before: %d, after: %d", fd, c, pool().count());
    return 0;
}

void HashEchoClient::stop() {
    LOG_INFO("fd: %d, stopping client", fd);
    terminate.store(true);
    cv.notify_one();
}

void HashEchoClient::onData() {
    LOG_INFO("fd: %d, data", fd);
    cv.notify_one();
}

ConnClient::State HashEchoClient::state() const {
    return mState;
}

void HashEchoClient::handleIO() {
    LOG_INFO("fd: %d, starting IO task", fd);

    using namespace std::chrono_literals;
    while (!terminate.load()) {
        char buffer[10];

        ssize_t count = read(fd, buffer, sizeof(buffer));
        if (count <= 0) {
            if (errno == EAGAIN) {
                auto lk = std::unique_lock(cvMutex);
                //cv.wait(lk);
                //LOG_INFO("fd: %d, read error: %s", fd, strerror(errno));
                continue;
            }
            LOG_INFO("fd: %d, read error: %s", fd, strerror(errno));
            close(this->fd);
            connMgr.clientDisconnected(fd);
            break;
        }

        LOG_DEBUG("fd: %d, read size:%ld ", fd, count);
        auto write_hash = [this](std::string s) {
            LOG_DEBUG("fd: %d, echoing hash: %s ", fd, s.c_str());
            write(fd, s.data(), s.size());
            return 0;
        };
        digest->append(write_hash, {buffer, (size_t) count});
    }

    LOG_INFO("fd: %d, completed client", fd);
    close(this->fd);
    mState =  State::COMPLETED;
}

Poco::TaskManager &HashEchoClient::pool() {
    static Poco::TaskManager tm{"connections", 2, static_cast<int>(std::thread::hardware_concurrency())};
    return tm;
}


ConnClient *HashEchoClientFactory::create(int fd, ConnManager &m) {
    return new HashEchoClient(fd, m, std::make_unique<StreamMD5Digest>('n'));
}
