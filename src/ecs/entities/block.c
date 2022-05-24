#include "block.h"
#include "common.h"

static void BlockOnCollision() {}

static OnResolutionResult BlockOnResolution(const OnResolutionParams* params)
{
    return (OnResolutionResult)
    {
        .aabb = params->aabb,
    };
}

EntityBuilder BlockCreate(const Rectangle aabb, const u8 resolutionSchema, const u64 layer)
{
    Deque components = DEQUE_OF(Component);

    const u64 tags =
        TAG_NONE
        | TAG_POSITION
        | TAG_DIMENSION
        | TAG_COLLIDER;

    Vector2 position = Vector2Create(aabb.x, aabb.y);

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
        .onCollision = BlockOnCollision,
        .onResolution = BlockOnResolution,
    }));

    return (EntityBuilder)
    {
        .tags = tags,
        .components = components,
    };
}
