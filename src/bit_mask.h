#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint8_t u8;
typedef int32_t i32;
typedef uint64_t u64;
typedef size_t usize;

#define BIT_MASK_ENTRY_TYPE u64
#define BIT_MASK_ENTRY_SIZE (8)
#define BIT_MASK_ENTRY_TOTAL_BITS (64)

typedef struct
{
	usize width;
	usize height;
	BIT_MASK_ENTRY_TYPE* contents;
	usize size;
} BitMask;

BitMask BitMaskCreate(usize width, usize height);
bool BitMaskGet(const BitMask* self, i32 x, i32 y);
void BitMaskSet(BitMask* self, i32 x, i32 y, bool value);
void BitMaskDestroy(BitMask* self);
