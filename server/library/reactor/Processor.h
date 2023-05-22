#ifndef REACTOR_PROCESSOR_H
#define REACTOR_PROCESSOR_H
#include "../network/socket/Packet.h"

#include <memory>
#include <string>
#include <map>

class TcpSession;
class Processor
{
public:
    virtual void handleMsg(Packet& msg, std::shared_ptr<TcpSession> session) = 0;
};

class ProcessorProxy
{
private:
    ProcessorProxy() = default;
    ProcessorProxy(const ProcessorProxy& mgr) = delete;
    ProcessorProxy& operator =(const ProcessorProxy& rhs) = delete;

public:
    void setProcessor(const std::shared_ptr<Processor>& processor)
    {
        m_processor = processor;
    }

    void handleMsg(Packet& msg, std::shared_ptr<TcpSession> session)
    {
        m_processor->handleMsg(msg, session);
    }

    static ProcessorProxy& Instance()
    {
        static ProcessorProxy pro;
        return pro;
    }

private:
    std::shared_ptr<Processor> m_processor;
};

#endif