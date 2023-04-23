#include <sys/epoll.h>
#include <thread>
#include <functional>
#include <iostream>
#include <chrono>
#include <string.h>
#include "log/Logger.h"
#include <network/socket/TcpSocket.h>
int ep_fd = 0;

void test1(int id, int time)
{
    epoll_event events[10];
    memset(events, 0, sizeof(events));
    while(true)
    {
        epoll_wait(ep_fd, events, 10, time);
        std::cout << "thread id = " << id << ", timeout" << std::endl;
    }
    
}

void test2(int id, std::string ip, int port)
{
    TcpSocket socket;
    if(socket.bind(ip, port) < 0)
        LOG_INFO(LOG_ROOT()) << "bind error";
    
    if(socket.listen() < 0)
        LOG_INFO(LOG_ROOT()) << "listen error";

    epoll_event eve;
    memset(&eve, 0, sizeof(epoll_event));
    eve.data.fd = socket.fd();
    eve.events = EPOLLET | EPOLLIN;
    epoll_ctl(ep_fd, EPOLL_CTL_ADD, socket.fd(), &eve);

    epoll_event events[10];
    memset(events, 0, sizeof(events));
    while(true)
    {
        epoll_wait(ep_fd, events, 10, -1);
        std::cout << "thread id = " << id << ", trigger" << std::endl;
    }
}


int main()
{
    ep_fd = epoll_create(3);

    std::thread t1(&test2, 1, "127.0.0.1", 5261);
    std::thread t2(&test2, 2, "127.0.0.1", 5262);

    TcpSocket socket;
    socket.connect("127.0.0.1", 5261);
    std::this_thread::sleep_for(std::chrono::seconds(5));

    while(true)
    {

    }

    return 0;
}