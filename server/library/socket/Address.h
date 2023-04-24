#ifndef SOCKET_ADDRESS_H
#define SOCKET_ADDRESS_H

#include <memory>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <map>

class IPAddress;

class Address {
public:
    typedef std::shared_ptr<Address> ptr;

    /**
     * @brief create Address onject by sockaddr pointer
     * @param[in] addr 
     * @param[in] addrlen 
     * @return return nullptr if failed
     */
    static Address::ptr Create(const sockaddr* addr, socklen_t addrlen);

    /**
     * @brief return all address by host name
     * @param[out] result 
     * @param[in] host hostname or server name, www.baidu.com[:80]
     * @param[in] family AF_INT, AF_INT6, AF_UNIX
     * @param[in] type socket type: SOCK_STREAM, SOCK_DGRAM 
     * @param[in] protocol IPPROTO_TCP, IPPROTO_UDP 
     * @return true if succeed
     */
    static bool Lookup(std::vector<Address::ptr>& result, const std::string& host,
            int family = AF_INET, int type = 0, int protocol = 0);

    /**
     * @brief return an address by host name
     * @param[in] host hostname or server name, www.baidu.com[:80]
     * @param[in] family AF_INT, AF_INT6, AF_UNIX
     * @param[in] type socket type: SOCK_STREAM, SOCK_DGRAM 
     * @param[in] protocol IPPROTO_TCP, IPPROTO_UDP 
     * @return true if succeed
     */
    static Address::ptr LookupAny(const std::string& host,
            int family = AF_INET, int type = 0, int protocol = 0);
    static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string& host,
            int family = AF_INET, int type = 0, int protocol = 0);

    /**
     * @brief return all physical network card name, address, subnet mask
     * @param[out] result 
     * @param[in] family AF_INT, AF_INT6, AF_UNIX
     * @return 
     */
    static bool GetInterfaceAddresses(std::multimap<std::string
                    ,std::pair<Address::ptr, uint32_t> >& result,
                    int family = AF_INET);
    /**
     * @brief get a specific physical network card name, address, subnet mask
     * @param[out] result 
     * @param[in] iface 
     * @param[in] family AF_INT, AF_INT6, AF_UNIX
     * @return 
     */
    static bool GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t> >&result
                    ,const std::string& iface, int family = AF_INET);

    virtual ~Address() {}

    int getFamily() const;
    virtual const sockaddr* getAddr() const = 0;
    virtual sockaddr* getAddr() = 0;
    virtual socklen_t getAddrLen() const = 0;
    virtual std::ostream& insert(std::ostream& os) const = 0;
    std::string toString() const;

    bool operator<(const Address& rhs) const;
    bool operator==(const Address& rhs) const;
    bool operator!=(const Address& rhs) const;
};

class IPAddress : public Address {
public:
    typedef std::shared_ptr<IPAddress> ptr;

    static IPAddress::ptr Create(const char* address, uint16_t port = 0);

    virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;

    virtual IPAddress::ptr networdAddress(uint32_t prefix_len) = 0;

    virtual IPAddress::ptr subnetMask(uint32_t prefix_len) = 0;

    virtual uint32_t getPort() const = 0;

    virtual void setPort(uint16_t v) = 0;
};

class IPv4Address : public IPAddress {
public:
    typedef std::shared_ptr<IPv4Address> ptr;

    static IPv4Address::ptr Create(const char* address, uint16_t port = 0);

    IPv4Address(const sockaddr_in& address);

    IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);

    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;

    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networdAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) override;
    uint32_t getPort() const override;
    void setPort(uint16_t v) override;
private:
    sockaddr_in m_addr;
};

class IPv6Address : public IPAddress {
public:
    typedef std::shared_ptr<IPv6Address> ptr;

    static IPv6Address::ptr Create(const char* address, uint16_t port = 0);


    IPv6Address();

    IPv6Address(const sockaddr_in6& address);

    IPv6Address(const uint8_t address[16], uint16_t port = 0);

    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;

    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networdAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) override;
    uint32_t getPort() const override;
    void setPort(uint16_t v) override;
private:
    sockaddr_in6 m_addr;
};

class UnixAddress : public Address {
public:
    typedef std::shared_ptr<UnixAddress> ptr;

    UnixAddress();

    UnixAddress(const std::string& path);

    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    void setAddrLen(uint32_t v);
    std::string getPath() const;
    std::ostream& insert(std::ostream& os) const override;
private:
    sockaddr_un m_addr;
    socklen_t m_length;
};

class UnknownAddress : public Address {
public:
    typedef std::shared_ptr<UnknownAddress> ptr;
    UnknownAddress(int family);
    UnknownAddress(const sockaddr& addr);
    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;
private:
    sockaddr m_addr;
};

std::ostream& operator<<(std::ostream& os, const Address& addr);

#endif
