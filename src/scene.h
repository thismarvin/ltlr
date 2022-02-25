#pragma once

#include "common.h"
#include "components.h"
#include "level_segment.h"

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
    CPlayer players[MAX_ENTITIES];
} Components;

typedef struct
{
    Components components;
    usize nextEntity;
    usize nextFreeSlot;
    usize freeSlots[MAX_ENTITIES];
    bool debugging;
    Camera2D camera;
    usize player;
    Rectangle bounds;
    LevelSegment* segments;
    usize segmentsLength;
} Scene;

void SceneInit(Scene* self);
usize SceneAllocateEntity(Scene* self);
void SceneDeallocateEntity(Scene* self, usize entity);
usize SceneGetEntityCount(Scene* self);
void SceneUpdate(Scene* self);
void SceneDraw(Scene* self, Texture2D* atlas);
void SceneDestroy(Scene* self);
