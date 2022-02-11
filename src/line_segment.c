#include "constants.h"
#include "line_segment.h"

void LineSegmentInit(LineSegment* self, float x1, float y1, float x2, float y2)
{
    self->start = Vector2Create(x1, y1);
    self->end = Vector2Create(x2, y2);
}
