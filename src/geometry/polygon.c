#include "polygon.h"

Polygon PolygonCreate(const usize totalVertices)
{
    return (Polygon)
    {
        .vertices = calloc(totalVertices, sizeof(Vector2)),
        .edges = calloc(totalVertices, sizeof(LineSegment)),
        .verticesLength = totalVertices,
        .edgesLength = totalVertices,
    };
}

void PolygonDestroy(Polygon* self)
{
    free(self->vertices);
    free(self->edges);
}
