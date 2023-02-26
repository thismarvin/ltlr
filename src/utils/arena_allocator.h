#pragma once

#include <stddef.h>

typedef size_t usize;

typedef struct
{
	void* data;
	usize head;
	usize size;
} ArenaAllocator;

ArenaAllocator ArenaAllocatorCreate(usize size);
void* ArenaAllocatorTake(ArenaAllocator* self, usize size);
void ArenaAllocatorFlush(ArenaAllocator* self);
void ArenaAllocatorDestroy(ArenaAllocator* self);
