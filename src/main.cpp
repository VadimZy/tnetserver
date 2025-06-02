//
// Created by vadimzy on 6/1/25.
//

#include <csignal>
#include <iostream>
#include <sstream>
#include <thread>

#include <sys/time.h>

#include "../util/logger.h"
#include "HashEchoClient.h"
#include "TcpServer.h"


COMMON_LOGGER();


namespace {

    // logger function
    void log_function(const char *component, const char *message, util::log::log_severity sev) {
        struct timeval tv{};
        gettimeofday(&tv, nullptr);
        char ts[std::size("yyyy-mm-ddThh:mm:ss")];
        std::strftime(std::data(ts), std::size(ts), "%FT%T", std::gmtime(&tv.tv_sec));
        std::stringstream ss;
        ss << ts << "." << tv.tv_usec / 1000 << ", " << std::this_thread::get_id() << ", "
           << util::log::log_sink::LOG_LEVELS[sev - 1] << ", " << component << message << "\n";
        std::cout << ss.str();
    }

    // file scope to clean up on exit
    std::shared_ptr<TcpServer> gServer;

} // namespace


int main() {

    // setup logger
    util::log::log_sink::init(log_function, "");
    util::log::log_sink::set_level("debug");

    // create server
    gServer = std::make_shared<TcpServer>(2323, std::make_unique<HashEchoClientFactory>());

    //
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

    auto st = std::thread([&]() { gServer->run(); });
    st.join();
    return 0;
}
