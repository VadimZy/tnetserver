#pragma once


#include <array>
#include <atomic>
#include <cstdarg>
#include <functional>
#include <string>

namespace util::log {
    typedef enum logSeverity {
        LOG_SEV_FATAL = 1,
        LOG_SEV_CRITICAL = 2,
        LOG_SEV_ERROR = 3,
        LOG_SEV_WARNING = 4,
        LOG_SEV_NOTICE = 5,
        LOG_SEV_INFO = 6,
        LOG_SEV_DEBUG = 7,
        LOG_SEV_TRACE = 8,
    } log_severity;

    using logFn_t = std::function<void(const char *component, const char *message, log_severity severity)>;

    class logSink {
    public:
        static constexpr std::array<const char *, 8> LOG_LEVELS{"FATAL",  "CRITICAL", "ERROR", "WARNING",
                                                                "NOTICE", "INFO",     "DEBUG", "TRACE"};

        explicit logSink(const char *fn);

        static void init(logFn_t f, const std::string &component = {});

        static bool setLevel(std::string level);
        static void useConsoleLog();

        static bool isEnabled(logSeverity sev) { return logFn != nullptr && sev <= severity; }

        void log(logSeverity sev, int line, const char *fmt, ...) const __attribute__((format(printf, 4, 5)));

        void log(logSeverity sev, int line, const std::string &msg) const;

    private:
        void logv(logSeverity sev, int line, const char *fmt, va_list &args) const;

        const std::string fileName{};

        inline static logSeverity severity{LOG_SEV_INFO};
        inline static logFn_t logFn{nullptr};
        inline static std::string component;
    };

} // namespace util::log

// Make an instance of a logger in a component that will always print
// the filename and the line number. If prefix is provided then that will
// proceed the filename.
#define COMMON_LOGGER() static const util::log::logSink sLogSink(__FILE__)

#define LOG_AT_PRIO_(priority, ...)                                                                                    \
    do {                                                                                                               \
        if (sLogSink.isEnabled(priority)) {                                                                            \
            sLogSink.log((priority), __LINE__, __VA_ARGS__);                                                           \
        }                                                                                                              \
    } while (false)

// clang-format off
// Macros to use in the cpp file to interface with the component logger.
#define LOG_TRACE(...)    LOG_AT_PRIO_(util::log::log_severity::LOG_SEV_TRACE,    __VA_ARGS__)
#define LOG_DEBUG(...)    LOG_AT_PRIO_(util::log::log_severity::LOG_SEV_DEBUG,    __VA_ARGS__)
#define LOG_INFO(...)     LOG_AT_PRIO_(util::log::log_severity::LOG_SEV_INFO,     __VA_ARGS__)
#define LOG_NOTICE(...)   LOG_AT_PRIO_(util::log::log_severity::LOG_SEV_NOTICE,   __VA_ARGS__)
#define LOG_WARNING(...)  LOG_AT_PRIO_(util::log::log_severity::LOG_SEV_WARNING,  __VA_ARGS__)
#define LOG_ERROR(...)    LOG_AT_PRIO_(util::log::log_severity::LOG_SEV_ERROR,    __VA_ARGS__)
#define LOG_CRITICAL(...) LOG_AT_PRIO_(util::log::log_severity::LOG_SEV_CRITICAL, __VA_ARGS__)
#define LOG_FATAL(...)    LOG_AT_PRIO_(util::log::log_severity::LOG_SEV_FATAL,    __VA_ARGS__)
// clang-format on
