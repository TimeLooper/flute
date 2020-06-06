//
// Created by why on 2019/12/29.
//

#ifndef FLUTE_LOGGER_H
#define FLUTE_LOGGER_H

#include <flute/config.h>
#include <flute/copyable.h>

#include <functional>
#include <sstream>

namespace flute {

enum class LogLevel { LEVEL_TRACE, LEVEL_DEBUG, LEVEL_INFO, LEVEL_WARN, LEVEL_ERROR, LEVEL_FATAL };

typedef std::function<void(const char*, int)> LogCallback;

class Logger : private copyable {
public:
    FLUTE_API_DECL Logger(LogLevel logLevel, const char* sourceFile, int line);
    FLUTE_API_DECL Logger(LogLevel logLevel, const char* sourceFile, int line, const char* func);
    FLUTE_API_DECL ~Logger();

    FLUTE_API_DECL std::stringstream& getStream();
    FLUTE_API_DECL static void setLogLevel(LogLevel level);
    FLUTE_API_DECL static LogLevel getLogLevel();
    FLUTE_API_DECL static void setLogCallback(LogCallback&& callback);
    FLUTE_API_DECL static void setLogCallback(const LogCallback& callback);

private:
    struct LoggerImpl;
    LoggerImpl* m_impl;

    static LogLevel s_logLevel;
    static LogCallback s_logCallback;

    void start();
    void finish();
};

#define LOG_TRACE                                                     \
    if (flute::Logger::getLogLevel() <= flute::LogLevel::LEVEL_TRACE) \
    flute::Logger(flute::LogLevel::LEVEL_TRACE, __FILE__, __LINE__).getStream()
#define LOG_DEBUG                                                     \
    if (flute::Logger::getLogLevel() <= flute::LogLevel::LEVEL_DEBUG) \
    flute::Logger(flute::LogLevel::LEVEL_DEBUG, __FILE__, __LINE__).getStream()
#define LOG_INFO flute::Logger(flute::LogLevel::LEVEL_INFO, __FILE__, __LINE__).getStream()
#define LOG_WARN flute::Logger(flute::LogLevel::LEVEL_WARN, __FILE__, __LINE__).getStream()
#define LOG_ERROR flute::Logger(flute::LogLevel::LEVEL_ERROR, __FILE__, __LINE__).getStream()
#define LOG_FATAL flute::Logger(flute::LogLevel::LEVEL_FATAL, __FILE__, __LINE__).getStream()

#define CHECK_NOTNULL(val) flute::checkNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

template <typename T>
inline T* checkNotNull(const char* file, int line, const char* msg, T* ptr) {
    if (ptr == nullptr) {
        LOG_FATAL << msg;
    }
    return ptr;
}

} // namespace flute

#endif // FLUTE_LOGGER_H
