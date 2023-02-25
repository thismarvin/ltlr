#include "arena_allocator.h"

#include <assert.h>
#include <stdlib.h>

ArenaAllocator ArenaAllocatorCreate(const usize capacity)
{
	return (ArenaAllocator) {
		.data = malloc(capacity),
		.head = 0,
		.capacity = capacity,
	};
}

void* ArenaAllocatorTake(ArenaAllocator* self, const usize size)
{
	const usize offset = self->head;
	self->head += size;
	assert(self->head <= self->capacity);
	return (char*)self->data + offset;
}

void ArenaAllocatorFlush(ArenaAllocator* self)
{
	self->head = 0;
}

void ArenaAllocatorDestroy(ArenaAllocator* self)
{
	free(self->data);
}
