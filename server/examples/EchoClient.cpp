#include "network/event/EventLoop.h"
#include "network/socket/TcpSocket.h"
#include "network/TcpConnection.h"
#include "network/TcpSession.h"
#include "reactor/EchoProcessor.h"

#define CLIENT_COUNT 2000

int main()
{
    ProcessorProxy::Instance().setProcessor(std::shared_ptr<Processor>(new EchoProcessor()));

    std::shared_ptr<EventLoop> loop(new EventLoop());
    
    std::vector<std::shared_ptr<TcpSocket>> clients;
    clients.resize(CLIENT_COUNT);
    
    for(int i = 0; i < clients.capacity(); i++)
    {
        clients[i].reset(new TcpSocket());
        clients[i]->connect("127.0.0.1", 30030);

        std::unique_ptr<TcpConnection> connection(new TcpConnection(clients[i]->fd(), loop));
        std::shared_ptr<TcpSession> session(new TcpSession(i, std::move(connection), loop));
        loop->addSession(session);

        loop->addTask(
        [&]()
        {
            std::string str = "test echo";
            Packet pack(1, str.c_str(), str.size());
            session->send(pack);
        }
        );
    }

    loop->loop();
    return 0;
}