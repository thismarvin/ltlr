#include "../../atlas.h"
#include "solar_panel.h"
#include "common.h"
#include <raymath.h>

EntityBuilder SolarPanelCreate(const f32 x, const f32 y, const AtlasSprite* atlasSprite)
{
    Deque components = DEQUE_OF(Component);

    const u64 tags =
        TAG_NONE
        | TAG_POSITION
        | TAG_DIMENSION
        | TAG_SPRITE
        | TAG_COLLIDER
        | TAG_SOLAR_PANEL;

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

    return (EntityBuilder)
    {
        .tags = tags,
        .components = components,
    };
}
