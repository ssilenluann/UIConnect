#include <iostream>
#include "./network/TcpServer.h"
#include "./log/Logger.h"
#include "./log/outputter/LogStdoutOutputter.h"
#include "./log/outputter/LogFileOutputter.h"

int main()
{
    Logger::ptr logger(new Logger);
    logger->addOutputter(std::make_shared<LogStdoutOutputter>());

    LogFileOutputter::ptr fileOutputter(new LogFileOutputter("./server_log.log"));
    fileOutputter->setLevel(LogLevel::Level::ERROR);
    logger->addOutputter(fileOutputter);

    std::cout << "test project" << std::endl;
    TcpServer server;
    server.init("192.168.134.133", 5260);
	server.run();
    return 0;
}
