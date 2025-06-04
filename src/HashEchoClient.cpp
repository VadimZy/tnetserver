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

#include "../util/Configuration.h"
#include "DigestGenerators.h"


COMMON_LOGGER();

HashEchoClient::HashEchoClient(int fd, ConnManager &m, std::unique_ptr<StreamDigest> d) :
    connMgr(m), fd(fd), digest(std::move(d)) {
    LOG_INFO("client: %lu, id, fd: %d, constructing", id, fd);
}

HashEchoClient::~HashEchoClient() { std::cout << "dtor client: " << id << "\n"; }

int HashEchoClient::start() {
    static size_t buffSize = util::Configuration::instance().readBufferSize();
    std::vector<char> buffer(buffSize);

    LOG_INFO("client: %lu, id, fd: %d, starting IO task", id, fd);

    while (!terminate.load()) {

        auto count = read(fd, buffer.data(), buffSize);
        if (count <= 0) {

            if (errno == EAGAIN) {
                continue;
            }

            LOG_INFO("client: %lu, id, fd: %d, read condition: %s", id, fd, strerror(errno));
            break;
        }

        LOG_DEBUG("client: %lu, id, fd: %d, read size:%ld ", id, fd, count);

        auto write_hash = [this](std::string s) {
            LOG_DEBUG("client: %lu, id, fd: %d, echoing hash: %s ", id, fd, s.c_str());
            s += '\n';
            write(fd, s.data(), s.size());
            return 0;
        };

        digest->append(write_hash, {buffer.data(), buffSize});
    }

    close(fd);

    LOG_INFO("client: %lu, id, fd: %d, completed", id, fd);
    mState = State::COMPLETED;
    return 0;
}

void HashEchoClient::stop() {
    LOG_INFO("client: %lu, id, fd: %d, stopping client", id, fd);
    terminate.store(true);
}

ConnClient::State HashEchoClient::state() const { return mState; }


ConnClient *HashEchoClientFactory::create(int fd, ConnManager &m) {

    if (util::Configuration::instance().hashType() == "md5") {
        return new HashEchoClient(fd, m, std::make_unique<StreamMD5Digest>('n'));
    }
    throw std::runtime_error(util::Configuration::instance().hashType() + " not supported");
}
