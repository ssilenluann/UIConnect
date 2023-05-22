#ifndef REACTOR_ECHOPROCESSOR_H
#define REACTOR_ECHOPROCESSOR_H
#include "Processor.h"
#include <chrono>
#include <thread>

class EchoProcessor: public Processor
{
public:
    virtual void handleMsg(Packet& msg, std::shared_ptr<TcpSession> session) override;
};
#endif