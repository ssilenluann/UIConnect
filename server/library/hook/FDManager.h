#ifndef HOOK_FDMANAGER_H
#define HOOK_FDMANAGER_H

#include "Mutex.h"
#include "FDItem.h"
#include <vector>
class FDManager
{
public:
    typedef RWMutex RWMutexType;

    FDManager();

    /**
     * @brief get/create fd item
     * @param[in] fd target fd
     * @param[in] create if not exist, create a new one
     * @return item class
    */
    FDItem::ptr get(int fd, bool create = false);
    
    void del(int fd);

private:
    RWMutexType m_mutex;
    std::vector<FDItem::ptr> m_fds;
};
#endif