#include "assert.h"
#include "usize_deque.h"

#define USIZE_DEQUE_RESIZE_FACTOR 2

static void Resize(UsizeDeque* self)
{
    const usize oldCapacity = self->m_capacity;
    usize* oldData = self->m_data;

    const usize newCapacity = (usize)(self->m_capacity * USIZE_DEQUE_RESIZE_FACTOR);
    usize* newData = malloc(sizeof(usize) * newCapacity);

    self->m_data = newData;
    self->m_capacity = newCapacity;
    self->m_headIndex = 0;
    self->m_tailIndex = newCapacity - 1;
    self->m_needsResize = false;

    for (usize i = 0; i < oldCapacity; ++i)
    {
        newData[self->m_headIndex] = oldData[i];
        self->m_headIndex += 1;
    }

    free(oldData);
}

UsizeDeque UsizeDequeCreate(const usize initialCapacity)
{
    usize* data = malloc(sizeof(usize) * initialCapacity);

    return (UsizeDeque)
    {
        .m_data = data,
        .m_capacity = initialCapacity,
        .m_headIndex = 0,
        .m_tailIndex = initialCapacity - 1,
        .m_needsResize = false,
    };
}

void UsizeDequePushFront(UsizeDeque* self, const usize value)
{
    if (self->m_needsResize)
    {
        Resize(self);
    }
    else if (UsizeDequeGetSize(self) == self->m_capacity - 1)
    {
        self->m_needsResize = true;
    }

    self->m_data[self->m_headIndex] = value;
    self->m_headIndex = (self->m_headIndex + 1) % self->m_capacity;
}

void UsizeDequePushBack(UsizeDeque* self, const usize value)
{
    if (self->m_needsResize)
    {
        Resize(self);
    }
    else if (UsizeDequeGetSize(self) == self->m_capacity - 1)
    {
        self->m_needsResize = true;
    }

    self->m_data[self->m_tailIndex] = value;

    if (self->m_tailIndex == 0)
    {
        self->m_tailIndex = self->m_capacity - 1;
    }
    else
    {
        self->m_tailIndex -= 1;
    }
}

usize UsizeDequePopFront(UsizeDeque* self)
{
    assert(UsizeDequeGetSize(self) > 0);

    const usize result = UsizeDequePeekFront(self);

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

usize UsizeDequePopBack(UsizeDeque* self)
{
    assert(UsizeDequeGetSize(self) > 0);

    const usize result = UsizeDequePeekBack(self);
    self->m_tailIndex = (self->m_tailIndex + 1) % self->m_capacity;
    return result;
}

usize UsizeDequePeekFront(const UsizeDeque* self)
{
    assert(UsizeDequeGetSize(self) > 0);

    if (self->m_headIndex == 0)
    {
        return self->m_data[self->m_capacity - 1];
    }

    return self->m_data[self->m_headIndex - 1];
}

usize UsizeDequePeekBack(const UsizeDeque* self)
{
    assert(UsizeDequeGetSize(self) > 0);

    return self->m_data[(self->m_tailIndex + 1) % self->m_capacity];
}

usize UsizeDequeGetSize(const UsizeDeque* self)
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

void UsizeDequeDestroy(UsizeDeque* self)
{
    free(self->m_data);
}
