//
// Created by vadimzy on 6/1/25.
//

#include "TcpServer.h"

#include <cstring>
#include <iostream>
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

#include "../../util/logger.h"

#define PORT 2323
#define MAX_EVENTS 1
#define THREAD_COUNT 4

COMMON_LOGGER();

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

TcpServer::TcpServer(int port, std::unique_ptr<ClientFactory> cf) : clientFactory(std::move(cf)) {
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
}

TcpServer::~TcpServer() { LOG_INFO("in Server destructor"); }

void TcpServer::cleanup() {
    LOG_DEBUG("clients cleanup job");
    for (auto iter = connDone.begin(); iter != connDone.end();) {
        if ((*iter)->state() != ConnClient::State::RUNNING) {
            LOG_DEBUG("removing client");
            iter = connDone.erase(iter);
        } else {
            ++iter;
        }
    }
}

int TcpServer::run() {
    if (server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0); server_fd < 0) {
        return -1;
    }

    if (bind(server_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
        return -1;
    }

    if (listen(server_fd, SOMAXCONN) < 0) {
        return -1;
    }

    if (epoll_fd = epoll_create1(O_CLOEXEC); epoll_fd < 0) {
        return -1;
    }

    epoll_event event{.events = EPOLLIN, .data = {.fd = server_fd}};

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) < 0) {
        return -1;
    }

    std::vector<epoll_event> events(MAX_EVENTS);
    LOG_INFO("Multithreaded HashEcho server running on port %d", PORT);

    while (!stop.load()) {
        int n = epoll_wait(epoll_fd, events.data(), MAX_EVENTS, -1);

        if (n < 0) {
            stop.store(true);
            break;
        }

        if (n == 0) {
            continue;
        }

        for (int i = 0; i < n; ++i) {
            const auto cl_fd = events[i].data.fd;

            if (cl_fd == server_fd) {
                LOG_DEBUG("fd: %d, epoll (connect) events: %s", cl_fd, epoll_names(events[i].events).c_str());
                sockaddr_in client_addr{};
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(server_fd, reinterpret_cast<sockaddr *>(&client_addr), &client_len);

                if (client_fd != -1) {
                    LOG_INFO("new client on port %d", PORT);
                    epoll_event client_event{};
                    client_event.events = EPOLLIN | EPOLLET | EPOLLHUP | EPOLLRDHUP | EPOLLERR;
                    client_event.data.fd = client_fd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event);
                } else {
                    LOG_ERROR("bad client_id:  %d", client_fd);
                }

            } else if (cl_fd > 0) {

                LOG_DEBUG("fd: %d, epoll events: %s", cl_fd, epoll_names(events[i].events).c_str());

                auto createClient = [&]() {                    LOG_INFO("creating new client on fd %d", cl_fd);
                    auto client = clientFactory->create(cl_fd, *this);

                    if (client->start() == 0) {
                        connMap.emplace(cl_fd, std::shared_ptr<ConnClient>(clientFactory->create(cl_fd, *this)));
                    } else {
                        LOG_ERROR("new client on fd %d failed to start, deleting", cl_fd);
                    }
                };

                if (auto it = connMap.find(cl_fd); it != connMap.end()) {

                    if (it->second->state() != ConnClient::State::RUNNING) {
                        connDone.emplace_back(std::move(it->second));
                        connMap.erase(it);
                        createClient();
                    }
                    else if (events[i].events & (EPOLLHUP | EPOLLRDHUP | EPOLLERR)) {
                        LOG_INFO("terminating client on fd %d", cl_fd);
                        it->second->stop();
                    } else {
                        it->second->onData();
                    }

                } else {
                    createClient();
                }
            } else {
                LOG_ERROR("event on invalid fd:  %d", cl_fd);
            }
        }
    }
    close(server_fd);
    return 0;
}

void TcpServer::shutdown() {
    stop.store(true);
    LOG_INFO("waiting server shutdown");
}
