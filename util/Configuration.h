//
// Created by vadimzy on 6/3/25.
//
#pragma once
#include <thread>
#include "Poco/Util/IniFileConfiguration.h"

namespace util {

    class Configuration : Poco::Util::IniFileConfiguration {
    public:
        ~Configuration() override = default;
        void loadFile(const std::string &p);
        static Configuration &instance() {
            static Configuration instance;
            return instance;
        }

        int maxThreads() const { return maxThreads_; }
        int serverPort() const { return serverPort_; }
        int epollEventsNum() const { return epollEventsNum_; }
        const std::string &hashType() const { return hashType_; }
        int readBufferSize() const { return readBufferSize_; }

    private:
        Configuration() = default;
        // default values
        int maxThreads_{static_cast<int>(std::thread::hardware_concurrency())};
        int readBuffSize_{1024};
        int serverPort_{2323};
        int epollEventsNum_{1};
        std::string hashType_{"md5"};
        int readBufferSize_{1024};
        std::string logLevel_{"debug"};
    };
    ;

} // namespace util
