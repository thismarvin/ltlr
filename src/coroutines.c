#include "coroutines.h"

Coroutine CoroutineCreate(void)
{
    return (Coroutine)
    {
        .state = -1,
        .timer = 0.0f,
        .duration = 0.0f,
        .done = false,
    };
}

void CoroutineStart(Coroutine* self)
{
    self->state = 0;
}


