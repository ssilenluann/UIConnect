#include "../library/network/socket/TcpSocket.h"
#include "./log/Logger.h"
#include "./log/outputter/LogStdoutOutputter.h"
#include "./log/outputter/LogFileOutputter.h"


#include <thread>
#include <list>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>
#include <iostream>

#define thread_count 2
#define client_count_per_thread 1500

std::vector<std::list<std::shared_ptr<TcpSocket>>> g_sockets;
std::vector<std::list<std::shared_ptr<Buffer>>> g_buffers;
void thread_func(int thread_no);

bool g_isRun = true;
Logger::ptr g_logger(new Logger("client"));

int main()
{
    auto stdoutOutputter = std::make_shared<LogStdoutOutputter>();
    stdoutOutputter->setLevel(LogLevel::Level::INFO);
    g_logger->addOutputter(stdoutOutputter);

    LogFileOutputter::ptr fileOutputter(new LogFileOutputter("./client_log.log"));
    fileOutputter->setLevel(LogLevel::Level::ERROR);
    g_logger->addOutputter(fileOutputter);

    for(int i = 0; i < thread_count; i++)
    {
        std::list<std::shared_ptr<TcpSocket>> socketList;
        std::list<std::shared_ptr<Buffer>> bufferList;
        for(int j = 0; j < client_count_per_thread; j++)
        {
            socketList.push_back(std::make_shared<TcpSocket>(false));
            bufferList.push_back(std::make_shared<Buffer>());
        }
        g_sockets.push_back(socketList);
        g_buffers.push_back(bufferList);
    }


    for(int i = 0; i < thread_count; i++)
    {
        std::thread t(std::bind(&thread_func, i));
        t.detach();
    }

    while(g_isRun)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::this_thread::sleep_for(std::chrono::seconds(5));
    return 0;
}

void thread_func(int thread_no)
{
    std::string str = "test client";
    Packet pack(1, str.c_str(), str.size());

    std::list<std::shared_ptr<TcpSocket>>::iterator it = g_sockets[thread_no].begin();
    std::list<std::shared_ptr<Buffer>>::iterator it_buffer = g_buffers[thread_no].begin();
    for(; it != g_sockets[thread_no].end();)
    {
        int retp = (*it)->connect("192.168.134.133", 5260);
        if(retp < 0)
        {
            LOG_FMT_INFO(g_logger, "connect error, fd = %d, errno = %d", (*it)->fd(), errno);
            it = g_sockets[thread_no].erase(it);
            it_buffer = g_buffers[thread_no].erase(it_buffer);
            continue;
        }

        // set msg to buffer
        bool ret = (*it_buffer)->setMsg(pack);
        if(!ret)
        {
            it = g_sockets[thread_no].erase(it);
            it_buffer = g_buffers[thread_no].erase(it_buffer);
            continue;
        }

        it ++;
        it_buffer++;
    }

    while(g_isRun)
    {
        it = g_sockets[thread_no].begin();
        it_buffer = g_buffers[thread_no].begin();

        for(; it != g_sockets[thread_no].end();)
        {
            int sentSize = 0;
            // if((*it_buffer)->empty())
            //     (*it_buffer)->setMsg(pack);

            if(!(*it)->send((*it_buffer), sentSize))
            {
                LOG_FMT_INFO(g_logger, "send error, fd = %d, errno = %d", (*it)->fd(), errno);
                it = g_sockets[thread_no].erase(it);
                it_buffer = g_buffers[thread_no].erase(it_buffer);
                continue;
            }

            if(!(*it)->recv((*it_buffer), sentSize))
            {
                LOG_FMT_INFO(g_logger, "recv error, fd = %d, errno = %d", (*it)->fd(), errno);
                it = g_sockets[thread_no].erase(it);
                it_buffer = g_buffers[thread_no].erase(it_buffer);

                continue;
            }

            auto msg = (*it_buffer)->getPack();
            // std::cout << msg->dataLoad() << std::endl;

            (*it_buffer)->setMsg(msg);

            it++;
            it_buffer++;
        }
    }
}
