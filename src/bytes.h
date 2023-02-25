#pragma once

#include <stdint.h>

typedef uint8_t u8;
typedef uint32_t u32;

u32 U32SwapBytes(u32 self);
u32 U32ToBigEndian(u32 self);
u32 U32FromBigEndian(u32 self);
