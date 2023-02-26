#include "arena_allocator.h"

#include <assert.h>
#include <stdlib.h>

ArenaAllocator ArenaAllocatorCreate(const usize size)
{
	return (ArenaAllocator) {
		.data = malloc(size),
		.head = 0,
		.size = size,
	};
}

void* ArenaAllocatorTake(ArenaAllocator* self, const usize size)
{
	const usize offset = self->head;

	self->head += size;

	assert(self->head <= self->size);

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
