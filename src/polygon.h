#pragma once

#include "line_segment.h"
#include <stddef.h>

typedef struct
{
    Vector2* vertices;
    LineSegment* edges;
    size_t verticesLength;
    size_t edgesLength;
} Polygon;

Polygon PolygonCreate(size_t totalVertices);
void PolygonDestroy(Polygon* self);
