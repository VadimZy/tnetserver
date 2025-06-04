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
    void log_sink::logv(log_severity sev, const int line_no, const char *fmt, va_list &args) const {
        auto try_print = [&](auto &buffer, size_t buff_size) {
            va_list mutable_args;
            va_copy(mutable_args, args);

            std::string::size_type pfix_len = 0;

            if (line_no) {
                // Try to write the prefix to log_buffer
                pfix_len = std::snprintf(&buffer[0], buff_size, "%s: ", mk_prefix(file_name, line_no).c_str());
            }

            // if we don't have room use nullptr to calculate required buff size
            auto msg_len = std::vsnprintf(pfix_len < buff_size ? &buffer[pfix_len] : nullptr, buff_size - pfix_len, fmt,
                                          mutable_args);

            va_end(mutable_args);

            // One extra byte needed for the NUL terminator
            return pfix_len + msg_len + 1;
        };

        // try to fill fixed len array on the stack
        std::array<char, DEFAULT_LOG_STR_LENGTH> arr_buffer{};
        auto msg_len = try_print(arr_buffer, DEFAULT_LOG_STR_LENGTH);

        if (msg_len <= DEFAULT_LOG_STR_LENGTH) {
            log_fn(component.c_str(), arr_buffer.data(), sev);
        } else {
            // go for the heap
            std::vector<char> log_buffer(msg_len);
            try_print(log_buffer, msg_len);
            log_fn(component.c_str(), log_buffer.data(), sev);
        }
    }

    void log_sink::init(log_fn_t f, const std::string &comp) {
        log_fn = std::move(f);
        component = comp;
    }

    void log_sink::log(log_severity sev, int line, const char *fmt, ...) const {
        if (!is_enabled(sev)) {
            return;
        }

        va_list args;
        va_start(args, fmt);
        logv(sev, line, fmt, args);
        va_end(args);
    }

    bool log_sink::set_level(std::string level) {
        std::transform(level.begin(), level.end(), level.begin(), ::toupper);
        for (int i = 0; i < sizeof(LOG_LEVELS); ++i) {
            if (level == LOG_LEVELS[i]) {
                severity = static_cast<log_severity>(i + 1);
                return true;
            }
        }
        return false;
    }

    // default log sink
    void log_sink::use_console_log() {
        log_fn = []
                // logger function
                (const char *component, const char *message, util::log::log_severity sev) {
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

    void log_sink::log(log_severity sev, int line, const std::string &msg) const {
        if (!is_enabled(sev)) {
            return;
        }
        std::string out{};
        out.reserve(msg.size() + 128);
        out += mk_prefix(file_name, line);
        out += ": ";
        out += msg;
        log_fn(component.c_str(), out.c_str(), sev);
    }

    log_sink::log_sink(const char *fn) : file_name(std::filesystem::path(fn).filename()) {}
} // namespace util::log
