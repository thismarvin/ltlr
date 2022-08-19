#include "../components.h"
#include "common.h"
#include "fog.h"
#include <math.h>
#include <raymath.h>

#define FOG_HEIGHT CTX_VIEWPORT_HEIGHT * 2
#define FOG_INITIAL_POSITION (Vector2) \
{ \
    .x = -CTX_VIEWPORT_WIDTH, \
    .y = -(FOG_HEIGHT - CTX_VIEWPORT_HEIGHT) * 0.5f, \
}

static const f32 fogMoveSpeed = 50;
static f32 breathingParticleSpawnTimer = 0;
static f32 breathingParticleSpawnDuration = 3;
static f32 movingParticleSpawnTimer = 0;
static f32 movingParticleSpawnDuration = 0.3f;

EntityBuilder FogCreate(void)
{
    Deque components = DEQUE_OF(Component);

    const u64 tags =
        TAG_NONE
        | TAG_POSITION
        | TAG_DIMENSION
        | TAG_COLOR
        | TAG_KINETIC
        | TAG_SMOOTH
        | TAG_FOG;

    ADD_COMPONENT(CPosition, ((CPosition)
    {
        .value = FOG_INITIAL_POSITION,
    }));

    ADD_COMPONENT(CColor, ((CColor)
    {
        .value = COLOR_BLACK,
    }));

    ADD_COMPONENT(CKinetic, ((CKinetic)
    {
        .velocity = VECTOR2_ZERO,
        .acceleration = VECTOR2_ZERO,
    }));

    ADD_COMPONENT(CSmooth, ((CSmooth)
    {
        .previous = FOG_INITIAL_POSITION,
    }));

    return (EntityBuilder)
    {
        .tags = tags,
        .components = components,
    };
}

static void SpawnBreathingParticles(Scene* scene, const CPosition* position)
{
    if (breathingParticleSpawnTimer >= breathingParticleSpawnDuration)
    {
        static const i32 spawnDomain = 6;
        static const i32 spawnRange = 12;

        static const i32 minSize = 16;
        static const i32 maxSize = 32;

        static const i32 minLifetime = 2;
        static const i32 maxLifetime = 12;

        const i32 spawnCount = GetRandomValue(16, 48);

        for (i32 i = 0; i < spawnCount; i++)
        {
            const Vector2 spawnPosition = (Vector2)
            {
                .x = position->value.x + GetRandomValue(0, spawnDomain),
                .y = position->value.y + (1.0f * i / spawnCount) * FOG_HEIGHT +
                     GetRandomValue(-spawnRange, spawnRange),
            };

            const EntityBuilder particleBuilder = FogBreathingParticleCreate(
                    spawnPosition,
                    GetRandomValue(minSize, maxSize),
                    GetRandomValue(minLifetime, maxLifetime));

            SceneDeferAddEntity(scene, particleBuilder);
        }

        breathingParticleSpawnTimer = 0;
    }
}

static void SpawnMovingParticles(Scene* scene, const CPosition* position)
{
    if (movingParticleSpawnTimer >= movingParticleSpawnDuration)
    {
        if (GetRandomValue(0, 2) == 0)
        {
            static const i32 minSize = 5;
            static const i32 maxSize = 10;

            static const i32 minXSpeed = 20;
            static const i32 maxXSpeed = 30;

            static const i32 minYSpeed = -10;
            static const i32 maxYSpeed = 10;

            static const i32 minLifetime = 1;
            static const i32 maxLifetime = 3;

            const Vector2 spawnPosition = (Vector2)
            {
                .x = position->value.x,
                .y = GetRandomValue(0, CTX_VIEWPORT_HEIGHT),
            };

            const Vector2 velocity = Vector2Create(
                                         kinetic->velocity.x + GetRandomValue(minXSpeed, maxXSpeed),
                                         GetRandomValue(minYSpeed, maxYSpeed));

            const EntityBuilder movingBuilder = FogMovingParticleCreate(
                                                    spawnPosition,
                                                    velocity,
                                                    GetRandomValue(minSize, maxSize),
                                                    GetRandomValue(minLifetime, maxLifetime));

            SceneDeferAddEntity(scene, movingBuilder);
        }
    }
}

void FogUpdate(Scene* scene, const usize entity)
{
    const u64 dependencies = TAG_FOG | TAG_POSITION | TAG_KINETIC;

    if (!SceneEntityHasDependencies(scene, entity, dependencies))
    {
        return;
    }

    assert(SceneEntityHasDependencies(scene, scene->player, TAG_POSITION));

    const CPosition* playerPosition = SCENE_GET_COMPONENT_PTR(scene, playerPosition, scene->player);
    CPosition* position = SCENE_GET_COMPONENT_PTR(scene, position, entity);
    CKinetic* kinetic = SCENE_GET_COMPONENT_PTR(scene, kinetic, entity);

    const bool hasNotMoved = kinetic->velocity.x == 0;
    
    if (hasNotMoved && playerPosition->value.x < CTX_VIEWPORT_WIDTH)
    {
        return;
    }

    kinetic->velocity = (Vector2)
    {
        .x = fogMoveSpeed,
        .y = cosf(ContextGetTotalTime()) * 32,
    };

    breathingParticleSpawnTimer += CTX_DT;
    movingParticleSpawnTimer += CTX_DT;

    SpawnBreathingParticles(scene, position);
    SpawnMovingParticles(scene, position);
}

void FogDraw(const Scene* scene, const usize entity)
{
    const u64 dependencies = TAG_FOG | TAG_POSITION | TAG_COLOR | TAG_SMOOTH;

    if (!SceneEntityHasDependencies(scene, entity, dependencies))
    {
        return;
    }

    const CPosition* position = SCENE_GET_COMPONENT_PTR(scene, position, entity);
    const CColor* color = SCENE_GET_COMPONENT_PTR(scene, color, entity);
    const CSmooth* smooth = SCENE_GET_COMPONENT_PTR(scene, smooth, entity);

    const Vector2 interpolated = Vector2Lerp(smooth->previous, position->value, ContextGetAlpha());

    DrawRectangle(interpolated.x - CTX_VIEWPORT_WIDTH * 2, interpolated.y, CTX_VIEWPORT_WIDTH * 2,
                  dimension->height, color->value);

    Vector2 currentCenter = Vector2Create(interpolated.x, interpolated.y);

    const f32 radius = 32.0f;

    while (currentCenter.y <= position->value.y + dimension->height)
    {
        DrawCircleV(currentCenter, radius, color->value);
        currentCenter.y += radius * 1.5f;
    }
}
