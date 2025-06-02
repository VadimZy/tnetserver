#pragma once
#include <functional>
#include <string>

class HashDigest {
public:
    virtual ~HashDigest() = default;
    virtual void update(std::string_view buf) = 0;
    virtual std::string to_hex_string() = 0;
    virtual void reset() = 0;
};

class StreamDigest {
public:
    virtual ~StreamDigest() = default;
    virtual void append(std::function<int(std::string)>, std::string_view buf) = 0;
    virtual void reset() = 0;
};

class ConnClient {
public:
    enum class State {
        CREATED,
        RUNNING,
        COMPLETED,
        FAILED,
    };

    virtual ~ConnClient() = default;
    virtual int start() = 0;
    virtual void stop() = 0;
    virtual void onData() = 0;
    virtual State state() const = 0;
};

class ConnManager {
public:
    virtual ~ConnManager() = default;

    virtual void clientDisconnected(int fd) = 0;
};

class ClientFactory {
public:
    virtual ~ClientFactory() = default;
    virtual ConnClient *create(int fd, ConnManager &m) = 0;
};
