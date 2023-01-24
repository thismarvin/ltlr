#include "battery.h"
#include <math.h>

void BatteryCreate(Scene* scene, const void* params)
{
    const BatteryBuilder* builder = params;

    const Vector2 position = (Vector2)
    {
        .x = builder->x,
        .y = builder->y,
    };
    const Rectangle intramural = (Rectangle)
    {
        .x = 1,
        .y = 0,
        .width = 14,
        .height = 32,
    };

    scene->components.tags[builder->entity] =
        TAG_NONE
        | TAG_POSITION
        | TAG_DIMENSION
        | TAG_SPRITE
        | TAG_COLLIDER
        | TAG_SMOOTH
        | TAG_KINETIC
        | TAG_BATTERY;

    scene->components.positions[builder->entity] = (CPosition)
    {
        .value = position,
    };

    scene->components.dimensions[builder->entity] = (CDimension)
    {
        .width = intramural.width,
        .height = intramural.height,
    };

    scene->components.sprites[builder->entity] = (CSprite)
    {
        .type = SPRITE_BATTERY,
        .intramural = intramural,
        .reflection = REFLECTION_NONE,
    };

    scene->components.colliders[builder->entity] = (CCollider)
    {
        .resolutionSchema = RESOLVE_NONE,
        .layer = LAYER_INTERACTABLE,
        .mask = LAYER_NONE,
        .onCollision = OnCollisionNoop,
        .onResolution = OnResolutionNoop,
    };

    scene->components.smooths[builder->entity] = (CSmooth)
    {
        .previous = position,
    };

    scene->components.kinetics[builder->entity] = (CKinetic)
    {
        .velocity = VECTOR2_ZERO,
        .acceleration = VECTOR2_ZERO,
    };
}

void BatteryUpdate(Scene* scene, const usize entity)
{
    static const u64 dependencies = TAG_BATTERY | TAG_KINETIC;

    if (!SceneEntityHasDependencies(scene, entity, dependencies))
    {
        return;
    }

    CKinetic* kinetic = &scene->components.kinetics[entity];

    kinetic->velocity.y = sinf(ContextGetTotalTime() * 3.0f) * 10.0f;
}
