#include "../../atlas.h"
#include "battery.h"
#include "common.h"
#include <raymath.h>

EntityBuilder BatteryCreate(const f32 x, const f32 y, const AtlasSprite* atlasSprite)
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

    ADD_COMPONENT(CPosition, ((CPosition)
    {
        .value = position,
    }));

    ADD_COMPONENT(CDimension, ((CDimension)
    {
        .width = 14,
        .height = 32,
    }));

    const Rectangle source = (Rectangle)
    {
        .x = atlasSprite->x,
        .y = atlasSprite->y,
        .width = atlasSprite->width,
        .height = atlasSprite->height,
    };

    ADD_COMPONENT(CSprite, ((CSprite)
    {
        .source = source,
        .offset = VECTOR2_ZERO,
        .mirroring = FLIP_NONE,
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

    CKinetic* kinetic = SCENE_GET_COMPONENT_PTR(scene, kinetic, entity);

    kinetic->velocity.y = sinf(ContextGetTotalTime() * 3.0f) * 10.0f;
}
