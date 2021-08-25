//
// Created by why on 2019/12/29.
//

#include <flute/Logger.h>

#include <algorithm>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iomanip>

#ifdef _WIN32
#define filename(x) strrchr(x, '\\') ? strrchr(x, '\\') + 1 : x
#else
#define filename(x) strrchr(x, '/') ? strrchr(x, '/') + 1 : x
#endif

namespace flute {

static const char* LEVEL_TAGS[] = {"[TRACE]", "[DEBUG]", "[INFO] ", "[WARN] ", "[ERROR]", "[FATAL]"};

static inline const char* getLogLevelTag(LogLevel level) {
    switch (level) {
        case LogLevel::LEVEL_TRACE:
        case LogLevel::LEVEL_DEBUG:
        case LogLevel::LEVEL_INFO:
        case LogLevel::LEVEL_WARN:
        case LogLevel::LEVEL_ERROR:
        case LogLevel::LEVEL_FATAL:
            return LEVEL_TAGS[static_cast<int>(level)];
        default:
            return LEVEL_TAGS[0];
    }
}

static void defaultCallback(const char* msg, int length) {
    fwrite(msg, 1, length, stdout);
    fflush(stdout);
}

struct Logger::LoggerImpl {
    int m_line;
    LogLevel m_logLevel;
    std::stringstream m_buffer;
    std::string m_sourceFile;
    std::string m_function;
    LoggerImpl(LogLevel level, const char* sourceFile, int line, const char* func);
};

LogLevel Logger::s_logLevel = LogLevel::LEVEL_TRACE;

LogCallback Logger::s_logCallback = defaultCallback;

Logger::LoggerImpl::LoggerImpl(LogLevel level, const char* sourceFile, int line, const char* func)
    : m_line(line), m_logLevel(level), m_buffer(), m_sourceFile(filename(sourceFile)), m_function(func) {}

Logger::Logger(LogLevel level, const char* sourceFile, int line) : m_impl(new LoggerImpl(level, sourceFile, line, "")) {
    start();
}

Logger::Logger(LogLevel level, const char* sourceFile, int line, const char* func)
    : m_impl(new LoggerImpl(level, sourceFile, line, func)) {
    start();
}

Logger::~Logger() {
    finish();
    if (s_logCallback) {
        auto temp = m_impl->m_buffer.str();
        s_logCallback(temp.c_str(), static_cast<int>(temp.length()));
    }
    delete m_impl;
}

std::stringstream& Logger::getStream() { return m_impl->m_buffer; }

void Logger::setLogLevel(LogLevel level) { s_logLevel = level; }

LogLevel Logger::getLogLevel() { return s_logLevel; }

void Logger::start() {
    auto ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    std::chrono::time_point<std::chrono::system_clock> t1(ms);
    std::time_t t = std::chrono::system_clock::to_time_t(t1);
    struct tm time {};
#ifdef _WIN32
    localtime_s(&time, &t);
#else
    localtime_r(&t, &time);
#endif
    auto temp = std::put_time(&time, "%Y-%m-%d %H:%M:%S");
    getStream() << temp << "," << std::setw(3) << std::setfill('0') << ms.count() % 1000 << " "
                << getLogLevelTag(m_impl->m_logLevel) << " - ";
}

void Logger::finish() {
    if (m_impl->m_function.length() > 0) {
        getStream() << " - " << m_impl->m_sourceFile << ":" << m_impl->m_line << " function(" << m_impl->m_function
                    << ")" << std::endl;
    } else {
        getStream() << " - " << m_impl->m_sourceFile << ":" << m_impl->m_line << std::endl;
    }
}

void Logger::setLogCallback(LogCallback&& callback) { s_logCallback = std::move(callback); }

void Logger::setLogCallback(const LogCallback& callback) { s_logCallback = callback; }

} // namespace flute