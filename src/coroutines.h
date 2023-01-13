#pragma once

#include <stdbool.h>

typedef struct
{
    float timer;
    float duration;
    int state;
    bool done;
} Coroutine;

Coroutine CoroutineCreate(void);
void CoroutineStart(Coroutine* self);

#define CO_BEGIN() switch(coroutine->state) { case -1:
#define CO_LABEL(mLabel) case mLabel:
#define CO_GOTO(mLabel) coroutine->state = mLabel; return
#define CO_WAIT(mDuration) coroutine->duration = mDuration; coroutine->state = __LINE__; case __LINE__:; if (coroutine->timer < coroutine->duration) { return; } coroutine->timer = 0.0f; coroutine->duration = 0.0f
#define CO_YIELD() coroutine->state = __LINE__; return; case __LINE__:
#define CO_CONTINUE() return
#define CO_RETURN() coroutine->done = true; return
#define CO_END() } coroutine->done = true

// CO_RESTART() coroutine->state = 0 ????
