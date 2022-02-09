#include "polygon.h"
#include <stdlib.h>

void PolygonInit(Polygon* self, size_t verticesLength)
{
    self->vertices = calloc(verticesLength, sizeof(Vector2));
    self->edges = calloc(verticesLength, sizeof(LineSegment));
}

void PolygonDestroy(Polygon* self)
{
    free(self->vertices);
    free(self->edges);
}

