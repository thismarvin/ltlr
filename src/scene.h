#pragma once

#include "./collections/deque.h"
#include "./ecs/command.h"
#include "./ecs/components.h"
#include "./ecs/entities.h"
#include "atlas.h"
#include "common.h"
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
    CAnimation animations[MAX_ENTITIES];
} Components;

typedef struct
{
    // The next available index in the Components struct that has not been used before.
    usize m_nextFreshEntityIndex;
    // A stack of indices in the Components struct that are not currently allocated but were
    // previously deallocated (an entity once resided in these indices).
    // `Deque<usize>`
    Deque m_recycledEntityIndices;
} EntityManager;

struct Scene
{
    Components components;
    EntityManager m_entityManager;
    bool debugging;
    usize player;
    usize fog;
    Rectangle bounds;
    Atlas atlas;
    LevelSegment* segments;
    usize segmentsLength;
    Rectangle trueResolution;
    Rectangle renderResolution;
    RenderTexture2D rootLayer;
    RenderTexture2D backgroundLayer;
    RenderTexture2D targetLayer;
    RenderTexture2D foregroundLayer;
    RenderTexture2D debugLayer;
    InputProfile defaultActionProfile;
    InputHandler input;
    // `Deque<Command>`
    Deque commands;
    bool resetRequested;
};

void SceneInit(Scene* self);
void SceneDeferEnableComponent(Scene* self, usize entity, usize tag);
void SceneDeferDisableComponent(Scene* self, usize entity, usize tag);
// Defer the creation of a given entity (note that the EntityBuilder's Deque will be destroyed)
usize SceneDeferAddEntity(Scene* self, EntityBuilder entityBuilder);
void SceneDeferDeallocateEntity(Scene* self, usize entity);
bool SceneEntityHasDependencies(const Scene* self, usize entity, u64 dependencies);
usize SceneGetEntityCount(const Scene* self);
void SceneSubmitCommand(Scene* self, Command command);
void SceneUpdate(Scene* self);
void SceneDraw(const Scene* self);
void SceneDeferReset(Scene* self);
void SceneDestroy(Scene* self);

#define SCENE_GET_COMPONENT(mScene, mValue, mEntity) _Generic((mValue), \
    CPosition: mScene->components.positions[mEntity], \
    CDimension: mScene->components.dimensions[mEntity], \
    CColor: mScene->components.colors[mEntity], \
    CSprite: mScene->components.sprites[mEntity], \
    CKinetic: mScene->components.kinetics[mEntity], \
    CSmooth: mScene->components.smooths[mEntity], \
    CCollider: mScene->components.colliders[mEntity], \
    CPlayer: mScene->components.players[mEntity], \
    CMortal: mScene->components.mortals[mEntity], \
    CDamage: mScene->components.damages[mEntity], \
    CFleeting: mScene->components.fleetings[mEntity], \
    CAnimation: mScene->components.animations[mEntity])

#define SCENE_GET_COMPONENT_PTR(mScene, mValue, mEntity) _Generic((mValue), \
    const CPosition*: &mScene->components.positions[mEntity], \
    CPosition*: &mScene->components.positions[mEntity], \
    const CDimension*: &mScene->components.dimensions[mEntity], \
    CDimension*: &mScene->components.dimensions[mEntity], \
    const CColor*: &mScene->components.colors[mEntity], \
    CColor*: &mScene->components.colors[mEntity], \
    const CSprite*: &mScene->components.sprites[mEntity], \
    CSprite*: &mScene->components.sprites[mEntity], \
    const CKinetic*: &mScene->components.kinetics[mEntity], \
    CKinetic*: &mScene->components.kinetics[mEntity], \
    const CSmooth*: &mScene->components.smooths[mEntity], \
    CSmooth*: &mScene->components.smooths[mEntity], \
    const CCollider*: &mScene->components.colliders[mEntity], \
    CCollider*: &mScene->components.colliders[mEntity], \
    const CPlayer*: &mScene->components.players[mEntity], \
    CPlayer*: &mScene->components.players[mEntity], \
    const CMortal*: &mScene->components.mortals[mEntity], \
    CMortal*: &mScene->components.mortals[mEntity], \
    const CDamage*: &mScene->components.damages[mEntity], \
    CDamage*: &mScene->components.damages[mEntity], \
    const CFleeting*: &mScene->components.fleetings[mEntity], \
    CFleeting*: &mScene->components.fleetings[mEntity], \
    const CAnimation*: &mScene->components.animations[mEntity], \
    CAnimation*: &mScene->components.animations[mEntity])

#define SCENE_SET_COMPONENT(mScene, mEntity, mValue) \
    SCENE_GET_COMPONENT(mScene, mValue, mEntity) = mValue
