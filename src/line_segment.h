#pragma once

#include "common.h"

typedef struct
{
    Vector2 start;
    Vector2 end;
} LineSegment;

LineSegment LineSegmentCreate(const f32 x1, const f32 y1, const f32 x2, const f32 y2);
