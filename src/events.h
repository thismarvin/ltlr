#pragma once

#include "common.h"

#define MAX_EVENTS 64

typedef enum
{
    EVENT_NONE,
    EVENT_COLLISION,
    EVENT_DAMAGE,
    EVENT_CLOUD_PARTICLE,
} EventTag;

typedef struct
{
    usize otherEntity;
} EventCollisionInner;

typedef struct
{
    usize otherEntity;
} EventDamageInner;

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
        EventDamageInner damageInner;
        EventCloudParticleInner  cloudParticleInner;
    };
} Event;

void EventCollisionInit(Event* self, const usize entity, const usize otherEntity);
void EventDamageInit(Event* self, const usize entity, const usize otherEntity);
void EventCloudParticleInit(Event* self, const usize entity, const u16 spawnCount);
