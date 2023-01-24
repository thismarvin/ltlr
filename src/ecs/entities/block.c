#include "block.h"

void BlockCreate(Scene* scene, const void* params)
{
    const BlockBuilder* builder = params;

    const Vector2 position = (Vector2)
    {
        .x = builder->aabb.x,
        .y = builder->aabb.y,
    };

    scene->components.tags[builder->entity] =
        TAG_NONE
        | TAG_POSITION
        | TAG_DIMENSION
        | TAG_COLLIDER;

    scene->components.positions[builder->entity] = (CPosition)
    {
        .value = position,
    };

    scene->components.dimensions[builder->entity] = (CDimension)
    {
        .width = builder->aabb.width,
        .height = builder->aabb.height,
    };

    scene->components.colliders[builder->entity] = (CCollider)
    {
        .resolutionSchema = builder->resolutionSchema,
        .layer = builder->layer,
        .mask = LAYER_NONE,
        .onCollision = OnCollisionNoop,
        .onResolution = OnResolutionNoop,
    };
}
