#ifndef LOG_LOGSTDOUTOUTPUTTER_H
#define LOG_LOGSTDOUTOUTPUTTER_H

#include "../LogOutputter.h"

// output log to standare io device
class LogStdoutOutputter: public LogOutputter
{
	typedef std::shared_ptr<StdoutLogOutputter> ptr;
public:
    void log(LogLevel::Level level, std::shared_ptr<LogItem> item) override;
private:
};
#endif