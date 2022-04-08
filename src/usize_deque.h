#pragma once

#include "common.h"

typedef struct
{
    // A pointer to the Deque's internal data (which acts as a circular buffer).
    usize* _data;
    // The maximum number of elements that the deque can hold.
    usize _capacity;
    // The next index to insert into the front of the deque.
    usize _headIndex;
    // The next index to insert into the back of the deque.
    usize _tailIndex;
    // True if the Deque's internal array is full.
    bool _needsResize;
} UsizeDeque;

UsizeDeque UsizeDequeCreate(const usize initialCapacity);
void UsizeDequePushFront(UsizeDeque* self, const usize value);
void UsizeDequePushBack(UsizeDeque* self, const usize value);
usize UsizeDequePopFront(UsizeDeque* self);
usize UsizeDequePopBack(UsizeDeque* self);
usize UsizeDequePeekFront(const UsizeDeque* self);
usize UsizeDequePeekBack(const UsizeDeque* self);
usize UsizeDequeGetSize(const UsizeDeque* self);
void UsizeDequeDestroy(UsizeDeque* self);
