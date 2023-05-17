#ifndef SOCKET_TCPSERVER_CPP
#define SOCKET_TCPSERVER_CPP

#include "TcpServer.h"
#include "../config/Config.h"
#include "../log/Logger.h"

static ConfigItem<uint64_t>::ptr g_tcp_server_read_timeout = 
    Config::SearchOrAdd("tcp_server.read_timeout", (uint64_t)(60 * 1000 * 2),
        "tcp server read timeout");
static Logger::ptr g_logger = LOG_NAME("system");

TcpServer::TcpServer(): m_name("uiconnect/1.0.0"), m_readTimeout(g_tcp_server_read_timeout->getValue()), m_isStop(true)
{
}

TcpServer::~TcpServer()
{
    for(auto& sock: m_socks)    sock->close();
    m_socks.clear();
}

bool TcpServer::bind(Address::ptr addr)
{
    Socket::ptr sock = Socket::CreateTCP(addr);
    if(!sock->bind(addr))
    {
        LOG_ERROR(g_logger) << "bind failed, errno = " << errno
            << ", err string: " << strerror(errno) 
            << ", addr = [" << addr->toString() << "]";
        return false;
    }

    if(!sock->listen())
    {
        LOG_ERROR(g_logger) << "listen failed, errno = " << errno
            << ", err string: " << strerror(errno) 
            << ", addr = [" << addr->toString() << "]";
        return false;      
    }

    m_socks.push_back(sock);
    return true;
}
bool TcpServer::bind(const std::vector<Address::ptr> &addrs, std::vector<Address::ptr>& failedAddrs)
{
    for(auto& addr: addrs)
    {
        if(!bind(addr)) failedAddrs.push_back(addr);
    }

    if(!failedAddrs.empty())
    {
        m_socks.clear();
        return false;
    }

    for(auto& sock: m_socks)
        LOG_INFO(g_logger) << "sock bind, listen succeed, sock << " << sock;

    return true;
}

bool TcpServer::start()
{
    if(!m_isStop)   return true;
    m_isStop = false;

    m_worker.reset(new EpollScheduler(4));
    m_acceptWorker.reset(new EpollScheduler(4));
    m_worker->start(m_worker);
    m_acceptWorker->start(m_acceptWorker);

    for(auto& sock : m_socks)
    {
        m_acceptWorker->schedule(std::bind(&TcpServer::startAccept,
            shared_from_this(), sock));
    }

    return true;
}

bool TcpServer::startAccept(Socket::ptr& sock)
{
    while(!m_isStop)    
    {
        Socket::ptr client = sock->accept();
        client->setRecvTimeout(m_readTimeout);
        if(!client)
        {
            LOG_ERROR(g_logger) << "accept socket error, errno = " << errno
                << ", err info: " << strerror(errno);
            continue;
        }

        m_worker->schedule(std::bind(&TcpServer::handleClient, shared_from_this(), client));
    }
}

bool TcpServer::handleClient(Socket::ptr client)
{
    LOG_INFO(g_logger) << "handle client << " << client;
    return true;
}

bool TcpServer::stop()
{
    if(m_isStop)  return true;

    m_isStop = true;
    
    m_acceptWorker->stop();
    m_worker->stop();

    auto self = shared_from_this();
    m_acceptWorker->schedule([&]()
    {
        for(auto& sock: m_socks)
        {
            sock->cancelAll();
            sock->close();
        }
        m_socks.clear();
    });

    return true;
}
#endif