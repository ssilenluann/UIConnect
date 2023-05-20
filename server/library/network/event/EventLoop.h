#ifndef NETWORK_EVENT_EVENTLOOP_H
#define NETWORK_EVENT_EVENTLOOP_H

#include <thread>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <functional>
#include "../epoll/Epoll.h"
#include "../../TimerFunc.h"
class TcpChannel;
class TcpSession;

class EventLoop
{
    typedef std::vector<std::shared_ptr<TcpChannel>> CHANNEL_LIST;
    typedef std::unordered_map<unsigned long, std::shared_ptr<TcpSession>> SESSION_MAP;
	typedef std::function<void()> TASK_FUNCTION;
	typedef std::vector<TASK_FUNCTION> TASK_LIST;

public:
    EventLoop(std::thread::id id = std::this_thread::get_id());
    ~EventLoop();

    EventLoop(const EventLoop& loop) = delete;
    EventLoop& operator=(const EventLoop& rhs) = delete;

public:
    void loop();
    void quit();
    bool isInLoopThread();
    bool updateChannel(SOCKET fd, std::shared_ptr<TcpChannel> pChannel, int action, uint32_t event);
    bool checkChannelInLoop(std::shared_ptr<TcpChannel>& pChannel);
	void addTask(TASK_FUNCTION task);
	void doTasks();
    void removeSession(unsigned long sessionId);
    TimerFunc::ptr addTimer(uint64_t cycle, std::function<void()> cb, bool oneshot = true);
    
    template<class T>
    void addSession(std::shared_ptr<T> session)
    {
        addTask([session, this]()
        {
            if(!session->init())
            {
                // TOOD: log
                return;
            }

            m_sessions.emplace(session->id(), std::move(session));
        });
    }
    
private:
    std::thread::id m_threadId;
    std::mutex m_mutex;
    std::condition_variable m_cv;

    bool m_isQuiting;
    bool m_isRunning;

	Epoll m_epoll;
    CHANNEL_LIST m_activeChannels;
    SESSION_MAP m_sessions;
    TASK_LIST m_tasks;
    std::shared_ptr<TimerManager> m_timer;
};
#endif
