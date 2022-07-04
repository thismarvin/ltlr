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
// TODO(austin0209): move this somewhere else maybe?
static f32 particleSpawnTimer = 0;
static f32 particleSpawnDuration = 3;

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

    ADD_COMPONENT(CDimension, ((CDimension)
    {
        .width = 32,
        .height = FOG_HEIGHT,
    }));

    ADD_COMPONENT(CColor, ((CColor)
    {
        .value = COLOR_BLACK,
    }));

    ADD_COMPONENT(CKinetic, ((CKinetic)
    {
        .velocity = Vector2Create(fogMoveSpeed, 0),
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

    if (position->value.x < 0 && playerPosition->value.x < CTX_VIEWPORT_WIDTH)
    {
        kinetic->velocity = VECTOR2_ZERO;
        return;
    }

    kinetic->velocity = (Vector2)
    {
        .x = fogMoveSpeed,
        .y = cosf(position->value.x * CTX_DT) * 32,
    };

    particleSpawnTimer += CTX_DT;

    if (particleSpawnTimer >= particleSpawnDuration)
    {
        static const i32 spawnCount = 32;
        static const i32 spawnDomain = 6;
        static const i32 spawnRange = 12;

        static const i32 minSize = 16;
        static const i32 maxSize = 32;

        static const i32 minLifetime = 2;
        static const i32 maxLifetime = 12;

        for (i32 i = 0; i < spawnCount; i++)
        {
            const Vector2 spawnPosition = (Vector2)
            {
                .x = position->value.x + GetRandomValue(0, spawnDomain),
                .y = (1.0f * i / spawnCount) * FOG_HEIGHT +
                     GetRandomValue(-spawnRange, spawnRange),
            };

            const EntityBuilder particleBuilder = FogBreathingParticleCreate(
                    spawnPosition,
                    GetRandomValue(minSize, maxSize),
                    GetRandomValue(minLifetime, maxLifetime));

            SceneDeferAddEntity(scene, particleBuilder);
        }

        particleSpawnTimer = 0;
    }
}

void FogDraw(const Scene* scene, const usize entity)
{
    u64 dependencies = TAG_FOG | TAG_POSITION | TAG_DIMENSION | TAG_COLOR | TAG_SMOOTH;

    if (!SceneEntityHasDependencies(scene, entity, dependencies))
    {
        return;
    }

    const CPosition* position = SCENE_GET_COMPONENT_PTR(scene, position, entity);
    const CDimension* dimension = SCENE_GET_COMPONENT_PTR(scene, dimension, entity);
    const CColor* color = SCENE_GET_COMPONENT_PTR(scene, color, entity);
    const CSmooth* smooth = SCENE_GET_COMPONENT_PTR(scene, smooth, entity);

    const Vector2 interpolated = Vector2Lerp(smooth->previous, position->value, ContextGetAlpha());

    DrawRectangle(interpolated.x - CTX_VIEWPORT_WIDTH * 2, interpolated.y, CTX_VIEWPORT_WIDTH * 2,
                  dimension->height, color->value);

    Vector2 currentCenter = Vector2Create(interpolated.x + dimension->width * 0.5f, interpolated.y);

    const f32 radius = 32.0f;

    while (currentCenter.y <= position->value.y + dimension->height)
    {
        DrawCircleV(currentCenter, radius, color->value);
        currentCenter.y += radius * 1.5f;
    }
}
