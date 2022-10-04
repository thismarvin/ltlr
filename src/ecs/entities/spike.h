#pragma once

#include "../entity_builder.h"

typedef enum
{
    SPIKE_ROTATE_0,
    SPIKE_ROTATE_90,
    SPIKE_ROTATE_180,
    SPIKE_ROTATE_270,
} SpikeRotation;

EntityBuilder SpikeCreate(f32 x, f32 y, SpikeRotation rotation);
