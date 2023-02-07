#ifndef LOG_LOGSTDOUTOUTPUTTER_H
#define LOG_LOGSTDOUTOUTPUTTER_H

#include "../LogOutputter.h"

// output log to standare io device
class LogStdoutOutputter: public LogOutputter
{
	typedef std::shared_ptr<LogStdoutOutputter> ptr;
public:
    LogStdoutOutputter(std::string pattern = "");
    void log(Logger::ptr logger, LogLevel::Level level, LogItem::ptr item) override;
    std::string toYamlString() override;
private:
};
#endif