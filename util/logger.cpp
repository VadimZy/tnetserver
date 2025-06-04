//
// Created by vadimzy on 6/1/25.
//

#include "logger.h"


#include <filesystem>
#include <iostream>
#include <memory>
#include <sys/time.h>
#include <thread>
#include <utility>
#include <vector>

namespace util::log {

    // default buffer size
    // should fit most log strings without resizing
    constexpr size_t DEFAULT_LOG_STR_LENGTH = 512;

    inline std::string mk_prefix(const std::string &file_name, const int line_no) {
        return std::string{}.append(file_name).append(":").append(std::to_string(line_no));
    }

    // log string generating
    void logSink::logv(logSeverity sev, const int line_no, const char *fmt, va_list &args) const {
        auto tryPrint = [&](auto &buffer, size_t buffSize) {
            va_list mutableArgs;
            va_copy(mutableArgs, args);

            std::string::size_type pfix_len = 0;

            if (line_no) {
                // Try to write the prefix to log_buffer
                pfix_len = std::snprintf(&buffer[0], buffSize, "%s: ", mk_prefix(fileName, line_no).c_str());
            }

            // if we don't have room use nullptr to calculate required buff size
            auto msg_len = std::vsnprintf(pfix_len < buffSize ? &buffer[pfix_len] : nullptr, buffSize - pfix_len, fmt,
                                          mutableArgs);

            va_end(mutableArgs);

            // One extra byte needed for the NUL terminator
            return pfix_len + msg_len + 1;
        };

        // try to fill fixed len array on the stack
        std::array<char, DEFAULT_LOG_STR_LENGTH> arrBuffer{};
        auto msg_len = tryPrint(arrBuffer, DEFAULT_LOG_STR_LENGTH);

        if (msg_len <= DEFAULT_LOG_STR_LENGTH) {
            logFn(component.c_str(), arrBuffer.data(), sev);
        } else {
            // go for the heap
            std::vector<char> logBuffer(msg_len);
            tryPrint(logBuffer, msg_len);
            logFn(component.c_str(), logBuffer.data(), sev);
        }
    }

    void logSink::init(logFn_t f, const std::string &comp) {
        logFn = std::move(f);
        component = comp;
    }

    void logSink::log(logSeverity sev, int line, const char *fmt, ...) const {
        if (!isEnabled(sev)) {
            return;
        }

        va_list args;
        va_start(args, fmt);
        logv(sev, line, fmt, args);
        va_end(args);
    }

    bool logSink::setLevel(std::string level) {
        std::transform(level.begin(), level.end(), level.begin(), ::toupper);
        for (int i = 0; i < sizeof(LOG_LEVELS); ++i) {
            if (level == LOG_LEVELS[i]) {
                severity = static_cast<logSeverity>(i + 1);
                return true;
            }
        }
        return false;
    }

    // default log sink
    void logSink::useConsoleLog() {
        logFn = []
                // logger function
                (const char *component, const char *message, util::log::logSeverity sev) {
                    struct timeval tv{};
                    gettimeofday(&tv, nullptr);
                    char ts[std::size("yyyy-mm-ddThh:mm:ss")];
                    std::strftime(std::data(ts), std::size(ts), "%FT%T", std::gmtime(&tv.tv_sec));
                    std::stringstream ss;
                    ss << ts << "." << std::setfill('0') << std::setw(3) << tv.tv_usec / 1000 << ", "
                       << std::this_thread::get_id() << ", " << LOG_LEVELS[sev - 1] << ", "
                       << component << message << "\n";
                    std::cout << ss.str();
                };
    }

    void logSink::log(logSeverity sev, int line, const std::string &msg) const {
        if (!isEnabled(sev)) {
            return;
        }
        std::string out{};
        out.reserve(msg.size() + 128);
        out += mk_prefix(fileName, line);
        out += ": ";
        out += msg;
        logFn(component.c_str(), out.c_str(), sev);
    }

    logSink::logSink(const char *fn) : fileName(std::filesystem::path(fn).filename()) {}
} // namespace util::log
