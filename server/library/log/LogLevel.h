#ifndef LOG_LOGLEVEL_H
#define LOG_LOGLEVEL_H

#include <string>

class LogLevel {
public:
    enum Level {
        UNKNOWN = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };

    /**
     * @brief transfer log level to string
     * @param[in] level: log level
     */
    static const char* ToString(LogLevel::Level level);
    
    /**
     * @brief transfer string to log level
     * @param[in] str log level
     */
    static LogLevel::Level FromString(const std::string& str);
};

#endif