#pragma once

#include "raylib.h"
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

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

#define VECTOR2_ZERO (Vector2) { .x = 0, .y = 0 }

Vector2 Vector2Create(f32 x, f32 y);

void DebugVector2(Vector2 value);
