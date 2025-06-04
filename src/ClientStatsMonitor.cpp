//
// Created by vadimzy on 6/3/25.
//

#include "ClientStatsMonitor.h"
#include "../util/logger.h"

COMMON_LOGGER();

void ClientStatsMonitor::clientCreated(int fd) {
    if (closed) {
        return;
    }
    ++created;
    LOG_INFO("Conn stats: new client; created: %d, deleted: %d, running: %d, completed: %d,  failed: %d, errors: %d",
             created.load(), deleted.load(), running.load(), completed.load(), failed.load(), errors.load());
}
void ClientStatsMonitor::clientStatusChanged(int fd, ConnClient::State st, ConnClient::State old) {
    if (closed) {
        return;
    }

    switch (st) {
        case ConnClient::State::RUNNING:
            ++running;
            break;
        case ConnClient::State::COMPLETED:
            --running;
            ++completed;
            break;
        case ConnClient::State::FAILED:
            if (old == ConnClient::State::RUNNING) {
                --running;
            }
            ++failed;
            break;
        default:
            break;
    }
}
void ClientStatsMonitor::clientDeleted(int fd) {
    if (closed) {
        return;
    }
    ++deleted;
    LOG_INFO("Conn stats: client destroyed; created: %d, deleted: %d, running: %d, completed: %d,  failed: %d, "
             "errors: %d",
             created.load(), deleted.load(), running.load(), completed.load(), failed.load(), errors.load());
}

void ClientStatsMonitor::clientError(int fd, int errNo) {
    // LOG_INFO("Client fd: %d, reported error: %s ", fd, strerror(errNo));
    if (closed) {
        return;
    }
    ++errors;
}

void ClientStatsMonitor::shutdown() {
    if (closed) {
        return;
    }

    LOG_INFO("Conn stats: client destroyed; created: %d, deleted: %d, running: %d, completed: %d,  failed: %d, "
         "errors: %d",
         created.load(), deleted.load(), running.load(), completed.load(), failed.load(), errors.load());
    closed.store(true);
}
