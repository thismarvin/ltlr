#pragma once

#include "common.h"
#include "deque.h"

typedef void (*OnRaise)(const void*);

typedef struct
{
    Deque listeners;
} EventHandler;

EventHandler EventHandlerCreate();
void EventHandlerSubscribe(EventHandler* self, OnRaise onRaise);
void EventHandlerRaise(const EventHandler* self, const void* arguments);
void EventHandlerDestroy(EventHandler* self);

typedef struct Scene Scene;

typedef struct
{
    Scene* scene;
    usize entity;
    Vector2 anchor;
    Vector2 direction;
    usize spawnCount;
    usize spread;
} EventCloudParticleParams;

void SpawnCloudParticles(const EventCloudParticleParams* params);
