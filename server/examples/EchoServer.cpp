// #include "socket/TcpServer.h"
// #include "log/Logger.h"
// #include "socket/ByteArray.h"

// static Logger::ptr g_logger = LOG_ROOT();

// class EchoServer : public TcpServer
// {
// public:
//     EchoServer(int type);
//     bool handleClient(Socket::ptr client);

// private:
//     int m_type = 0;
// };

// EchoServer::EchoServer(int type) : m_type(type) {}

// bool EchoServer::handleClient(Socket::ptr client)
// {
//     LOG_INFO(g_logger) << "handle client " << *client;
//     ByteArray::ptr ba(new ByteArray());

//     while(true)
//     {
//         ba->clear();
//         std::vector<iovec> iovs;
//         ba->getWriteBuffers(iovs, 1024);

//         int ret = client->recv(&iovs[0], iovs.size());
//         if(ret == 0)
//         {
//             LOG_INFO(g_logger) << "client close: " << *client;
//             break;
//         }
//         if(ret < 0)
//         {
//             LOG_INFO(g_logger) << "client error rt = " << ret
//                 << " errno = " << errno << ", errstr: " << strerror(errno);
//                 break;
//         }

//         ba->setPosition(ba->getPosition() + ret);
//         ba->setPosition(0);

//         LOG_INFO(g_logger) << (m_type == 1 ? ba->toString() : ba->toHexString());
//     }

//     return true;
// }

// void run()
// {
//     EchoServer::ptr es(new EchoServer(1));
//     auto addr = Address::LookupAny("0.0.0.0:5261");
//     while(!es->bind(addr))
//     {
//         std::this_thread::sleep_for(std::chrono::seconds(5));
//     }

//     es->start();

//     // while(1);
// }


int main(int argc, char** argv)
{
    // EpollScheduler::ptr iom(new EpollScheduler(1, "", true));
    // iom->start(iom);
    // iom->schedule(run);

    // iom->callThreadJoinWork();
    return 0;
}