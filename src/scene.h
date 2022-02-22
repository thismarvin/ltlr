#pragma once

#include "common.h"
#include "components.h"

#define MAX_ENTITIES 64

typedef struct
{
    u64 tags[MAX_ENTITIES];
    CPosition positions[MAX_ENTITIES];
    CDimension dimensions[MAX_ENTITIES];
    CColor colors[MAX_ENTITIES];
    CSprite sprites[MAX_ENTITIES];
    CKinetic kinetics[MAX_ENTITIES];
    CSmooth smooths[MAX_ENTITIES];
    CCollider colliders[MAX_ENTITIES];
    CBody bodies[MAX_ENTITIES];
} Components;

typedef struct
{
    Components components;
    usize nextEntity;
    usize nextFreeSlot;
    usize freeSlots[MAX_ENTITIES];
} Scene;

void SceneInit(Scene* self);
usize SceneAllocateEntity(Scene* self);
void SceneDeallocateEntity(Scene* self, usize entity);
usize SceneGetEntityCount(Scene* self);
void SceneUpdate(Scene* self);
void SceneDraw(Scene* self, Texture2D* atlas);
