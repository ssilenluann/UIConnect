#include "protocol/http/HttpServer.h"
#include "reactor/IndexHttpProcessor.h"
#include "reactor/HttpDispatcher.h"

int main(int argc, char** argv)
{
    HttpDispatcher::Instance().Regist("/index", std::make_shared<IndexHttpProcessor>());

    std::shared_ptr<HttpServer> server(new HttpServer(2));
    server->init("127.0.0.1", 5261);
    server->run();

    return 0;
}