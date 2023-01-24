#pragma once

#include "common.h"

typedef struct
{
    usize entity;
    f32 x;
    f32 y;
} SolarPanelBuilder;

void SolarPanelCreate(Scene* scene, const void* params);
