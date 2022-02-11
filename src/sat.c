#include "constants.h"
#include "raymath.h"
#include "sat.h"
#include <float.h>

typedef struct
{
    Vector2 normal;
    float overlap;
    int valid;
} OverlapInformation;

static Rectangle CalculateAABB(Vector2* vertices, size_t verticesLength)
{
    float xMin = vertices[0].x;
    float xMax = xMin;
    float yMin = vertices[0].y;
    float yMax = yMin;

    for (int i = 1; i < verticesLength; ++i)
    {
        xMin = MIN(xMin, vertices[i].x);
        xMax = MAX(xMax, vertices[i].x);
        yMin = MIN(yMin, vertices[i].y);
        yMax = MAX(yMax, vertices[i].y);
    }

    return (Rectangle)
    {
        .x = xMin,
        .y = yMin,
        .width = xMax - xMin,
        .height = yMax - yMin
    };
}

static OverlapInformation CalculateOverlap(Polygon a, Polygon b)
{
    Vector2 minNormal = { 0, 0 };
    float minOverlap = FLT_MAX;

    for (int i = 0; i < a.edgesLength; ++i)
    {
        Vector2 normal = (Vector2)
        {
            .x = -(a.edges[i].end.y - a.edges[i].start.y),
            .y = a.edges[i].end.x - a.edges[i].start.x
        };

        normal = Vector2Normalize(normal);

        float minProjectionA = FLT_MAX;
        float maxProjectionA = FLT_MIN;

        for (int j = 0; j < a.verticesLength; ++j)
        {
            float projection = Vector2DotProduct(a.vertices[j], normal);
            minProjectionA = MIN(minProjectionA, projection);
            maxProjectionA = MAX(maxProjectionA, projection);
        }

        float minProjectionB = FLT_MAX;
        float maxProjectionB = FLT_MIN;

        for (int j = 0; j < b.verticesLength; ++j)
        {
            float projection = Vector2DotProduct(b.vertices[j], normal);
            minProjectionB = MIN(minProjectionB, projection);
            maxProjectionB = MAX(maxProjectionB, projection);
        }

        float overlap = MIN(maxProjectionA, maxProjectionB) - MAX(minProjectionA, minProjectionB);

        if (overlap < minOverlap)
        {
            minOverlap = overlap;
            minNormal = normal;
        }

        if (maxProjectionB < minProjectionA || maxProjectionA < minProjectionB)
        {
            return (OverlapInformation)
            {
                .normal = { 0, 0 },
                .overlap = 0,
                .valid = 0
            };
        }
    }

    return (OverlapInformation)
    {
        .normal = minNormal,
        .overlap = minOverlap,
        .valid = 1
    };
}

static Vector2 RectangleGetCenter(Rectangle rectangle)
{
    return (Vector2)
    {
        .x = rectangle.x + rectangle.width * 0.5,
        .y = rectangle.y + rectangle.height * 0.5
    };
}

Vector2 SATGetResolution(Polygon a, Polygon b)
{
    Rectangle aAABB = CalculateAABB(a.vertices, a.verticesLength);
    Rectangle bAABB = CalculateAABB(b.vertices, b.verticesLength);

    if (!CheckCollisionRecs(aAABB, bAABB))
    {
        return VECTOR2_ZERO;
    }

    OverlapInformation pass0 = CalculateOverlap(a, b);

    if (!pass0.valid)
    {
        return VECTOR2_ZERO;
    }

    OverlapInformation pass1 = CalculateOverlap(b, a);

    if (!pass1.valid)
    {
        return VECTOR2_ZERO;
    }

    OverlapInformation minPass = pass0.overlap < pass1.overlap ? pass0 : pass1;
    Vector2 resolution = Vector2Scale(minPass.normal, minPass.overlap);

    Vector2 aCenter = RectangleGetCenter(aAABB);
    Vector2 bCenter = RectangleGetCenter(bAABB);
    Vector2 difference = Vector2Subtract(aCenter, bCenter);

    if (Vector2DotProduct(difference, resolution) < 0)
    {
        resolution = Vector2Scale(resolution, -1);
    }

    return resolution;
}
