#pragma once

#include "common.h"
#include "components.h"
#include "events.h"
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
    CPlayer players[MAX_ENTITIES];
    CMortal mortals[MAX_ENTITIES];
    CDamage damages[MAX_ENTITIES];
} Components;

typedef struct
{
    usize nextEntity;
    usize nextFreeSlot;
    usize freeSlots[MAX_ENTITIES];
} EntityManager;

typedef struct
{
    Event events[MAX_EVENTS];
    usize nextEvent;
    usize nextFreeSlot;
    usize freeSlots[MAX_ENTITIES];
} EventManager;

typedef struct
{
    Components components;
    EntityManager entityManager;
    EventManager eventManager;
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
usize SceneGetEventCount(Scene* self);
void SceneRaiseEvent(Scene* self, const Event* event);
void SceneConsumeEvent(Scene* self, usize eventIndex);
void SceneUpdate(Scene* self);
void SceneDraw(Scene* self, Texture2D* atlas);
void SceneReset(Scene* self);
void SceneDestroy(Scene* self);
