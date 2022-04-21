#pragma once

#include "common.h"

typedef enum
{
    EVENT_NONE,
    EVENT_CLOUD_PARTICLE,
} EventTag;

typedef struct
{
    usize spawnCount;
} EventCloudParticleInner;

typedef struct
{
    EventTag tag;
    usize entity;
    union
    {
        EventCloudParticleInner cloudParticleInner;
    };
} Event;

void EventCloudParticleInit(Event* self, usize entity, u16 spawnCount);
