#include "network/TcpServer.h"
#include "log/Logger.h"

static Logger::ptr g_logger = LOG_ROOT();

class EchoProcessor: public Processor
{
public:
    void handleMsg(Packet& msg, std::shared_ptr<TcpSession> session)
    {
        LOG_DEBUG(g_logger) << msg.dataLoad();
        session->send(msg);
    }
};

int main(int argc, char** argv)
{
    ProcessorProxy::Instance().setProcessor(std::make_shared<EchoProcessor>());

    TcpServer::ptr server(new TcpServer(1));
    server->run("127.0.0.1:30030");
}