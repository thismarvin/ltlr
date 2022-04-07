#pragma once

#include "common.h"

typedef struct
{
    // Pointer to internal data (which is a circular buffer).
    usize* _data;
    // Maximum elements that the deque can hold.
    usize _capacity;
    // Next index to insert into the front of the deque.
    usize _headIndex;
    // Next index to insert into the back of the deque.
    usize _tailIndex;
    // True if the deque is full.
    bool fullCapacity;
} UsizeDeque;

UsizeDeque UsizeDequeCreate(const usize initalCapacity);
void UsizeDequePushFront(UsizeDeque* self, const usize value);
void UsizeDequePushBack(UsizeDeque* self, const usize value);
usize UsizeDequePopFront(UsizeDeque* self);
usize UsizeDequePopBack(UsizeDeque* self);
usize UsizeDequePeekFront(const UsizeDeque* self);
usize UsizeDequePeekBack(const UsizeDeque* self);
usize UsizeDequeSize(const UsizeDeque* self);
void UsizeDequeDestroy(UsizeDeque* self);
