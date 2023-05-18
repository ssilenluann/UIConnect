#include "network/TcpServer.h"
#include "log/Logger.h"

// static Logger::ptr g_logger = LOG_ROOT();

int main(int argc, char** argv)
{
    TcpServer::ptr server(new TcpServer);
    server->init("127.0.0.1", 5261);
    server->run();


}