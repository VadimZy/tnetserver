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
#include <mutex>
#include <sstream>
#include <sys/epoll.h>
#include <unordered_set>

#include "Poco/DigestEngine.h"
#include "Poco/Task.h"
#include "Poco/TaskManager.h"
#include "../include/tserver.h"
#include "../util/logger.h"

#include "HashEchoClient.h"

#include "Poco/MD5Engine.h"


COMMON_LOGGER();

class MD5Digest : public HashDigest {
public:
    MD5Digest() = default;

    void update(void *buff, size_t len) override { engine.update(buff, len); }

    std::string to_hex_string() override {
        auto ret{Poco::DigestEngine::digestToHex(engine.digest())};
        engine.reset();
        return ret;
    };

    ~MD5Digest() override = default;

private:
    Poco::MD5Engine engine;
};


void HashEchoClient::stop() {
    LOG_INFO("fd: %d, stopping client", fd);
    terminate.store(true);
}

void HashEchoClient::handleIO() {
    LOG_INFO("fd: %d, starting IO task", fd);

    using namespace std::chrono_literals;
    while (!terminate.load()) {
        char buffer[1024];
        ssize_t count = read(fd, buffer, sizeof(buffer));
        if (count <= 0) {
            if (errno == EAGAIN) {
                std::this_thread::sleep_for(100ms);
                continue;
            }
            LOG_INFO("fd: %d, read error: %s", fd, strerror(errno));
            close(this->fd);
            connMgr.clientDisconnected(fd);
            break;
        }
        auto sv = std::string_view(buffer, count);
        std::string_view svl, svr;
        if (auto pos = sv.find('\n'); pos != std::string_view::npos) {
            svl = sv.substr(0, pos);
            svr = svr.substr(pos + 1);
        }

        LOG_DEBUG("fd: %d, read size:%ld ", fd, count);

        auto pos = std::find(buffer, buffer + count, '\n');
        if (pos < buffer + count) {
            digest->update(buffer, pos - buffer);

            auto dd = digest->to_hex_string();
            dd += "\n";
            LOG_DEBUG("fd: %d, sending digest:: %s", fd, dd.c_str());
            digest->update(pos + 1, count - (pos - buffer));
            write(fd, dd.data(), dd.size()); // Echo back
        } else {
            digest->update(buffer, count);
        }
    }
    LOG_INFO("fd: %d, completed client", fd);
    close(this->fd);
}

Poco::TaskManager &HashEchoClient::pool() {
    static Poco::TaskManager tm{"connections", 2, static_cast<int>(std::thread::hardware_concurrency())};
    return tm;
}

ConnClient *HashEchoClientFactory::create(int fd, ConnManager &m) {
    return new HashEchoClient(fd, m, std::make_unique<MD5Digest>());
}
