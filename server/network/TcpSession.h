#ifndef NETWORK_TCPSESSION_H
#define NETWORK_TCPSESSION_H

class TcpSession
{
public:
    TcpSession();
    ~TcpSession();

    TcpSession(const TcpSession& session) = delete;
    TcpSession& operator=(const TcpSession& rhs) = delete;
    
private:
    
};
#endif