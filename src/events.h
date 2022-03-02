#pragma once

#include "common.h"

#define MAX_EVENTS 64

typedef enum
{
    EVENT_NONE,
    EVENT_COLLISION,
    EVENT_DAMAGE
} EventTag;

typedef struct
{
    usize otherEntity;
    Rectangle overlap;
} EventCollisionInner;

typedef struct
{
    usize otherEntity;
} EventDamageInner;

typedef struct
{
    EventTag tag;
    usize entity;
    union
    {
        EventCollisionInner collisionInner;
        EventDamageInner damageInner;
    };
} Event;

void EventCollisionInit(Event* self, usize entity, usize otherEntity, Rectangle overlap);
void EventDamageInit(Event* self, usize entity, usize otherEntity);
