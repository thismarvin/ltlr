#include "bit_mask.h"

#include <math.h>
#include <stdlib.h>

BitMask BitMaskCreate(const usize width, const usize height)
{
	const usize area = width * height;
	const usize length = ceil(1.0 * area / BIT_MASK_ENTRY_TOTAL_BITS);
	const usize size = length * BIT_MASK_ENTRY_SIZE;

	return (BitMask) {
		.width = width,
		.height = height,
		.contents = calloc(length, BIT_MASK_ENTRY_SIZE),
		.size = size,
	};
}

bool BitMaskGet(const BitMask* self, const i32 x, const i32 y)
{
	if (x < 0 || (usize)x >= self->width || y < 0 || (usize)y >= self->height)
	{
		return false;
	}

	const usize index = y * self->width + x;
	const usize container = index / BIT_MASK_ENTRY_TOTAL_BITS;

	const BIT_MASK_ENTRY_TYPE target = self->contents[container];
	const usize adjusted = index % BIT_MASK_ENTRY_TOTAL_BITS;

	const u8 bit = (target >> adjusted) & 1;

	return bit == 1;
}

void BitMaskSet(BitMask* self, const i32 x, const i32 y, const bool value)
{
	if (x < 0 || (usize)x >= self->width || y < 0 || (usize)y >= self->height)
	{
		return;
	}

	const usize index = y * self->width + x;
	const usize container = index / BIT_MASK_ENTRY_TOTAL_BITS;

	const BIT_MASK_ENTRY_TYPE target = self->contents[container];
	const usize adjusted = index % BIT_MASK_ENTRY_TOTAL_BITS;

	const BIT_MASK_ENTRY_TYPE shifted = (BIT_MASK_ENTRY_TYPE)1 << adjusted;
	const BIT_MASK_ENTRY_TYPE replacement = value ? target | shifted : target & (~shifted);

	self->contents[container] = replacement;
}

void BitMaskDestroy(BitMask* self)
{
	free(self->contents);
}
