#ifndef PROTOCOL_HTTP_SERVER_H
#define PROTOCOL_HTTP_SERVER_H

#include "../network/TcpServer.h"

class HttpServer: public TcpServer
{
public:
    HttpServer(int threadCount = 4);
    virtual void onConnect() override; 
    virtual bool init(std::string ip, int port) override; 
    
};
#endif