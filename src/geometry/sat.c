#include "../common.h"
#include "sat.h"
#include <float.h>
#include <raymath.h>

typedef struct
{
    Vector2 normal;
    f32 overlap;
    bool valid;
} OverlapInformation;

static Rectangle CalculateAABB(const Vector2* vertices, const usize verticesLength)
{
    f32 xMin = vertices[0].x;
    f32 xMax = xMin;
    f32 yMin = vertices[0].y;
    f32 yMax = yMin;

    for (usize i = 1; i < verticesLength; ++i)
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

static OverlapInformation CalculateOverlap(const Polygon a, const Polygon b)
{
    Vector2 minNormal = VECTOR2_ZERO;
    f32 minOverlap = FLT_MAX;

    for (usize i = 0; i < a.edgesLength; ++i)
    {
        Vector2 normal = (Vector2)
        {
            .x = a.edges[i].end.y - a.edges[i].start.y,
            .y = -(a.edges[i].end.x - a.edges[i].start.x)
        };

        normal = Vector2Normalize(normal);

        f32 minProjectionA = FLT_MAX;
        f32 maxProjectionA = FLT_MIN;

        for (usize j = 0; j < a.verticesLength; ++j)
        {
            const f32 projection = Vector2DotProduct(a.vertices[j], normal);
            minProjectionA = MIN(minProjectionA, projection);
            maxProjectionA = MAX(maxProjectionA, projection);
        }

        f32 minProjectionB = FLT_MAX;
        f32 maxProjectionB = FLT_MIN;

        for (usize j = 0; j < b.verticesLength; ++j)
        {
            const f32 projection = Vector2DotProduct(b.vertices[j], normal);
            minProjectionB = MIN(minProjectionB, projection);
            maxProjectionB = MAX(maxProjectionB, projection);
        }

        const f32 overlap = MIN(maxProjectionA, maxProjectionB) - MAX(minProjectionA, minProjectionB);

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
                .valid = false
            };
        }
    }

    return (OverlapInformation)
    {
        .normal = minNormal,
        .overlap = minOverlap,
        .valid = true
    };
}

static Vector2 RectangleGetCenter(const Rectangle rectangle)
{
    return (Vector2)
    {
        .x = rectangle.x + rectangle.width * 0.5,
        .y = rectangle.y + rectangle.height * 0.5
    };
}

Vector2 SATGetResolution(const Polygon a, const Polygon b)
{
    const Rectangle aAABB = CalculateAABB(a.vertices, a.verticesLength);
    const Rectangle bAABB = CalculateAABB(b.vertices, b.verticesLength);

    if (!CheckCollisionRecs(aAABB, bAABB))
    {
        return VECTOR2_ZERO;
    }

    const OverlapInformation pass0 = CalculateOverlap(a, b);

    if (!pass0.valid)
    {
        return VECTOR2_ZERO;
    }

    const OverlapInformation pass1 = CalculateOverlap(b, a);

    if (!pass1.valid)
    {
        return VECTOR2_ZERO;
    }

    const OverlapInformation minPass = pass0.overlap < pass1.overlap ? pass0 : pass1;
    Vector2 resolution = Vector2Scale(minPass.normal, minPass.overlap);

    const Vector2 aCenter = RectangleGetCenter(aAABB);
    const Vector2 bCenter = RectangleGetCenter(bAABB);
    const Vector2 difference = Vector2Subtract(aCenter, bCenter);

    if (Vector2DotProduct(difference, resolution) < 0)
    {
        resolution = Vector2Scale(resolution, -1);
    }

    return resolution;
}
