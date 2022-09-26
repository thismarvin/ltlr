#include "../../atlas.h"
#include "solar_panel.h"
#include "common.h"
#include <raymath.h>

EntityBuilder SolarPanelCreate(const f32 x, const f32 y)
{
    Deque components = DEQUE_OF(Component);

    const u64 tags =
        TAG_NONE
        | TAG_POSITION
        | TAG_DIMENSION
        | TAG_SPRITE
        | TAG_COLLIDER
        | TAG_SOLAR_PANEL;

    const Rectangle intramural = (Rectangle)
    {
        .x = 4,
        .y = 8,
        .width = 88,
        .height = 40,
    };

    ADD_COMPONENT(CPosition, ((CPosition)
    {
        .value = Vector2Create(x, y),
    }));

    ADD_COMPONENT(CDimension, ((CDimension)
    {
        .width = intramural.width,
        .height = intramural.height,
    }));

    ADD_COMPONENT(CSprite, ((CSprite)
    {
        .source = (Rectangle) { 217, 4, 88, 40 },
        .intramural = intramural,
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
