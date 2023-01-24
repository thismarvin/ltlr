#pragma once

#include "common.h"

typedef struct
{
    usize entity;
    Rectangle aabb;
    u8 resolutionSchema;
    u64 layer;
} BlockBuilder;

void BlockCreate(Scene* scene, const void* params);
