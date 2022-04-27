#include "common.h"

void ApplyResolutionPerfectly
(
    CPosition* position,
    const Rectangle aabb,
    const Rectangle otherAabb,
    const Vector2 resolution
)
{
    if (resolution.x < 0)
    {
        position->value.x = RectangleLeft(otherAabb) - aabb.width;
    }
    else if (resolution.x > 0)
    {
        position->value.x = RectangleRight(otherAabb);
    }

    if (resolution.y < 0)
    {
        position->value.y = RectangleTop(otherAabb) - aabb.height;
    }
    else if (resolution.y > 0)
    {
        position->value.y = RectangleBottom(otherAabb);
    }
}

