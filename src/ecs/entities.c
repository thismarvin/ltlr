#include "../context.h"
#include "../geometry/collider.h"
#include "../scene.h"
#include "components.h"
#include "entities.h"
#include <assert.h>
#include <raymath.h>

#define ADD_COMPONENT(mType, mValue) DEQUE_PUSH_FRONT(&components, Component, ComponentCreate##mType(mValue))

// TODO(thismarvin): We need some of the macros from systems.c...
#define GET_COMPONENT(mValue, mEntity) SCENE_GET_COMPONENT_PTR(params->scene, mEntity, mValue)
#define ENTITY_HAS_DEPS(mEntity, mDependencies) ((params->scene->components.tags[mEntity] & (mDependencies)) == (mDependencies))

static void ApplyResolutionPerfectly
(
    CPosition* position,
    const Rectangle aabb,
    const Rectangle otherAabb,
    const Vector2 resolution
)
{
    if (resolution.x < 0)
    {
        position->value.x = RectangleLeft(otherAabb) - aabb.width;
    }
    else if (resolution.x > 0)
    {
        position->value.x = RectangleRight(otherAabb);
    }

    if (resolution.y < 0)
    {
        position->value.y = RectangleTop(otherAabb) - aabb.height;
    }
    else if (resolution.y > 0)
    {
        position->value.y = RectangleBottom(otherAabb);
    }
}

// TODO(thismarvin): The collider no longer scales...
static OnCollisionResult CloudParticleOnCollision(const OnCollisionParams* params)
{
    assert(ENTITY_HAS_DEPS(params->entity, TAG_POSITION));

    CPosition* position = GET_COMPONENT(position, params->entity);

    // If the aabb is completely within another collider then remove it.
    if (params->overlap.width >= params->aabb.width && params->overlap.height >= params->aabb.height)
    {
        SceneDeferDeallocateEntity(params->scene, params->entity);

        return ON_COLLISION_RESULT_NONE;
    }

    // Resolve collision.
    ApplyResolutionPerfectly(position, params->aabb, params->otherAabb, params->resolution);

    return (OnCollisionResult)
    {
        .xAxisResolved = params->resolution.x != 0,
        .yAxisResolved = params->resolution.y != 0,
    };
}

EntityBuilder ECreateBlock(const f32 x, const f32 y, const f32 width, const f32 height)
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

EntityBuilder ECreateCloudParticle
(
    const f32 centerX,
    const f32 centerY,
    const Vector2 direction
)
{
    Deque components = DEQUE_OF(Component);

    const u64 tags =
        TAG_NONE
        | TAG_POSITION
        | TAG_DIMENSION
        | TAG_COLOR
        | TAG_KINETIC
        | TAG_SMOOTH
        | TAG_COLLIDER
        | TAG_FLEETING;

    f32 radius = 1;

    if (GetRandomValue(1, 100) < 25)
    {
        radius = (f32) GetRandomValue(4, 5);
    }
    else
    {
        radius = (f32)GetRandomValue(1, 3);
    }

    Vector2 position = Vector2Create(centerX - radius, centerY - radius);

    ADD_COMPONENT(CPosition, ((CPosition)
    {
        .value = position,
    }));

    ADD_COMPONENT(CDimension, ((CDimension)
    {
        .width = radius * 2,
        .height = radius * 2,
    }));

    ADD_COMPONENT(CColor, ((CColor)
    {
        .value = COLOR_WHITE,
    }));

    f32 speed = (f32)GetRandomValue(5, 15);

    ADD_COMPONENT(CKinetic, ((CKinetic)
    {
        .velocity = Vector2Scale(direction, speed),
        .acceleration = Vector2Create(0, 15),
    }));

    ADD_COMPONENT(CSmooth, ((CSmooth)
    {
        .previous = position,
    }));

    ADD_COMPONENT(CCollider, ((CCollider)
    {
        .layer = LAYER_NONE,
        .mask = LAYER_ALL,
        .onCollision = CloudParticleOnCollision,
    }));

    f32 lifetime = MIN(1.0f, (f32)GetRandomValue(1, 100) * 0.03f);

    ADD_COMPONENT(CFleeting, ((CFleeting)
    {
        .lifetime = lifetime,
        .age = 0,
    }));

    return (EntityBuilder)
    {
        .tags = tags,
        .components = components,
    };
}
