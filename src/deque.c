#include "assert.h"
#include "deque.h"
#include <string.h>

#define DEQUE_RESIZE_FACTOR 2

// An internal get: used for indexing the internal m_data array.
static void* Get(const Deque* deque, const usize index)
{
    return (u8*)deque->m_data + deque->m_dataSize * index;
}

// An internal set: used for indexing the internal m_data array.
static void Set(Deque* deque, const usize index, const void* valuePointer)
{
    void* destination = Get(deque, index);
    memcpy(destination, valuePointer, deque->m_dataSize);
}

static void Resize(Deque* self)
{
    u8* oldData = self->m_data;
    const usize oldCapacity = self->m_capacity;
    const void* tailChunkStart = oldData + (self->m_tailIndex + 1) * self->m_dataSize;
    const usize tailChunkSize = (self->m_capacity - (self->m_tailIndex + 1)) * self->m_dataSize;
    const void* headChunkStart = oldData;
    const usize headChunkSize = (self->m_tailIndex + 1) * self->m_dataSize;

    const usize newCapacity = (usize)(self->m_capacity * DEQUE_RESIZE_FACTOR);
    u8* newData = malloc(self->m_dataSize * newCapacity);

    self->m_data = newData;
    self->m_capacity = newCapacity;
    self->m_headIndex = oldCapacity;
    self->m_tailIndex = newCapacity - 1;
    self->m_needsResize = false;

    memcpy(newData, tailChunkStart, tailChunkSize);
    memcpy(newData + tailChunkSize, headChunkStart, headChunkSize);

    free(oldData);
}

Deque DequeCreate(const usize dataSize, const usize initialCapacity)
{
    void* data = malloc(dataSize * initialCapacity);

    return (Deque)
    {
        .m_data = data,
        .m_dataSize = dataSize,
        .m_capacity = initialCapacity,
        .m_headIndex = 0,
        .m_tailIndex = initialCapacity - 1,
        .m_needsResize = false,
    };
}

void DequePushFront(Deque* self, const void* valuePointer)
{
    if (self->m_needsResize)
    {
        Resize(self);
    }
    else if (DequeGetSize(self) == self->m_capacity - 1)
    {
        self->m_needsResize = true;
    }

    Set(self, self->m_headIndex, valuePointer);
    self->m_headIndex = (self->m_headIndex + 1) % self->m_capacity;
}

void DequePushBack(Deque* self, const void* valuePointer)
{
    if (self->m_needsResize)
    {
        Resize(self);
    }
    else if (DequeGetSize(self) == self->m_capacity - 1)
    {
        self->m_needsResize = true;
    }

    Set(self, self->m_tailIndex, valuePointer);

    if (self->m_tailIndex == 0)
    {
        self->m_tailIndex = self->m_capacity - 1;
    }
    else
    {
        self->m_tailIndex -= 1;
    }
}

void* DequePopFront(Deque* self)
{
    assert(DequeGetSize(self) > 0);

    void* result = DequePeekFront(self);

    if (self->m_headIndex == 0)
    {
        self->m_headIndex = self->m_capacity - 1;
    }
    else
    {
        self->m_headIndex -= 1;
    }

    return result;
}

void* DequePopBack(Deque* self)
{
    assert(DequeGetSize(self) > 0);

    void* result = DequePeekBack(self);
    self->m_tailIndex = (self->m_tailIndex + 1) % self->m_capacity;

    return result;
}

void* DequePeekFront(const Deque* self)
{
    assert(DequeGetSize(self) > 0);

    if (self->m_headIndex == 0)
    {
        return Get(self, self->m_capacity - 1);
    }

    return Get(self, self->m_headIndex - 1);
}

void* DequePeekBack(const Deque* self)
{
    assert(DequeGetSize(self) > 0);

    return Get(self, (self->m_tailIndex + 1) % self->m_capacity);
}

void* DequeGetUnchecked(const Deque* self, const usize index)
{
    return Get(self, (self->m_tailIndex + 1 + index) % self->m_capacity);
}

void DequeSetUnchecked(Deque* self, const usize index, const void* valuePointer)
{
    Set(self, (self->m_tailIndex + 1 + index) % self->m_capacity, valuePointer);
}

void DequeClear(Deque* self)
{
    self->m_headIndex = 0;
    self->m_tailIndex = self->m_capacity - 1;
}

usize DequeGetSize(const Deque* self)
{
    if (self->m_needsResize)
    {
        return self->m_capacity;
    }

    if (self->m_headIndex == self->m_tailIndex)
    {
        return self->m_capacity - 1;
    }

    if (self->m_headIndex < self->m_tailIndex)
    {
        return self->m_headIndex + self->m_capacity - self->m_tailIndex - 1;
    }

    return self->m_headIndex - self->m_tailIndex - 1;
}

void DequeDestroy(Deque* self)
{
    free(self->m_data);
}
