#include "line_segment.h"

void LineSegmentInit(LineSegment* self, float x1, float y1, float x2, float y2)
{
    Vector2 start = { x1, y1 };
    Vector2 end = { x2, y2 };

    self->start = start;
    self->end = end;
}
