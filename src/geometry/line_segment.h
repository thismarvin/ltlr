#pragma once

#include "../common.h"

typedef struct
{
    Vector2 start;
    Vector2 end;
} LineSegment;

LineSegment LineSegmentCreate(f32 x1, f32 y1, f32 x2, f32 y2);
