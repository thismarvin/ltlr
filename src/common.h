#pragma once

#include "raylib.h"

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

#define VECTOR2_ZERO (Vector2) { .x = 0, .y = 0 }

Vector2 Vector2Create(float x, float y);

void DebugVector2(Vector2 value);
