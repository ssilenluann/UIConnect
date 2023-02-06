#ifndef LOG_LOGSTDOUTOUTPUTTER_H
#define LOG_LOGSTDOUTOUTPUTTER_H

#include "LogOutputter.h"

class LogStdoutOutputter: public LogOutputter
{
public:
    void log(LogLevel::Level level, std::shared_ptr<LogItem> event) override;
private:
};
#endif