#ifndef HOOK_HOOK_CPP
#define HOOK_HOOK_CPP

#include "Hook.h"
#include "log/Logger.h"
#include "config/Config.h"

static thread_local bool t_hook_enabled = false;
static Logger::ptr g_logger = LOG_NAME("system");
static ConfigItem<int>::ptr g_tcp_connect_timeout = Config::SearchOrAdd("tcp.connect.timeout", 5000, "tcp connect timeout");

bool Hook::IsHookEnabled()
{
    return t_hook_enabled;
}

void Hook::SetHookEnable(bool flag)
{
    t_hook_enabled = flag;
}

#define HOOK_FUNC(xx)
#endif