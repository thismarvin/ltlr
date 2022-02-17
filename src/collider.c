#include "collider.h"
#include "constants.h"
#include "raymath.h"

Vector2 RectangleRectangleResolution(Rectangle a, Rectangle b)
{
    float aLeft = a.x;
    float aRight = a.x + a.width;
    float bLeft = b.x;
    float bRight = b.x + b.width;

    if (bRight < aLeft || aRight < bLeft) {
        return VECTOR2_ZERO;
    }

    float aTop = a.y;
    float aBottom = a.y + a.height;
    float bTop = b.y;
    float bBottom = b.y + b.height;

    if (bBottom < aTop || aBottom < bTop) {
        return VECTOR2_ZERO;
    }

    float xOverlap = MIN(aRight, bRight) - MAX(aLeft, bLeft);
    float yOverlap = MIN(aBottom, bBottom) - MAX(aTop, bTop);

    Vector2 normal = xOverlap < yOverlap ? Vector2Create(1, 0) : Vector2Create(0, 1);
    float minOverlap = MIN(xOverlap, yOverlap);

    Vector2 resolution = Vector2Scale(normal, minOverlap);

    Vector2 aPosition = Vector2Create(a.x, a.y);
    Vector2 bPosition = Vector2Create(b.x, b.y);
    Vector2 difference = Vector2Subtract(aPosition, bPosition);

    if (Vector2DotProduct(difference, resolution) < 0) {
        resolution = Vector2Scale(resolution, -1);
    }

    return resolution;
}

Polygon PolygonFromRectangle(Rectangle rectangle)
{
    Polygon polygon;

    PolygonInit(&polygon, 4);

    Vector2 v0 = { rectangle.x, rectangle.y };
    Vector2 v1 = { rectangle.x, rectangle.y + rectangle.height };
    Vector2 v2 = { rectangle.x + rectangle.width, rectangle.y + rectangle.height };
    Vector2 v3 = { rectangle.x + rectangle.width, rectangle.y };

    LineSegment l0 = LineSegmentCreate(v0.x, v0.y, v1.x, v1.y);
    LineSegment l1 = LineSegmentCreate(v1.x, v1.y, v2.x, v2.y);
    LineSegment l2 = LineSegmentCreate(v2.x, v2.y, v3.x, v3.y);
    LineSegment l3 = LineSegmentCreate(v3.x, v3.y, v0.x, v0.y);

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
