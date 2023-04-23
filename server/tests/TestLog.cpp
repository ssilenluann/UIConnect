
#include <memory>

#include "log/LoggerManager.h"
#include "log/Logger.h"
#include "log/outputter/LogFileOutputter.h"
#include "log/outputter/LogStdoutOutputter.h"

int main()
{
    Logger::ptr logger(new Logger);
    logger->addOutputter(std::make_shared<LogStdoutOutputter>());

    LogFileOutputter::ptr fileOutputter(new LogFileOutputter("./log.txt", "%d%T%m%n"));
    fileOutputter->setLevel(LogLevel::Level::ERROR);
    logger->addOutputter(fileOutputter);

    LOG_INFO(logger) << "test macro";
    LOG_ERROR(logger) << "test macro error";

    LOG_FMT_ERROR(logger, "test macro fmt error %s", "aa");
    
    return 0;
}