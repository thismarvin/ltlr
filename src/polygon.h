#pragma once

#include "common.h"
#include "line_segment.h"

typedef struct
{
    Vector2* vertices;
    LineSegment* edges;
    usize verticesLength;
    usize edgesLength;
} Polygon;

Polygon PolygonCreate(const usize totalVertices);
void PolygonDestroy(Polygon* self);
