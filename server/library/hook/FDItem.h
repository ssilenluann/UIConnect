#ifndef HOOK_FDITEM_H
#define HOOK_FDITEM_H

#include <memory>

class FDItem: public std::enable_shared_from_this<FDItem>
{
public:
    typedef std::shared_ptr<FDItem> ptr;

    FDItem(int fd);
    ~FDItem();

    inline bool isInited() const { return m_isInited;}
    inline bool isSocket() const { return m_isSocket;}
    inline bool isClosed() const { return m_isClosed;}

    inline void setUserNonblock(bool v) { m_userNonblock = v;}
    inline bool getUserNonblock() const { return m_userNonblock;}

    inline void setSysNonblock(bool v) { m_sysNonblock = v;}
    inline bool getSysNonblock() const { return m_sysNonblock;}

    /**
     * @brief set timeout
     * @param[in] type SO_RCVTIMEO(read timeout), SO_SNDTIMEO(send timeout)
     * @param[in] v time
     */
    void setTimeout(int type, uint64_t v);

    /**
     * @brief get timeout
     * @param[in] type SO_RCVTIMEO(read timeout), SO_SNDTIMEO(send timeout)
     * @return timeout
     */
    uint64_t getTimeout(int type);

private:
    bool init();
private:
    bool m_isInited;  // inited
    bool m_isSocket;    // is socket
    bool m_sysNonblock;     // nonblock, by hook
    bool m_userNonblock;    // nonblock, by user call system func
    bool m_isClosed;    // fd closed
    int m_fd;       // fd
    uint64_t m_recvTimeout;     // fd recv timeout
    uint64_t m_sendTimeout;     // fd send timeout
};
#endif
