#pragma once

#include "common.h"

typedef struct
{
    usize entity;
} LakituBuilder;

void LakituBuild(Scene* scene, const void* params);
