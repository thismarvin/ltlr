#include "common.h"

EntityBuilder BlockCreate(const f32 x, const f32 y, const f32 width, const f32 height)
{
    Deque components = DEQUE_OF(Component);

    const u64 tags =
        TAG_NONE
        | TAG_POSITION
        | TAG_DIMENSION
        | TAG_COLLIDER;

    Vector2 position = Vector2Create(x, y);

    ADD_COMPONENT(CPosition, ((CPosition)
    {
        .value = position,
    }));

    ADD_COMPONENT(CDimension, ((CDimension)
    {
        .width = width,
        .height = height,
    }));

    ADD_COMPONENT(CCollider, ((CCollider)
    {
        .layer = LAYER_ALL,
        .mask = LAYER_NONE,
    }));

    return (EntityBuilder)
    {
        .tags = tags,
        .components = components,
    };
}
