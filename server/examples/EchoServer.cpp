#include "network/TcpServer.h"
#include "reactor/EchoProcessor.h"

int main(int argc, char** argv)
{
    ProcessorProxy::Instance().setProcessor(std::shared_ptr<Processor>(new EchoProcessor()));

    std::shared_ptr<TcpServer> server(new TcpServer());
    server->init("127.0.0.1", 30030);
    server->run();

    return 0;
}