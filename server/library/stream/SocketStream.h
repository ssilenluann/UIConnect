#ifndef STREAM_SOCKETSTREAM_H
#define STREAM_SOCKETSTREAM_H

#include "Stream.h"
#include "../socket/Socket.h"

class SocketStream: public Stream
{
public:
    typedef std::shared_ptr<SocketStream> ptr;

    SocketStream(Socket::ptr sock);
    ~SocketStream();

    /**
     * @param[in] buffer recv buffer
     * @param[in] length size
     * 
     * @return 
     *      @retval > 0 actual recv size
     *      @retval = 0 closed
     *      @retval < 0 error
    */
    virtual int read(void* buffer, size_t length) override;

    /**
     * @param[in] ba recv buffer
     * @param[in] length size
     * 
     * @return 
     *      @retval > 0 actual recv size
     *      @retval = 0 closed
     *      @retval < 0 error
    */
    virtual int read(ByteArray::ptr ba, size_t length) override;

    /**
     * @param[in] buffer wirte buffer
     * @param[in] length size
     * 
     * @return 
     *      @retval > 0 actual wirte size
     *      @retval = 0 closed
     *      @retval < 0 error
    */
    virtual int write(void* buffer, size_t length) override;

    /**
     * @param[in] ba wirte buffer
     * @param[in] length size
     * 
     * @return 
     *      @retval > 0 actual wirte size
     *      @retval = 0 closed
     *      @retval < 0 error
    */
    virtual int write(ByteArray::ptr ba, size_t length) override;

    Socket::ptr getSocket() const { return m_socket;}

    bool isConnected() const;

    Address::ptr getRemoteAddress();
    Address::ptr getLocalAddress();
    std::string getRemoteAddressString();
    std::string getLocalAddressString();

private:
    Socket::ptr m_socket;
};

#endif