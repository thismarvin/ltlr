#pragma once

#include "../collections/deque.h"
#include "../common.h"

typedef void (*OnRaise)(const void*);

typedef struct
{
    Deque listeners;
} EventHandler;

EventHandler EventHandlerCreate(void);
void EventHandlerSubscribe(EventHandler* self, OnRaise onRaise);
void EventHandlerRaise(const EventHandler* self, const void* arguments);
void EventHandlerDestroy(EventHandler* self);
