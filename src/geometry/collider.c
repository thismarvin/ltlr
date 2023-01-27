#include "collider.h"

#include "../common.h"

#include <raymath.h>

Vector2 RectangleRectangleResolution(const Rectangle a, const Rectangle b)
{
	const f32 aLeft = RectangleLeft(a);
	const f32 aRight = RectangleRight(a);
	const f32 bLeft = RectangleLeft(b);
	const f32 bRight = RectangleRight(b);

	if (bRight < aLeft || aRight < bLeft)
	{
		return VECTOR2_ZERO;
	}

	const f32 aTop = RectangleTop(a);
	const f32 aBottom = RectangleBottom(a);
	const f32 bTop = RectangleTop(b);
	const f32 bBottom = RectangleBottom(b);

	if (bBottom < aTop || aBottom < bTop)
	{
		return VECTOR2_ZERO;
	}

	const f32 xOverlap = MIN(aRight, bRight) - MAX(aLeft, bLeft);
	const f32 yOverlap = MIN(aBottom, bBottom) - MAX(aTop, bTop);

	const Vector2 normal = xOverlap < yOverlap ? Vector2Create(1, 0) : Vector2Create(0, 1);
	const f32 minOverlap = MIN(xOverlap, yOverlap);

	Vector2 resolution = Vector2Scale(normal, minOverlap);

	const Vector2 aPosition = Vector2Create(a.x, a.y);
	const Vector2 bPosition = Vector2Create(b.x, b.y);
	const Vector2 difference = Vector2Subtract(aPosition, bPosition);

	if (Vector2DotProduct(difference, resolution) < 0)
	{
		resolution = Vector2Scale(resolution, -1);
	}

	return resolution;
}

Polygon PolygonFromRectangle(const Rectangle rectangle)
{
	const Polygon polygon = PolygonCreate(4);

	const Vector2 v0 = { rectangle.x, rectangle.y };
	const Vector2 v1 = { rectangle.x, rectangle.y + rectangle.height };
	const Vector2 v2 = { rectangle.x + rectangle.width, rectangle.y + rectangle.height };
	const Vector2 v3 = { rectangle.x + rectangle.width, rectangle.y };

	const LineSegment l0 = LineSegmentCreate(v0.x, v0.y, v1.x, v1.y);
	const LineSegment l1 = LineSegmentCreate(v1.x, v1.y, v2.x, v2.y);
	const LineSegment l2 = LineSegmentCreate(v2.x, v2.y, v3.x, v3.y);
	const LineSegment l3 = LineSegmentCreate(v3.x, v3.y, v0.x, v0.y);

	polygon.vertices[0] = v0;
	polygon.vertices[1] = v1;
	polygon.vertices[2] = v2;
	polygon.vertices[3] = v3;

	polygon.edges[0] = l0;
	polygon.edges[1] = l1;
	polygon.edges[2] = l2;
	polygon.edges[3] = l3;

	return polygon;
}
