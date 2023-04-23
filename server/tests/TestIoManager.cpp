#include "coroutine/IOManager.h"
#include "log/Logger.h"
#include "network/socket/TcpSocket.h"
#include <errno.h>

static Logger::ptr g_logger = LOG_ROOT();
TcpSocket sock;
void testIO()
{
    
    int ret = sock.bind("127.0.0.1", 30040);
    sock.listen();

    IOManager::GetThis()->addEvent(sock.fd(), IOManager::EventType::READ, []()
        {
            LOG_INFO(g_logger) << "read callback";
        }
    );
    
    IOManager::GetThis()->addEvent(sock.fd(), IOManager::EventType::WRITE, []()
        {
            LOG_INFO(g_logger) << "write callback";
        }
    );
}

void testTimer()
{
    IOManager::GetThis()->addTimer(
        2000, [](){LOG_INFO(g_logger) << "test timer one short";}
    );

    IOManager::GetThis()->addTimer(
        4000, [](){LOG_INFO(g_logger) << "test timer 4000 ms";}, false
    );

    IOManager::GetThis()->addTimer(
        8000, [](){LOG_INFO(g_logger) << "test timer 8000 ms";}, false
    );
}

int main()
{
    std::shared_ptr<IOManager> ioManager(new IOManager(5));
    ioManager->start(ioManager);
    testIO();
    testTimer();
    ioManager->stop();
    return 0;
}