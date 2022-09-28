#include "block.h"
#include "common.h"

EntityBuilder BlockCreate(const Rectangle aabb, const u8 resolutionSchema, const u64 layer)
{
    Deque components = DEQUE_OF(Component);

    const u64 tags =
        TAG_NONE
        | TAG_POSITION
        | TAG_DIMENSION
        | TAG_COLLIDER;

    const Vector2 position = Vector2Create(aabb.x, aabb.y);

    ADD_COMPONENT(CPosition, ((CPosition)
    {
        .value = position,
    }));

    ADD_COMPONENT(CDimension, ((CDimension)
    {
        .width = aabb.width,
        .height = aabb.height,
    }));

    ADD_COMPONENT(CCollider, ((CCollider)
    {
        .resolutionSchema = resolutionSchema,
        .layer = layer,
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
