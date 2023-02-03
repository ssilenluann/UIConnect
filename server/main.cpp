#include <iostream>
#include "./network/TcpServer.h"

int main()
{
    std::cout << "test project" << std::endl;
    TcpServer server;
    server.init("192.168.134.133", 5260);
	server.run();
    return 0;
}
