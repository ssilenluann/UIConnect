#ifndef SOCKET_SOCKET_H
#define SOCKET_SOCKET_H

#include <memory>
#include <sys/socket.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include "Address.h"
#include "../Noncopiable.h"

class Socket : public std::enable_shared_from_this<Socket>, Noncopyable {
public:
    typedef std::shared_ptr<Socket> ptr;
    typedef std::weak_ptr<Socket> weak_ptr;

    enum Type {
        TCP = SOCK_STREAM,
        UDP = SOCK_DGRAM
    };

    enum Family {
        /// IPv4 socket
        IPv4 = AF_INET,
        /// IPv6 socket
        IPv6 = AF_INET6,
        /// Unix socket
        UNIX = AF_UNIX,
    };

    static Socket::ptr CreateTCP(Address::ptr address);

    static Socket::ptr CreateUDP(Address::ptr address);

    static Socket::ptr CreateTCPSocket();

    static Socket::ptr CreateUDPSocket();

    static Socket::ptr CreateTCPSocket6();

    static Socket::ptr CreateUDPSocket6();

    static Socket::ptr CreateUnixTCPSocket();

    static Socket::ptr CreateUnixUDPSocket();

    Socket(int family, int type, int protocol = 0);

    virtual ~Socket();

    int64_t getSendTimeout();

    void setSendTimeout(int64_t v);

    int64_t getRecvTimeout();

    void setRecvTimeout(int64_t v);

    bool getOption(int level, int option, void* result, socklen_t* len);

    template<class T>
    bool getOption(int level, int option, T& result) {
        socklen_t length = sizeof(T);
        return getOption(level, option, &result, &length);
    }

    bool setOption(int level, int option, const void* result, socklen_t len);

    template<class T>
    bool setOption(int level, int option, const T& value) {
        return setOption(level, option, &value, sizeof(T));
    }

    virtual Socket::ptr accept();

    virtual bool bind(const Address::ptr addr);

    virtual bool connect(const Address::ptr addr, uint64_t timeout_ms = -1);

    virtual bool reconnect(uint64_t timeout_ms = -1);

    virtual bool listen(int backlog = SOMAXCONN);

    virtual bool close();

    /**
     * @return
     *      @retval >0 succeed
     *      @retval =0 socket closed
     *      @retval <0 socket error
     */
    virtual int send(const void* buffer, size_t length, int flags = 0);

    /**
     * @return
     *      @retval >0 succeed
     *      @retval =0 socket closed
     *      @retval <0 socket error
     */
    virtual int send(const iovec* buffers, size_t length, int flags = 0);

    virtual int sendTo(const void* buffer, size_t length, const Address::ptr to, int flags = 0);

    virtual int sendTo(const iovec* buffers, size_t length, const Address::ptr to, int flags = 0);

    virtual int recv(void* buffer, size_t length, int flags = 0);

    virtual int recv(iovec* buffers, size_t length, int flags = 0);

    virtual int recvFrom(void* buffer, size_t length, Address::ptr from, int flags = 0);

    virtual int recvFrom(iovec* buffers, size_t length, Address::ptr from, int flags = 0);

    Address::ptr getRemoteAddress();

    Address::ptr getLocalAddress();

    int getFamily() const { return m_family;}

    int getType() const { return m_type;}

    int getProtocol() const { return m_protocol;}

    bool isConnected() const { return m_isConnected;}

    bool isValid() const;

    int getError();

    virtual std::ostream& dump(std::ostream& os) const;

    virtual std::string toString() const;

    int getSocket() const { return m_sock;}

    bool cancelRead();

    bool cancelWrite();

    bool cancelAccept();

    bool cancelAll();
protected:

    void initSock();

    void newSock();

    virtual bool init(int sock);
protected:
    int m_sock;
    int m_family;
    int m_type;
    int m_protocol;
    bool m_isConnected;
    Address::ptr m_localAddress;
    Address::ptr m_remoteAddress;
};

class SSLSocket : public Socket {
public:
    typedef std::shared_ptr<SSLSocket> ptr;

    static SSLSocket::ptr CreateTCP(Address::ptr address);
    static SSLSocket::ptr CreateTCPSocket();
    static SSLSocket::ptr CreateTCPSocket6();

    SSLSocket(int family, int type, int protocol = 0);
    virtual Socket::ptr accept() override;
    virtual bool bind(const Address::ptr addr) override;
    virtual bool connect(const Address::ptr addr, uint64_t timeout_ms = -1) override;
    virtual bool listen(int backlog = SOMAXCONN) override;
    virtual bool close() override;
    virtual int send(const void* buffer, size_t length, int flags = 0) override;
    virtual int send(const iovec* buffers, size_t length, int flags = 0) override;
    virtual int sendTo(const void* buffer, size_t length, const Address::ptr to, int flags = 0) override;
    virtual int sendTo(const iovec* buffers, size_t length, const Address::ptr to, int flags = 0) override;
    virtual int recv(void* buffer, size_t length, int flags = 0) override;
    virtual int recv(iovec* buffers, size_t length, int flags = 0) override;
    virtual int recvFrom(void* buffer, size_t length, Address::ptr from, int flags = 0) override;
    virtual int recvFrom(iovec* buffers, size_t length, Address::ptr from, int flags = 0) override;

    bool loadCertificates(const std::string& cert_file, const std::string& key_file);
    virtual std::ostream& dump(std::ostream& os) const override;
protected:
    virtual bool init(int sock) override;
private:
    std::shared_ptr<SSL_CTX> m_ctx;
    std::shared_ptr<SSL> m_ssl;
};

std::ostream& operator<<(std::ostream& os, const Socket& sock);

#endif