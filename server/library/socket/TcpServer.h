#ifndef SOCKET_TCPSERVER_H
#define SOCKET_TCPSERVER_H

#include <memory>
#include <functional>
#include <vector>

#include "../coroutine/IOManager.h"
#include "Socket.h"
#include "Address.h"


class TcpServer: public std::enable_shared_from_this<TcpServer>
{
public:
    typedef std::shared_ptr<TcpServer> ptr;

    TcpServer();
    virtual ~TcpServer();

    virtual bool bind(Address::ptr addr);
    virtual bool bind(const std::vector<Address::ptr>& addrs, std::vector<Address::ptr>& failedAddrs);
    virtual bool start();
    virtual bool stop();

    uint64_t getReadTimeout() const { return m_readTimeout;} 
    std::string getName() const { return m_name;}
    void setReadTimeout(uint64_t v) { m_readTimeout = v;}
    void setName(const std::string& name) { m_name = name;}

    bool isStop() const { return m_isStop;}

protected:
    virtual bool startAccept(Socket::ptr& sock);
    virtual bool handleClient(Socket::ptr client);
private:
    std::string m_name;
    uint64_t m_readTimeout;
    bool m_isStop;
    std::vector<Socket::ptr> m_socks;
    EpollScheduler::ptr m_worker;
    EpollScheduler::ptr m_acceptWorker;
};

#endif