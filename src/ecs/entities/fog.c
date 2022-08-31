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

#define FOG_LUMP_TOTAL 8

static const f32 fogMoveSpeed = 50;
static const f32 movingParticleSpawnDuration = 0.05f;
static f32 movingParticleSpawnTimer = movingParticleSpawnDuration;

static const f32 baseRadius = 40.0f;
static const f32 lumpSpacing = baseRadius * 1.25f;
static f32 lumpRadii[FOG_LUMP_TOTAL];
static f32 lumpTargetRadii[FOG_LUMP_TOTAL];
static u8 breathingPhase = 0;
static f32 breathingPhaseTimer = 0;
static f32 breathingPhaseDuration = 1.0f;

EntityBuilder FogCreate(void)
{
    for (usize i = 0; i < FOG_LUMP_TOTAL; ++i)
    {
        lumpRadii[i] = baseRadius;
        lumpTargetRadii[i] = baseRadius;
    }

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

static void SpawnMovingParticles(Scene* scene, const CPosition* position, const CKinetic* kinetic)
{
    if (movingParticleSpawnTimer >= movingParticleSpawnDuration)
    {
        if (GetRandomValue(0, 2) == 0)
        {
            static const i32 minSize = 3;
            static const i32 maxSize = 6;

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

            const EntityBuilder movingBuilder = FogParticleCreate(
                                                    spawnPosition,
                                                    velocity,
                                                    GetRandomValue(minSize, maxSize),
                                                    GetRandomValue(minLifetime, maxLifetime));

            SceneDeferAddEntity(scene, movingBuilder);
        }

        movingParticleSpawnTimer = 0;
    }
}

static void ShiftBreathingPhase()
{
    switch (breathingPhase)
    {
        case 0:
        {
            for (usize i = 0; i < FOG_LUMP_TOTAL; ++i)
            {
                lumpTargetRadii[i] = baseRadius;
            }

            break;
        }

        case 1:
        {
            for (usize i = 0; i < FOG_LUMP_TOTAL; ++i)
            {
                if (i % 2 == 0)
                {
                    lumpTargetRadii[i] = baseRadius;
                }
                else
                {
                    const f32 radiusModifer = 1 - (GetRandomValue(-25, 25) / 100.0f);
                    lumpTargetRadii[i] = baseRadius * radiusModifer;
                }
            }

            break;
        }

        case 2:
        {
            for (usize i = 0; i < FOG_LUMP_TOTAL; ++i)
            {
                if (i % 2 == 1)
                {
                    lumpTargetRadii[i] = baseRadius;
                }
                else
                {
                    const f32 radiusModifer = 0.75f;
                    lumpTargetRadii[i] = baseRadius * radiusModifer;
                }
            }

            break;
        }

        case 3:
        {
            for (usize i = 0; i < FOG_LUMP_TOTAL; ++i)
            {
                if (i % 2 == 0)
                {
                    lumpTargetRadii[i] = baseRadius;
                }
                else
                {
                    const f32 radiusModifer = 1 - (GetRandomValue(-25, 25) / 100.0f);
                    lumpTargetRadii[i] = baseRadius * radiusModifer;
                }
            }

            break;
        }

        case 4:
        {
            for (usize i = 0; i < FOG_LUMP_TOTAL; ++i)
            {
                if (i % 2 == 1)
                {
                    lumpTargetRadii[i] = baseRadius;
                }
                else
                {
                    const f32 radiusModifer = 0.75f;
                    lumpTargetRadii[i] = baseRadius * radiusModifer;
                }
            }

            break;
        }
    }

    breathingPhase = (breathingPhase % 5) + 1;
    breathingPhaseTimer = 0;
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
        .y = cosf(ContextGetTotalTime() * 0.5f) * 16,
    };

    movingParticleSpawnTimer += CTX_DT;

    SpawnMovingParticles(scene, position, kinetic);

    // Smooth phase transitioning logic for breathing.
    {
        static const f32 transitionIncrement = 0.1f;
        static const f32 transitionLeeway = 0.5f;

        for (usize i = 0; i < FOG_LUMP_TOTAL; ++i)
        {
            if (fabs(lumpRadii[i] - lumpTargetRadii[i]) <= transitionLeeway)
            {
                continue;
            }

            if (lumpRadii[i] < lumpTargetRadii[i])
            {
                lumpRadii[i] += transitionIncrement;
            }
            else
            {
                lumpRadii[i] -= transitionIncrement;
            }
        }

        breathingPhaseTimer += CTX_DT;

        if (breathingPhaseTimer >= breathingPhaseDuration)
        {
            ShiftBreathingPhase();
        }
    }
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

    const f32 radiusPadding = sinf(ContextGetTotalTime() * 5) * 4;

    for (usize lumpCount = 0; lumpCount < FOG_LUMP_TOTAL; ++lumpCount)
    {
        const f32 radius = lumpRadii[lumpCount] + radiusPadding;
        const Vector2 center = Vector2Create(interpolated.x, interpolated.y + (lumpSpacing * lumpCount));
        DrawCircleV(center, radius * 1.1f, COLOR_WHITE);
    }

    for (usize lumpCount = 0; lumpCount < FOG_LUMP_TOTAL; ++lumpCount)
    {
        const f32 radius = lumpRadii[lumpCount] + radiusPadding;
        const Vector2 center = Vector2Create(interpolated.x, interpolated.y + (lumpSpacing * lumpCount));
        DrawCircleV(center, radius, COLOR_BLACK);
    }

    DrawRectangle(interpolated.x - CTX_VIEWPORT_WIDTH * 2, interpolated.y, CTX_VIEWPORT_WIDTH * 2,
                  FOG_HEIGHT, color->value);
}
