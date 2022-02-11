#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "raylib.h"

#define VECTOR2_ZERO (Vector2){ .x = 0, .y = 0 }

Vector2 Vector2Create(float x, float y);

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

#endif
