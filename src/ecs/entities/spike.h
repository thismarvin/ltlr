#pragma once

#include "common.h"

typedef enum
{
    SPIKE_ROTATE_0,
    SPIKE_ROTATE_90,
    SPIKE_ROTATE_180,
    SPIKE_ROTATE_270,
} SpikeRotation;

typedef struct
{
    usize entity;
    f32 x;
    f32 y;
    SpikeRotation rotation;
} SpikeBuilder;

void SpikeCreate(Scene* scene, const void* params);
