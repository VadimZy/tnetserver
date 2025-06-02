//
// Created by vadimzy on 6/1/25.
//

#pragma once
#include <string_view>
#include "../../include/tserver.h"
#include "Poco/MD5Engine.h"

class MD5Digest : public HashDigest {
public:
    MD5Digest() = default;

    void update(std::string_view buf) override { engine.update(buf.data(), buf.size()); }

    std::string to_hex_string() override { return Poco::DigestEngine::digestToHex(engine.digest()); }

    void reset() override { engine.reset(); };

    ~MD5Digest() override = default;

private:
    Poco::MD5Engine engine;
};


class StreamMD5Digest : public StreamDigest {
public:
    explicit StreamMD5Digest(char sep) : separator(sep) {}
    ~StreamMD5Digest() override = default;

    void append(std::function<int(std::string)>, std::string_view buf) override ;
    void reset() override {digest.reset();};

private:
    const char separator;
    MD5Digest digest;
};
