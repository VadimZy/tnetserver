//
// Created by vadimzy on 6/1/25.
//
#pragma once
#include <string_view>

#include "Poco/Util/IniFileConfiguration.h"

namespace util {
    class BuffSplitter {
    public:
        template<unsigned char D = '\n'>
        explicit BuffSplitter(std::string_view sv) {
            if (auto pos = sv.find(D); pos != std::string_view::npos) {
                svl = sv.substr(0, pos);
                svr = sv.substr(pos + 1);
                split = true;
            }else {
                svl = sv;
            }
        }

        bool has_split() const {
            return split;
        }

        std::string_view left() const {
            return svl;
        }

        std::string_view right() const {
            return svr;
        }

    private:
        bool split{false};
        std::string_view svl{};
        std::string_view svr{};
    };

} // namespace util
