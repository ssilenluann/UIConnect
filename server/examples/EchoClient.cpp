#include "network/socket/TcpSocket.h"
#include "network/socket/Packet.h"
#include "network/TcpConnection.h"
#include "network/TcpSession.h"
#include "network/epoll/EpollChannel.h"
#include "network/epoll/EpollWorker.h"
#include "reactor/Processor.h"
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


int main()
{
    TcpSocket::ptr client = std::make_shared<TcpSocket>();
    client->connect("127.0.0.1:5261");

    ProcessorProxy::Instance().setProcessor(std::make_shared<EchoProcessor>());

    std::shared_ptr<Scheduler<EpollWorker>> clientEpoller(new Scheduler<EpollWorker>(1));

    std::shared_ptr<EpollWorker> worker = clientEpoller->getNextWorker();
	std::unique_ptr<TcpConnection> connection(new TcpConnection(client, worker));
	worker->addSession(std::make_shared<TcpSession>(1, std::move(connection), worker));

    clientEpoller->schedule(
        [&]()
        {
            std::string str = "test echo";
            Packet pack(1, str.c_str(), str.size());
            worker->getSession(1)->send(pack);

        }
    );
    clientEpoller->start();
    clientEpoller->waitForQuit();
}