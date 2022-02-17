#pragma once

#include "raylib.h"

typedef struct
{
    Vector2 start;
    Vector2 end;
} LineSegment;

LineSegment LineSegmentCreate(float x1, float y1, float x2, float y2);
