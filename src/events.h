#pragma once

#include "common.h"

typedef enum
{
    EVENT_NONE,
    EVENT_COLLISION,
    EVENT_CLOUD_PARTICLE,
} EventTag;

typedef struct
{
    usize otherEntity;
} EventCollisionInner;

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
        EventCollisionInner collisionInner;
        EventCloudParticleInner cloudParticleInner;
    };
} Event;

void EventCollisionInit(Event* self, usize entity, usize otherEntity);
void EventCloudParticleInit(Event* self, usize entity, u16 spawnCount);
