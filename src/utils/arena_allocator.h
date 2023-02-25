#pragma once

#include <stddef.h>

typedef size_t usize;

typedef struct
{
	void* data;
	usize head;
	usize capacity;
} ArenaAllocator;

ArenaAllocator ArenaAllocatorCreate(usize capacity);
void* ArenaAllocatorTake(ArenaAllocator* self, usize size);
void ArenaAllocatorFlush(ArenaAllocator* self);
void ArenaAllocatorDestroy(ArenaAllocator* self);
