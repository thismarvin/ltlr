#include "bytes.h"

static const int endiannessHelper = 1;

#define USING_BIG_ENDIAN_BYTE_ORDERING() (*(char*)&endiannessHelper == 0)

u32 U32SwapBytes(const u32 self)
{
	const u32 a = (0xFF000000 & self) >> (8 * 3) << (8 * 0);
	const u32 b = (0x00FF0000 & self) >> (8 * 2) << (8 * 1);
	const u32 c = (0x0000FF00 & self) >> (8 * 1) << (8 * 2);
	const u32 d = (0x000000FF & self) >> (8 * 0) << (8 * 3);

	return a | b | c | d;
}

u32 U32ToBigEndian(const u32 self)
{
	if (USING_BIG_ENDIAN_BYTE_ORDERING())
	{
		return self;
	}

	return U32SwapBytes(self);
}

u32 U32FromBigEndian(const u32 self)
{
	if (USING_BIG_ENDIAN_BYTE_ORDERING())
	{
		return self;
	}

	return U32SwapBytes(self);
}
