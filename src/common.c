#include "common.h"

Vector2 Vector2Create(f32 x, f32 y)
{
    return (Vector2)
    {
        .x = x,
        .y = y,
    };
}

f32 RectangleLeft(Rectangle rectangle)
{
    return rectangle.x;
}

f32 RectangleRight(Rectangle rectangle)
{
    return rectangle.x + rectangle.width;
}

f32 RectangleBottom(Rectangle rectangle)
{
    return rectangle.y + rectangle.height;
}

f32 RectangleTop(Rectangle rectangle)
{
    return rectangle.y;
}
