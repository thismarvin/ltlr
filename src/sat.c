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

    Rectangle tmp;
    tmp.x = xMin;
    tmp.y = yMin;
    tmp.width = xMax - xMin;
    tmp.height = yMax - yMin;

    return tmp;
}

static OverlapInformation CalculateOverlap(Polygon a, Polygon b)
{
    Vector2 minNormal = { 0, 0 };
    float minOverlap = FLT_MAX;

    for (int i = 0; i < a.edgesLength; ++i)
    {
        Vector2 normal =
        {
            -(a.edges[i].end.y - a.edges[i].start.y),
                a.edges[i].end.x - a.edges[i].start.x
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
            OverlapInformation tmp = { 0 };

            return tmp;
        }
    }

    OverlapInformation result;
    result.normal = minNormal;
    result.overlap = minOverlap;
    result.valid = 1;

    return result;
}

static Vector2 RectangleGetCenter(Rectangle rectangle)
{
    Vector2 result;

    result.x = rectangle.x + rectangle.width * 0.5;
    result.y = rectangle.y + rectangle.height * 0.5;

    return result;
}

Vector2 SATGetResolution(Polygon a, Polygon b)
{
    Rectangle aAABB = CalculateAABB(a.vertices, a.verticesLength);
    Rectangle bAABB = CalculateAABB(b.vertices, b.verticesLength);

    if (!CheckCollisionRecs(aAABB, bAABB))
    {
        Vector2 tmp = { 0, 0 };

        return tmp;
    }

    OverlapInformation pass0 = CalculateOverlap(a, b);

    if (!pass0.valid)
    {
        Vector2 tmp = { 0, 0 };

        return tmp;
    }

    OverlapInformation pass1 = CalculateOverlap(b, a);

    if (!pass1.valid)
    {
        Vector2 tmp = { 0, 0 };

        return tmp;
    }

    OverlapInformation minPass = pass0.overlap < pass1.overlap ? pass0 : pass1;
    Vector2 resolution = Vector2Scale(minPass.normal, minPass.overlap);

    Vector2 aCenter = RectangleGetCenter(aAABB);
    Vector2 bCenter = RectangleGetCenter(bAABB);
    Vector2 tmp = Vector2Subtract(aCenter, bCenter);

    if (Vector2DotProduct(tmp, resolution) < 0)
    {
        resolution = Vector2Scale(resolution, -1);
    }

    return resolution;
}
