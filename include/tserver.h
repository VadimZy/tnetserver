#pragma once
#include <string>

class HashDigest {
public:
    virtual void update(void *buff, size_t len) = 0;

    virtual std::string to_hex_string() = 0;

    virtual ~HashDigest() = default;
};

class ConnClient {
public:
    virtual ~ConnClient() = default;

    virtual int start() = 0;

    virtual void stop() = 0;
};

class ConnManager {
public:
    virtual ~ConnManager() = default;

    virtual void clientDisconnected(int fd) = 0;
};

class ClientFactory {
public:
    virtual ~ClientFactory() = default;
    virtual ConnClient* create(int fd, ConnManager &m) = 0;
};

