//
// Created by vadimzy on 6/1/25.
//
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <utility>
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

#include "Configuration.h"
#include "DigestGenerators.h"


COMMON_LOGGER();

HashEchoClient::HashEchoClient(int fd, std::shared_ptr<ConnMonitor> m, std::unique_ptr<StreamDigest> d) :
    connMonitor(std::move(m)), fd(fd), digest(std::move(d)) {
    connMonitor->clientCreated(fd);
    LOG_TRACE("client: %lu, id, fd: %d, constructing", id, fd);
}

HashEchoClient::~HashEchoClient() { connMonitor->clientDeleted(fd); }

int HashEchoClient::start() {
    static size_t buffSize = Configuration::instance().readBufferSize();
    std::vector<char> buffer(buffSize);

    LOG_TRACE("client: %lu, id, fd: %d, starting IO task", id, fd);

    connMonitor->clientStatusChanged(fd, State::RUNNING, mState );
    mState = State::RUNNING;

    while (!terminate.load()) {

        auto count = read(fd, buffer.data(), buffSize);
        if (count <= 0) {

            if (errno == EAGAIN) {
                continue;
            }

            if (count != 0) {
                connMonitor->clientError(fd, errno);
            }

            LOG_TRACE("client: %lu, id, fd: %d, read condition: %s", id, fd, strerror(errno));
            break;
        }

        LOG_TRACE("client: %lu, id, fd: %d, read size:%ld ", id, fd, count);

        auto write_hash = [this](std::string s) {
            LOG_DEBUG("client: %lu, id, fd: %d, echoing hash: %s ", id, fd, s.c_str());
            s += '\n';
            write(fd, s.data(), s.size());
            return 0;
        };

        digest->append(write_hash, {buffer.data(), buffSize});
    }

    close(fd);

    LOG_TRACE("client: %lu, id, fd: %d, completed", id, fd);
    mState = State::COMPLETED;
    connMonitor->clientStatusChanged(fd, mState, State::RUNNING);
    return 0;
}

void HashEchoClient::stop() {
    LOG_TRACE("client: %lu, id, fd: %d, stopping client", id, fd);
    terminate.store(true);
}


ConnClient *HashEchoClientFactory::create(int fd, std::shared_ptr<ConnMonitor> m) {

    if (Configuration::instance().hashType() == "md5") {
        return new HashEchoClient(fd, m, std::make_unique<StreamMD5Digest>('n'));
    }
    throw std::runtime_error(Configuration::instance().hashType() + " not supported");
}
