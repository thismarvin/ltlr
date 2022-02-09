#ifndef LINE_SEGMENT_H
#define LINE_SEGMENT_H

#include "raylib.h"

typedef struct
{
    Vector2 start;
    Vector2 end;
} LineSegment;

void LineSegmentInit(LineSegment* self, float x1, float y1, float x2, float y2);

#endif
