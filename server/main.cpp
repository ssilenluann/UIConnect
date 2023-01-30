#include <iostream>
#include "./network/TcpServer.h"

int main()
{
    std::cout << "test project" << std::endl;
    TcpServer server;
    server.init("127.0.0.1", 5260);
    return 0;
}