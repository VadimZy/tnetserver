//
// Created by vadimzy on 6/1/25.
//
#include "DigestGenerators.h"

#include "../util/util.h"


void StreamMD5Digest::append(std::function<int(std::string)> acceptor, std::string_view buf) {

    while (true) {
        util::BuffSplitter spl(buf);
        digest.update(spl.left());
        if (!spl.has_split()) {
            break;
        }
        acceptor(digest.to_hex_string());
        digest.reset();
        buf = spl.right();
    }
}
