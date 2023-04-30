#include "socket/TcpServer.h"
#include "log/Logger.h"
#include "socket/Address.h"

static Logger::ptr g_logger = LOG_NAME("system");

void run()
{
    auto addr= Address::LookupAny("0.0.0.0:8033");
    auto addrUnix = UnixAddress::ptr(new UnixAddress("/tmp/unix_addr"));
    std::vector<Address::ptr> addrs;
    addrs.push_back(addr);
    addrs.push_back(addrUnix);

    TcpServer::ptr server(new TcpServer);
    std::vector<Address::ptr> failed;
    while(!server->bind(addrs, failed))
    {
        sleep(5000);
    }

    server->start();

    for(;;);
    // server->stop();
}

int main()
{
    run();
    return 0;
}
