#pragma once

#include "common.h"

typedef struct
{
    usize entity;
} LakituBuilder;

void LakituCreate(Scene* scene, const void* params);
