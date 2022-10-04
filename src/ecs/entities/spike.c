#include "../components.h"
#include "common.h"
#include "spike.h"

EntityBuilder SpikeCreate(const f32 x, const f32 y, const SpikeRotation rotation)
{
    Deque components = DEQUE_OF(Component);

    const u64 tags =
        TAG_NONE
        | TAG_POSITION
        | TAG_DIMENSION
        | TAG_COLLIDER
        | TAG_DAMAGE
        | TAG_SPRITE;

    Vector2 position;
    Rectangle intramural;
    Sprite spriteType;

    switch (rotation)
    {
        default:
        case SPIKE_ROTATE_0:
        {
            position = Vector2Create(x + 2, y + 13);
            intramural = (Rectangle)
            {
                2, 13, 12, 3
            };
            spriteType = SPRITE_SPIKE_0000;
            break;
        }

        case SPIKE_ROTATE_90:
        {
            position = Vector2Create(x, y + 2);
            intramural = (Rectangle)
            {
                0, 2, 3, 12
            };
            spriteType = SPRITE_SPIKE_0001;
            break;
        }

        case SPIKE_ROTATE_180:
        {
            position = Vector2Create(x + 2, y);
            intramural = (Rectangle)
            {
                2, 0, 12, 3
            };
            spriteType = SPRITE_SPIKE_0002;
            break;
        }

        case SPIKE_ROTATE_270:
        {
            position = Vector2Create(x + 13, y + 2);
            intramural = (Rectangle)
            {
                13, 2, 3, 12
            };
            spriteType = SPRITE_SPIKE_0003;
            break;
        }
    }

    ADD_COMPONENT(CPosition, ((CPosition)
    {
        .value = position,
    }));

    ADD_COMPONENT(CSprite, ((CSprite)
    {
        .intramural = intramural,
        .reflection = REFLECTION_NONE,
        .type = spriteType,
    }));

    ADD_COMPONENT(CDimension, ((CDimension)
    {
        .width = intramural.width,
        .height = intramural.height,
    }));

    ADD_COMPONENT(CCollider, ((CCollider)
    {
        .resolutionSchema = RESOLVE_NONE,
        .layer = LAYER_LETHAL,
        .mask = LAYER_NONE,
        .onCollision = OnCollisionNoop,
        .onResolution = OnResolutionNoop,
    }));

    ADD_COMPONENT(CDamage, ((CDamage)
    {
        .value = 1,
    }));

    return (EntityBuilder)
    {
        .tags = tags,
        .components = components,
    };
}
