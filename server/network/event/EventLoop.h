#ifndef NETWORK_EVENT_EVENTLOOP_H
#define NETWORK_EVENT_EVENTLOOP_H

#include <thread>
#include <memory>
#include <map>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <functional>
#include "../epoll/Epoll.h"
class TcpChannel;

class EventLoop
{
    typedef std::vector<std::shared_ptr<TcpChannel>> CHANNEL_LIST;
	typedef std::function<void()> TASK_FUNCTION;
	typedef std::vector<TASK_FUNCTION> TASK_LIST;

public:
    EventLoop(SOCKET sock = INVALID_SOCKET, std::thread::id id = std::this_thread::get_id());
    ~EventLoop();

    EventLoop(const EventLoop& loop) = delete;
    EventLoop& operator=(const EventLoop& rhs) = delete;

public:
    void loop();
    void quit();
    bool isInLoopThread();
    bool updateChannel(SOCKET fd, std::shared_ptr<TcpChannel> pChannel, int action, int type);
    bool checkChannelInLoop(std::shared_ptr<TcpChannel>& pChannel);
	void addTask(TASK_FUNCTION task);
	void doTasks();
    
private:
    std::thread::id m_threadId;
    std::mutex m_mutex;
    std::condition_variable m_cv;

    bool m_isQuiting;
    bool m_isRunning;

	Epoll m_epoll;
    CHANNEL_LIST m_activeChannels;
    TASK_LIST m_tasks;
};
#endif
