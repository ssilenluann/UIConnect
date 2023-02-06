#ifndef LOG_LOGFILEOUTPUTTER_H
#define LOG_LOGFILEOUTPUTTER_H

#include <string>
#include <memory>
#include <fstream>

#include "LogLevel.h"
#include "LogOutputter.h"
class LogFileOutputter : public LogOutputter
{
public:
    typedef std::shared_ptr<LogFileOutputter> ptr;
    LogFileOutputter(const std::string& fileName);
    void log(LogLevel::Level level, std::shared_ptr<LogItem> item) override;
    bool reopen();
private:
    std::string m_fileName;
    std::ofstream m_fileStream;
};
#endif