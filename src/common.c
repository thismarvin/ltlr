#include "common.h"

Vector2 Vector2Create(const f32 x, const f32 y)
{
    return (Vector2)
    {
        .x = x,
        .y = y,
    };
}

f32 RectangleLeft(const Rectangle self)
{
    return self.x;
}

f32 RectangleRight(const Rectangle self)
{
    return self.x + self.width;
}

f32 RectangleBottom(const Rectangle self)
{
    return self.y + self.height;
}

f32 RectangleTop(const Rectangle self)
{
    return self.y;
}
