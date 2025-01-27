#include "events.h"

#include "./collections/deque.h"
#include "common.h"

EventHandler EventHandlerCreate(void)
{
	return (EventHandler) {
		.listeners = DEQUE_OF(OnRaise),
	};
}

void EventHandlerSubscribe(EventHandler* self, const OnRaise onRaise)
{
	// NOLINTNEXTLINE
	DequePushFront(&self->listeners, &onRaise);
}

void EventHandlerRaise(const EventHandler* self, const void* arguments)
{
	for (usize i = 0; i < DequeGetSize(&self->listeners); ++i)
	{
		const OnRaise* onRaise = &DEQUE_GET_UNCHECKED(&self->listeners, OnRaise, i);

		(*onRaise)(arguments);
	}
}

void EventHandlerDestroy(EventHandler* self)
{
	DequeDestroy(&self->listeners);
}
