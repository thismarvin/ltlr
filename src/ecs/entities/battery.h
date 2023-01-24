#pragma once

#include "common.h"

typedef struct
{
    usize entity;
    f32 x;
    f32 y;
} BatteryBuilder;

void BatteryCreate(Scene* scene, const void* params);

void BatteryUpdate(Scene* scene, usize entity);
