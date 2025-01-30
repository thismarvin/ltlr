#pragma once

#include <raylib.h>
#include <stdint.h>
#include <stdlib.h>

typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;
typedef size_t usize;
typedef float f32;
typedef double f64;

#define UNUSED __attribute__((unused))

#define MAX(mA, mB) (((mA) > (mB)) ? (mA) : (mB))
#define MIN(mA, mB) (((mA) < (mB)) ? (mA) : (mB))
#define SIGN(mA) ((mA) == 0 ? 0 : (mA) < 0 ? -1 : 1)

#define VECTOR2_ZERO (Vector2) { 0.0F, 0.0F }

#define COLOR_TRANSPARENT (Color) { 0, 0, 0, 0 }
#define COLOR_BLACK (Color) { 0, 0, 0, 255 }
#define COLOR_WHITE (Color) { 255, 255, 255, 255 }
#define COLOR_RED (Color) { 255, 0, 0, 255 }
#define COLOR_GREEN (Color) { 0, 255, 0, 255 }
#define COLOR_BLUE (Color) { 0, 0, 255, 255 }

typedef enum
{
	ORD_LESS,
	ORD_EQUAL,
	ORD_GREATER,
} Ordering;

typedef enum
{
	DIR_NONE = 0,
	DIR_LEFT = 1 << 0,
	DIR_UP = 1 << 1,
	DIR_RIGHT = 1 << 2,
	DIR_DOWN = 1 << 3,
} Direction;

typedef enum
{
	REFLECTION_NONE = 0,
	REFLECTION_REVERSE_X_AXIS = 1 << 0,
	REFLECTION_REVERSE_Y_AXIS = 1 << 1,
} Reflection;

Vector2 Vector2Create(f32 x, f32 y);

f32 RectangleLeft(Rectangle self);
f32 RectangleRight(Rectangle self);
f32 RectangleBottom(Rectangle self);
f32 RectangleTop(Rectangle self);
bool RectangleContains(Rectangle self, Rectangle other);

Color ColorMultiply(Color color, f32 alpha);
