#include "protocol/http/HttpServer.h"
#include "reactor/IndexHttpProcessor.h"
#include "reactor/HttpDispatcher.h"
#include "config/Config.h"
#include <string>

int main(int argc, char** argv)
{
    HttpDispatcher::Instance().regist("/index", std::make_shared<IndexHttpProcessor>(), true);

    ConfigItem<std::string>::ptr ip = Config::SearchOrAdd("server.ip", std::string("127.0.0.1"), "server ip");
    ConfigItem<int>::ptr port = Config::SearchOrAdd("server.port", 5260, "port");

    std::shared_ptr<HttpServer> server(new HttpServer(8));
    server->init(ip->getValue(), port->getValue());
    server->run();

    return 0;
}