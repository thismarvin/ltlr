#ifndef POLYGON_H
#define POLYGON_H

#include "line_segment.h"
#include <stddef.h>

typedef struct
{
    Vector2* vertices;
    size_t verticesLength;
    LineSegment* edges;
    size_t edgesLength;
} Polygon;

void PolygonInit(Polygon* self, size_t verticesLength);
void PolygonDestroy(Polygon* self);

#endif
