#include "spike.h"

void SpikeCreate(Scene* scene, const void* params)
{
    const SpikeBuilder* builder = params;

    Vector2 position;
    Rectangle intramural;
    Sprite spriteType;

    switch (builder->rotation)
    {
        default:
        case SPIKE_ROTATE_0:
        {
            position = Vector2Create(builder->x + 2, builder->y + 13);
            intramural = (Rectangle)
            {
                2, 13, 12, 3
            };
            spriteType = SPRITE_SPIKE_0000;
            break;
        }

        case SPIKE_ROTATE_90:
        {
            position = Vector2Create(builder->x, builder->y + 2);
            intramural = (Rectangle)
            {
                0, 2, 3, 12
            };
            spriteType = SPRITE_SPIKE_0001;
            break;
        }

        case SPIKE_ROTATE_180:
        {
            position = Vector2Create(builder->x + 2, builder->y);
            intramural = (Rectangle)
            {
                2, 0, 12, 3
            };
            spriteType = SPRITE_SPIKE_0002;
            break;
        }

        case SPIKE_ROTATE_270:
        {
            position = Vector2Create(builder->x + 13, builder->y + 2);
            intramural = (Rectangle)
            {
                13, 2, 3, 12
            };
            spriteType = SPRITE_SPIKE_0003;
            break;
        }
    }

    scene->components.tags[builder->entity] =
        TAG_NONE
        | TAG_POSITION
        | TAG_DIMENSION
        | TAG_COLLIDER
        | TAG_DAMAGE
        | TAG_SPRITE;

    scene->components.positions[builder->entity] = (CPosition)
    {
        .value = position,
    };

    scene->components.sprites[builder->entity] = (CSprite)
    {
        .intramural = intramural,
        .reflection = REFLECTION_NONE,
        .type = spriteType,
    };

    scene->components.dimensions[builder->entity] = (CDimension)
    {
        .width = intramural.width,
        .height = intramural.height,
    };

    scene->components.colliders[builder->entity] = (CCollider)
    {
        .resolutionSchema = RESOLVE_NONE,
        .layer = LAYER_LETHAL,
        .mask = LAYER_NONE,
        .onResolution = OnResolutionNoop,
        .onCollision = OnCollisionNoop,
    };

    scene->components.damages[builder->entity] = (CDamage)
    {
        .value = 1,
    };
}
