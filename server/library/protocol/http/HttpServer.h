#ifndef PROTOCOL_HTTP_SERVER_H
#define PROTOCOL_HTTP_SERVER_H

#include "../network/TcpServer.h"

class HttpServer: public TcpServer
{
public:
    HttpServer(int threadCount = 4);
    void onConnect(); 
};
#endif