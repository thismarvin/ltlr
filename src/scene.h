#pragma once

#include "./collections/deque.h"
#include "./ecs/command.h"
#include "./ecs/components.h"
#include "./ecs/entities.h"
#include "atlas.h"
#include "common.h"
#include "fader.h"
#include "input.h"
#include "level.h"

#define MAX_ENTITIES 1024

#define MAX_SCORE_DIGITS (6 + 1)
#define MAX_SCORE (999999)

typedef enum
{
    SCENE_STATE_MENU,
    SCENE_STATE_ACTION,
} SceneState;

typedef enum
{
    DIRECTOR_STATE_ENTRANCE,
    DIRECTOR_STATE_SUPERVISE,
    DIRECTOR_STATE_EXIT,
} DirectorState;

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
    SceneState state;
    Components components;
    EntityManager m_entityManager;
    bool debugging;
    u32 score;
    f32 scoreBufferTimerDuration;
    f32 scoreBufferTimer;
    i32 scoreBuffer;
    char scoreString[MAX_SCORE_DIGITS];
    usize player;
    usize fog;
    usize lakitu;
    Rectangle bounds;
    Atlas atlas;
    Level level;
    DirectorState director;
    Fader fader;
    Rectangle renderResolution;
    RenderTexture2D rootLayer;
    RenderTexture2D backgroundLayer;
    RenderTexture2D targetLayer;
    RenderTexture2D foregroundLayer;
    RenderTexture2D interfaceLayer;
    RenderTexture2D transitionLayer;
    RenderTexture2D debugLayer;
    InputProfile defaultMenuProfile;
    InputProfile defaultActionProfile;
    InputHandler input;
    // `Deque<Command>`
    Deque commands;
    bool resetRequested;
    Vector2 actionCameraPosition;
    RenderTexture2D treeTexture;
    // `Deque<Vector2>`
    Deque treePositionsBack;
    // `Deque<Vector2>`
    Deque treePositionsFront;
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
void SceneIncrementScore(Scene* self, u32 value);
void SceneUpdate(Scene* self);
void SceneDraw(Scene* self);
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
