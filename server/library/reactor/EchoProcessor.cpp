#include "EchoProcessor.h"
#include "../log/Logger.h"
#include "../network/TcpSession.h"

static Logger::ptr g_logger = LOG_NAME("system");
void EchoProcessor::handleMsg(Packet & msg, std::shared_ptr<TcpSession> session)
{

    // std::this_thread::sleep_for(std::chrono::seconds(1));
    session->send(msg);

    LOG_DEBUG(g_logger) << msg.dataLoad();

}
