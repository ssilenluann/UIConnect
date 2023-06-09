#ifndef NETWORK_CALLBACK_H
#define NETWORK_CALLBACK_H

#include <functional>
#include "./socket/Packet.h"

typedef std::function<void()> CALLBACK;
typedef std::function<void (int)> EVENT_CALLBACK;
typedef std::function<void (Packet&)> MSG_CALLBACK;

#endif
