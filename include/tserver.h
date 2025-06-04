#pragma once
#include <functional>
#include <string>
#include <memory>

// digest generator interface
class HashDigest {
public:
    virtual ~HashDigest() = default;
    virtual void update(std::string_view buf) = 0;
    virtual std::string to_hex_string() = 0;
    virtual void reset() = 0;
};

// streaming digest interface
class StreamDigest {
public:
    virtual ~StreamDigest() = default;
    virtual void append(std::function<int(std::string)>, std::string_view buf) = 0;
    virtual void reset() = 0;
};

// connection client interface
class ConnClient {
public:
    enum class State {
        CREATED,
        RUNNING,
        COMPLETED,
        FAILED,
        DESTROYED,
    };

    virtual ~ConnClient() = default;
    virtual int start() = 0;
    virtual void stop() = 0;
};

//
class ConnMonitor {
public:
    virtual ~ConnMonitor() = default;
    virtual void clientCreated(int fd) = 0;
    virtual void clientStatusChanged(int fd, ConnClient::State st, ConnClient::State old) = 0;
    virtual void clientDeleted(int fd) = 0;
    virtual void clientError(int fd, int errNo) = 0;
    virtual void shutdown() = 0;
};

//
class ClientFactory {
public:
    virtual ~ClientFactory() = default;
    virtual ConnClient *create(int fd, std::shared_ptr<ConnMonitor> m) = 0;
};
