#include <iostream>
#include "./network/socket/TcpSocket.h"

int main()
{
    std::cout << "test project" << std::endl;
    TcpSocket sock;
    sock.bind("127.0.0.1", 5260);
    return 0;
}