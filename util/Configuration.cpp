//
// Created by vadimzy on 6/3/25.
//

#include "Configuration.h"

#include <sstream>

#include "logger.h"
COMMON_LOGGER();
namespace util {

    void Configuration::loadFile(const std::string &p) {

        try {
            load("tserver.cfg");
        } catch (std::exception &e) {
            LOG_INFO("Configuration file not found, using default values");
            return;
        }

        auto readVal = [this](auto &val, auto f, const char *key) {
            std::stringstream ss;
            try {
                auto newVal = f(key);
                ss << newVal;
                LOG_INFO("'%s' found: in configuration, using %s", key, ss.str().c_str());
                val = newVal;
            } catch (std::exception &e) {
                ss << val;
                LOG_INFO("'%s' not configured, using default %s", key, ss.str().c_str());
            }
        };

#define RFN(f) [this](const char *p) { return f(p); }

        readVal(serverPort_, RFN(getInt), "server.port");
        readVal(maxThreads_, RFN(getInt), "server.max_threads");
        readVal(epollEventsNum_, RFN(getInt), "server.epoll_events_num");
        readVal(readBufferSize_, RFN(getInt), "server.read_buffer_size");
        readVal(hashType_, RFN(getString), "server.hash_type");

    }
} // namespace util
