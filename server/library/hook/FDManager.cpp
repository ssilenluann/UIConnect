#ifndef HOOK_FDMANAGER_CPP
#define HOOK_FDMANAGER_CPP

#include "FDManager.h"

FDManager::FDManager()
{
    m_fds.resize(64);
}

FDItem::ptr FDManager::get(int fd, bool create)
{
    if(fd == -1)    return nullptr;

    RWMutexType::ReadLock lock(m_mutex);
    if(m_fds.size() <= fd && create == false)
        return nullptr;
    if(m_fds[fd])
        return m_fds[fd];
    lock.unlock();

    FDItem::ptr fdi(new FDItem(fd));
    
    RWMutexType::WriteLock wlock(m_mutex);
    if(fd >= m_fds.size())
        m_fds.resize(fd * 1.5);
    m_fds[fd] = fdi;

    return fdi;
}

void FDManager::del(int fd)
{
    RWMutexType::WriteLock lock(m_mutex);
    if(m_fds.size() <= fd)  return;

    m_fds[fd].reset();
}

#endif