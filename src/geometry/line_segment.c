#include "line_segment.h"

LineSegment LineSegmentCreate(const f32 x1, const f32 y1, const f32 x2, const f32 y2)
{
	return (LineSegment) {
		.start = (Vector2) { x1, y1 },
		.end = (Vector2) { x2, y2 },
	};
}
