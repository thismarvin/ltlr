#pragma once

#include "common.h"

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

void RaiseSpawnCloudParticleEvent(const EventCloudParticleParams* params);
