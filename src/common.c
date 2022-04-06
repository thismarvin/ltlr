#include "common.h"

Vector2 Vector2Create(const f32 x, const f32 y)
{
    return (Vector2)
    {
        .x = x,
        .y = y,
    };
}

f32 RectangleLeft(const Rectangle rectangle)
{
    return rectangle.x;
}

f32 RectangleRight(const Rectangle rectangle)
{
    return rectangle.x + rectangle.width;
}

f32 RectangleBottom(const Rectangle rectangle)
{
    return rectangle.y + rectangle.height;
}

f32 RectangleTop(const Rectangle rectangle)
{
    return rectangle.y;
}
