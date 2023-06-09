
#include "EventThreadPool.h"
#include "../../log/Logger.h"
static Logger::ptr g_logger = LOG_NAME("system");

// EventThreadPool::EventThreadPool(int size): ThreadPool<EventThread>(size){}

std::shared_ptr<EventLoop> EventThreadPool::getNextLoop()
{
	LOG_ASSERT_W(m_size > 0, "threadpoll empty");
	static int no = 0;
	no %= m_size;
	std::shared_ptr<EventLoop> loop = m_threads[no]->getLoop();
	no ++;
	return loop;
}
