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
        | TAG_BATTERY;

    ADD_COMPONENT(CPosition, ((CPosition)
    {
        .value = Vector2Create(x, y),
    }));

    ADD_COMPONENT(CDimension, ((CDimension)
    {
        .width = atlasSprite->width,
        .height = atlasSprite->height,
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
        .offset = Vector2Create(atlasSprite->trimRect.width, atlasSprite->trimRect.height),
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

    return (EntityBuilder)
    {
        .tags = tags,
        .components = components,
    };
}

void BatteryUpdate(Scene* scene, const usize entity)
{
    const u64 dependencies = TAG_BATTERY | TAG_POSITION;

    if (!SceneEntityHasDependencies(scene, entity, dependencies))
    {
        return;
    }

    CPosition* position = SCENE_GET_COMPONENT_PTR(scene, position, entity);

    position->value.y += sinf(ContextGetTotalTime() * 5) * 0.2f;
}
