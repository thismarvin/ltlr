#include "../../atlas.h"
#include "battery.h"
#include "common.h"
#include <raymath.h>

EntityBuilder BatteryCreate(const f32 x, const f32 y)
{
    Deque components = DEQUE_OF(Component);

    const u64 tags =
        TAG_NONE
        | TAG_POSITION
        | TAG_DIMENSION
        | TAG_SPRITE
        | TAG_COLLIDER
        | TAG_SMOOTH
        | TAG_KINETIC
        | TAG_BATTERY;

    const Vector2 position = Vector2Create(x, y);
    const Rectangle intramural = (Rectangle)
    {
        .x = 1,
        .y = 0,
        .width = 14,
        .height = 32,
    };

    ADD_COMPONENT(CPosition, ((CPosition)
    {
        .value = position,
    }));

    ADD_COMPONENT(CDimension, ((CDimension)
    {
        .width = intramural.width,
        .height = intramural.height,
    }));

    ADD_COMPONENT(CSprite, ((CSprite)
    {
        .type = SPRITE_BATTERY,
        .intramural = intramural,
        .reflection = REFLECTION_NONE,
    }));

    ADD_COMPONENT(CCollider, ((CCollider)
    {
        .resolutionSchema = RESOLVE_NONE,
        .layer = LAYER_INTERACTABLE,
        .mask = LAYER_NONE,
        .onCollision = OnCollisionNoop,
        .onResolution = OnResolutionNoop,
    }));

    ADD_COMPONENT(CSmooth, ((CSmooth)
    {
        .previous = position,
    }));

    ADD_COMPONENT(CKinetic, ((CKinetic)
    {
        .velocity = VECTOR2_ZERO,
        .acceleration = VECTOR2_ZERO,
    }));

    return (EntityBuilder)
    {
        .tags = tags,
        .components = components,
    };
}

void BatteryUpdate(Scene* scene, const usize entity)
{
    const u64 dependencies = TAG_BATTERY | TAG_KINETIC;

    if (!SceneEntityHasDependencies(scene, entity, dependencies))
    {
        return;
    }

    CKinetic* kinetic = &scene->components.kinetics[entity];

    kinetic->velocity.y = sinf(ContextGetTotalTime() * 3.0f) * 10.0f;
}
