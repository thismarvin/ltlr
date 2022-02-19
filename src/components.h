#pragma once

#include "common.h"

extern const u64 tagPosition;
extern const u64 tagDimension;
extern const u64 tagColor;

typedef struct
{
    Vector2 value;
} CPosition;

typedef struct
{
    f32 width;
    f32 height;
} CDimension;

typedef struct
{
    Color value;
} CColor;
