#include "line_segment.h"

LineSegment LineSegmentCreate(float x1, float y1, float x2, float y2)
{
    return (LineSegment)
    {
        .start = (Vector2) { x1, y1 },
        .end = (Vector2) { x2, y2 },
    };
}
