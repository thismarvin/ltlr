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
Components* SceneGetComponents(Scene* self);
void SceneEnableComponent(Scene* self, usize entity, usize tag);
void SceneDisableComponent(Scene* self, usize entity, usize tag);
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

#define GET_COMPONENT(mValue, mEntity) _Generic((mValue), \
    CPosition: SceneGetComponents(scene)->positions[mEntity], \
    CDimension: SceneGetComponents(scene)->dimensions[mEntity], \
    CColor: SceneGetComponents(scene)->colors[mEntity], \
    CSprite: SceneGetComponents(scene)->sprites[mEntity], \
    CKinetic: SceneGetComponents(scene)->kinetics[mEntity], \
    CSmooth: SceneGetComponents(scene)->smooths[mEntity], \
    CCollider: SceneGetComponents(scene)->colliders[mEntity], \
    CPlayer: SceneGetComponents(scene)->players[mEntity], \
    CMortal: SceneGetComponents(scene)->mortals[mEntity], \
    CDamage: SceneGetComponents(scene)->damages[mEntity])

#define GET_COMPONENT_MUT(mValue, mEntity) _Generic((mValue), \
    CPosition*: &SceneGetComponents(scene)->positions[mEntity], \
    CDimension*: &SceneGetComponents(scene)->dimensions[mEntity], \
    CColor*: &SceneGetComponents(scene)->colors[mEntity], \
    CSprite*: &SceneGetComponents(scene)->sprites[mEntity], \
    CKinetic*: &SceneGetComponents(scene)->kinetics[mEntity], \
    CSmooth*: &SceneGetComponents(scene)->smooths[mEntity], \
    CCollider*: &SceneGetComponents(scene)->colliders[mEntity], \
    CPlayer*: &SceneGetComponents(scene)->players[mEntity], \
    CMortal*: &SceneGetComponents(scene)->mortals[mEntity], \
    CDamage*: &SceneGetComponents(scene)->damages[mEntity])
