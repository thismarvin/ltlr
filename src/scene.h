#pragma once

#include "common.h"
#include "components.h"
#include "events.h"
#include "input.h"
#include "level_segment.h"

#define MAX_ENTITIES 1024

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
    CFleeting fleetings[MAX_ENTITIES];
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
    usize player;
    Rectangle bounds;
    Texture2D atlas;
    LevelSegment* segments;
    usize segmentsLength;
    Rectangle trueResolution;
    Rectangle renderResolution;
    RenderTexture2D backgroundLayer;
    RenderTexture2D targetLayer;
    RenderTexture2D foregroundLayer;
    // TODO(thismarvin): Should this exist in Scene?
    InputHandler input;
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
void SceneDraw(Scene* self);
void SceneReset(Scene* self);
void SceneDestroy(Scene* self);

#define GET_COMPONENT(mValue, mEntity) _Generic((mValue), \
    const CPosition*: &SceneGetComponents(scene)->positions[mEntity], \
    CPosition*: &SceneGetComponents(scene)->positions[mEntity], \
    const CDimension*: &SceneGetComponents(scene)->dimensions[mEntity], \
    CDimension*: &SceneGetComponents(scene)->dimensions[mEntity], \
    const CColor*: &SceneGetComponents(scene)->colors[mEntity], \
    CColor*: &SceneGetComponents(scene)->colors[mEntity], \
    const CSprite*: &SceneGetComponents(scene)->sprites[mEntity], \
    CSprite*: &SceneGetComponents(scene)->sprites[mEntity], \
    const CKinetic*: &SceneGetComponents(scene)->kinetics[mEntity], \
    CKinetic*: &SceneGetComponents(scene)->kinetics[mEntity], \
    const CSmooth*: &SceneGetComponents(scene)->smooths[mEntity], \
    CSmooth*: &SceneGetComponents(scene)->smooths[mEntity], \
    const CCollider*: &SceneGetComponents(scene)->colliders[mEntity], \
    CCollider*: &SceneGetComponents(scene)->colliders[mEntity], \
    const CPlayer*: &SceneGetComponents(scene)->players[mEntity], \
    CPlayer*: &SceneGetComponents(scene)->players[mEntity], \
    const CMortal*: &SceneGetComponents(scene)->mortals[mEntity], \
    CMortal*: &SceneGetComponents(scene)->mortals[mEntity], \
    const CDamage*: &SceneGetComponents(scene)->damages[mEntity], \
    CDamage*: &SceneGetComponents(scene)->damages[mEntity], \
    const CFleeting*: &SceneGetComponents(scene)->fleetings[mEntity], \
    CFleeting*: &SceneGetComponents(scene)->fleetings[mEntity])
