#pragma once

#include "common.h"

typedef struct
{
    // A pointer to the Deque's internal data (which acts as a circular buffer).
    void* m_data;
    // The size of a single element in bytes.
    usize m_dataSize;
    // The maximum number of elements that the deque can hold.
    usize m_capacity;
    // The next index to insert into the front of the deque.
    usize m_headIndex;
    // The next index to insert into the back of the deque.
    usize m_tailIndex;
    // True if the Deque's internal array is full.
    bool m_needsResize;
} Deque;

Deque DequeCreate(usize dataSize, usize initialCapacity);
void DequePushFront(Deque* self, const void* valuePointer);
void DequePushBack(Deque* self, const void* valuePointer);
void* DequePopFront(Deque* self);
void* DequePopBack(Deque* self);
void* DequePeekFront(const Deque* self);
void* DequePeekBack(const Deque* self);
void* DequeGetUnchecked(const Deque* self, usize index);
void* DequeGet(const Deque* self, usize index);
void DequeSetUnchecked(Deque* self, usize index, const void* valuePointer);
void DequeSet(Deque* self, usize index, const void* valuePointer);
usize DequeGetSize(const Deque* self);
void DequeClear(Deque* self);
void DequeDestroy(Deque* self);

#define DEQUE_OF(mType) DequeCreate(sizeof(mType), 16)
#define DEQUE_WITH_CAPACITY(mType, mCapacity) DequeCreate(sizeof(mType), mCapacity)
#define DEQUE_PUSH_FRONT(mDequePtr, mType, mValue) \
{ \
    mType temp = mValue; \
    DequePushFront(mDequePtr, &temp); \
}
#define DEQUE_PUSH_BACK(mDequePtr, mType, mValue) \
{ \
    mType temp = mValue; \
    DequePushBack(mDequePtr, &temp); \
}
#define DEQUE_POP_FRONT(mDequePtr, mType) (*((mType*)DequePopFront(mDequePtr)))
#define DEQUE_POP_BACK(mDequePtr, mType) (*((mType*)DequePopBack(mDequePtr)))
#define DEQUE_PEEK_FRONT(mDequePtr, mType) (*((mType*)DequePeekFront(mDequePtr)))
#define DEQUE_PEEK_BACK(mDequePtr, mType) (*((mType*)DequePeekBack(mDequePtr)))
#define DEQUE_GET_UNCHECKED(mDequePtr, mType, mIndex) (*((mType*)DequeGetUnchecked(mDequePtr, mIndex)))
#define DEQUE_SET_UNCHECKED(mDequePtr, mIndex, mType, mValue) \
{ \
    mType temp = mValue; \
    DequeSetUnchecked(mDequePtr, mIndex, &temp); \
}
#define DEQUE_GET(mDequePtr, mType, mIndex) (*((mType*)DequeGet(mDequePtr, mIndex)))
#define DEQUE_SET(mDequePtr, mIndex, mType, mValue) \
{ \
    mType temp = mValue; \
    DequeSet(mDequePtr, mIndex, &temp); \
}
