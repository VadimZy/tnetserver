//
// Created by vadimzy on 6/1/25.
//

#include <csignal>
#include <iostream>
#include <thread>

#include <sys/time.h>

#include "src/client/HashEchoClient.h"
#include "src/server/TcpServer.h"
#include "util/logger.h"


COMMON_LOGGER();


namespace {
    void log_function(const char *component, const char *message, util::log::log_severity sev) {
        struct timeval tv{};
        gettimeofday(&tv, nullptr);
        char timeString[std::size("yyyy-mm-ddThh:mm:ss")];
        std::strftime(std::data(timeString), std::size(timeString), "%FT%T", std::gmtime(&tv.tv_sec));

        std::cout << timeString << "." << tv.tv_usec / 1000 << ", " << std::this_thread::get_id() << ", "
                  << util::log::log_sink::LOG_LEVELS[sev - 1] << ", " << component << message << "\n";
    }
} // namespace

// cleanup on exit
static std::shared_ptr<TcpServer> gServer;

int main() {

    util::log::log_sink::init(log_function, "");
    util::log::log_sink::set_level("debug");
    std::unique_ptr<ClientFactory> basePtr(reinterpret_cast<ClientFactory *>(new HashEchoClientFactory()));

    gServer = std::make_shared<TcpServer>(2323, std::move(basePtr));

    //
    auto lam = [](int i) {
        std::cout << "on exit" << "\n";
        if (gServer) {
            gServer->shutdown();
        }
        exit(i);
    };

    // signal handlers
    signal(SIGINT, lam);
    signal(SIGABRT, lam);
    signal(SIGTERM, lam);
    signal(SIGTSTP, lam);

    auto st = std::thread([&]() { gServer->run(); });
    st.join();
    return 0;
}
