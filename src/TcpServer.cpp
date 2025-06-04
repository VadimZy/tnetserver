//
// Created by vadimzy on 6/1/25.
//

#include "TcpServer.h"

#include <csignal>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <utility>
#include <vector>

#include <cstring>
#include <fcntl.h>
#include <mutex>
#include <sys/epoll.h>
#include <unordered_set>

#include "../util/logger.h"
#include "ClientStatsMonitor.h"
#include "Configuration.h"
#include "Poco/TaskManager.h"

COMMON_LOGGER();

// log helper function
std::string epoll_names(int evt) {
    static std::vector<unsigned> epoll_events{
            EPOLLIN,  EPOLLPRI, EPOLLOUT,   EPOLLRDNORM,    EPOLLRDBAND, EPOLLWRNORM,  EPOLLWRBAND, EPOLLMSG,
            EPOLLERR, EPOLLHUP, EPOLLRDHUP, EPOLLEXCLUSIVE, EPOLLWAKEUP, EPOLLONESHOT, EPOLLET,
    };

    static std::vector<std::string> epoll_events_str{
            "EPOLLIN",     "EPOLLPRI",       "EPOLLOUT",    "EPOLLRDNORM",  "EPOLLRDBAND",
            "EPOLLWRNORM", "EPOLLWRBAND",    "EPOLLMSG",    "EPOLLERR",     "EPOLLHUP",
            "EPOLLRDHUP",  "EPOLLEXCLUSIVE", "EPOLLWAKEUP", "EPOLLONESHOT", "EPOLLET",
    };

    std::string ret = "[";
    for (int i = 0; i < epoll_events.size(); ++i) {

        if (epoll_events[i] & evt) {
            if (ret.size() > 1) {
                ret += ", ";
            }
            ret += epoll_events_str[i];
        }
    }
    return ret += "]";
}

// ctor
TcpServer::TcpServer(int port, std::unique_ptr<ClientFactory> cf) : clientFactory(std::move(cf)) {
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
}


// dtor
TcpServer::~TcpServer() { LOG_INFO("in Server destructor"); }

// ConnClient wrapper to satisfy Poco::Task
struct clientWrap : public Poco::Task {
    explicit clientWrap(ConnClient *c) : Task("conn"), client(c) {}
    ~clientWrap() override = default;
    void runTask() override { client->start(); };
    std::unique_ptr<ConnClient> client;
};

// epoll loop
int TcpServer::run() {

    struct exitGuard {
        explicit exitGuard(std::atomic<int> &stop) : serverFd(stop) {}
        ~exitGuard() { serverFd = 2; }
        std::atomic<int> &serverFd;
    };
    exitGuard eg{stop};

    auto capacity = std::max(2, Configuration::instance().maxThreads());
    auto port = Configuration::instance().serverPort();
    auto eventsNum = Configuration::instance().epollEventsNum();
    taskManager = std::make_unique<Poco::TaskManager>("connections", 2, capacity);
    auto connMonitor = std::make_shared<ClientStatsMonitor>();

    auto logErrorReturn = [](const char *msg) {
        std::string err(strerror(errno));
        LOG_ERROR("%s error: %s", msg, err.c_str());
        return -1;
    };

    if (server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0); server_fd < 0) {
        return logErrorReturn("create server socket");
    }

    if (bind(server_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
        return logErrorReturn("server socket bind");
    }

    if (listen(server_fd, SOMAXCONN) < 0) {
        return logErrorReturn("server socket listen");
    }

    if (epoll_fd = epoll_create1(0); epoll_fd < 0) {
        return logErrorReturn("server epoll_create1");
    }

    epoll_event event{.events = EPOLLIN, .data = {.fd = server_fd}};

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) < 0) {
        return logErrorReturn("server epoll_control");
    }

    std::vector<epoll_event> events(eventsNum);
    LOG_INFO("starting HashEcho server on port: %d, maxThreads: %d, epollEventsNum: %d", port, capacity, eventsNum);

    while (!stop.load()) {
        int n = epoll_wait(epoll_fd, events.data(), eventsNum, 100);

        if (n == 0) {
            continue;
        }

        if (n < 0) {
            stop.store(true);
            LOG_ERROR("epoll wait error");
            break;
        }

        for (int i = 0; i < n; ++i) {
            const auto cl_fd = events[i].data.fd;

            if (cl_fd == server_fd) {
                LOG_DEBUG("fd: %d, epoll (connect) events: %s", cl_fd, epoll_names(events[i].events).c_str());
                sockaddr_in client_addr{};
                socklen_t client_len = sizeof(client_addr);
                int new_fd = accept(server_fd, reinterpret_cast<sockaddr *>(&client_addr), &client_len);

                if (new_fd != -1) {
                    LOG_INFO("new client on port %d", port);
                    epoll_event client_event{};
                    client_event.events = EPOLLIN | EPOLLET | EPOLLHUP | EPOLLRDHUP | EPOLLERR;
                    client_event.data.fd = new_fd;

                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_fd, &client_event) < 0) {
                        LOG_ERROR("fd: %d, EPOLL_CTL_ADD error: %s", new_fd, strerror(errno));
                        close(new_fd);
                    } else {
                        LOG_DEBUG("fd: %d, creating new client, num active tasks: %d", new_fd, taskManager->count());
                        try {
                            taskManager->start(new clientWrap(clientFactory->create(new_fd, connMonitor)));
                        } catch (const std::exception &e) {
                            LOG_ERROR("taskManager reached capacity: %s, terminating", e.what());
                            break;
                        }
                    }
                } else {
                    LOG_ERROR("bad new fd:  %d", new_fd);
                }

            } else if (cl_fd > 0) {
                LOG_TRACE("fd: %d, epoll events: %s", cl_fd, epoll_names(events[i].events).c_str());

            } else {
                LOG_ERROR("event on invalid fd:  %d", cl_fd);
            }
        }
    }
    close(server_fd);
    close(epoll_fd);
    taskManager->cancelAll();
    taskManager->joinAll();
    taskManager.reset();
    return 0;
}

// stop server and cleanup
void TcpServer::shutdown() {
    LOG_INFO("server shutdown");
    stop.store(1);
    using namespace std::chrono_literals;
    int i{0};
    while (stop.load() != 2) {
        if (i++ % 10 == 0) {
            LOG_INFO("waiting for shutdown");
        }
        std::this_thread::sleep_for(100ms);
    }
    LOG_INFO("shutdown complete");
}
