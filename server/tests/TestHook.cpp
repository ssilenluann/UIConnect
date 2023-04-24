#include "hook/Hook.h"
#include "log/Logger.h"
#include "config/Config.h"
#include "coroutine/IOManager.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static Logger::ptr g_logger = LOG_ROOT();

void test_sleep(const IOManager::ptr& iom)
{
    iom->schedule([]()
    {
        LOG_INFO(g_logger) << "schedule to sleep 2";
        sleep(2);
        LOG_INFO(g_logger) << "sleep 2";
    });

    iom->schedule([]()
    {
        LOG_INFO(g_logger) << "schedule to sleep 3";
        sleep(3);
        LOG_INFO(g_logger) << "sleep 3";
    }); 

    iom->addTimer(2000, []()
    {
        LOG_INFO(g_logger) << "test timer 2000";
    }, false);

    iom->addTimer(3000, []()
    {
        LOG_INFO(g_logger) << "test timer 3000";
    }, false);

    iom->addTimer(6000, []()
    {
        LOG_INFO(g_logger) << "test timer 6000";
    }, false);

    LOG_INFO(g_logger) << "test sleep";
}

void test_sock()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "115.239.210.27", &addr.sin_addr.s_addr);

    LOG_INFO(g_logger) << "begin connect";
    int ret = connect(sock, (const sockaddr*)&addr, sizeof(addr));
    LOG_INFO(g_logger) << "connect ret = " << ret << ", errno = " << errno << ", info = " << strerror(errno);
    if(ret < 0) return;

    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    ret = send(sock, data, sizeof(data), 0);
    LOG_INFO(g_logger) << "send ret = " << ret << ", errno = " << errno << ", info = " << strerror(errno);
    if(ret < 0) return;

    std::string buff;
    buff.resize(4096);
    ret = recv(sock, &buff[0], buff.size(), 0);
    LOG_INFO(g_logger) << "recv ret = " << ret << ", errno = " << errno << ", info = " << strerror(errno);
    if(ret < 0) return;

    buff.resize(ret);
    LOG_INFO(g_logger) << buff;
}

int main(int argc, char** argv)
{
    IOManager::ptr iom = std::shared_ptr<IOManager>(new IOManager(4, "test hook"));
    iom->start(iom);

    test_sleep(iom);
    iom->schedule(test_sock);

    iom->stop();
    return 0;
}