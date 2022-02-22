#include "common.h"
#include <stdio.h>

Vector2 Vector2Create(f32 x, f32 y)
{
    return (Vector2)
    {
        .x = x,
        .y = y,
    };
}

void DebugVector2(Vector2 value)
{
    printf("(%f, %f)\n", value.x, value.y);
}
