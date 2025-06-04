//
// Created by vadimzy on 6/1/25.
//

#include <csignal>
#include <iostream>
#include <sstream>
#include <thread>

#include <Poco/Util/IniFileConfiguration.h>

#include "../util/logger.h"
#include "Configuration.h"
#include "HashEchoClient.h"
#include "TcpServer.h"


COMMON_LOGGER();


namespace {

    // file scope to clean up on exit
    std::shared_ptr<TcpServer> gServer;

} // namespace



int main() {

    // setup logger
    util::log::logSink::useConsoleLog();
    util::log::logSink::setLevel("debug");

    Configuration::instance().loadFile(".tserver.cfg");

    // create server
    gServer = std::make_shared<TcpServer>(2323, std::make_unique<HashEchoClientFactory>());

    // cleaup on
    auto shutdown = [](int i) {
        std::cout << "on exit" << "\n";
        if (gServer) {
            gServer->shutdown();
        }
        exit(i);
    };

    // signal handlers
    signal(SIGINT, shutdown);
    signal(SIGABRT, shutdown);
    signal(SIGTERM, shutdown);
    signal(SIGTSTP, shutdown);
    signal(SIGKILL, shutdown);

    auto st = std::thread([&]() { gServer->run(); });
    st.join();
    return 0;
}
