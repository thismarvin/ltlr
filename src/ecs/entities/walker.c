#include "common.h"
#include "walker.h"

static OnCollisionResult WalkerOnCollision(const OnCollisionParams* params)
{
    assert(ENTITY_HAS_DEPS(params->entity, TAG_POSITION | TAG_KINETIC));

    const CPosition* position = GET_COMPONENT(position, params->entity);
    CKinetic* kinetic = GET_COMPONENT(kinetic, params->entity);

    // Resolve collision.
    Rectangle resolvedAabb = ApplyResolutionPerfectly(params->aabb, params->otherAabb,
                             params->resolution);

    // Walk side to side.
    {
        if (params->resolution.x != 0)
        {
            kinetic->velocity.x *= -1;
        }

        if (params->resolution.y != 0)
        {
            kinetic->velocity.y = 0;
        }
    }

    return (OnCollisionResult)
    {
        .aabb = resolvedAabb,
    };
}

EntityBuilder WalkerCreate(const f32 x, const f32 y)
{
    Deque components = DEQUE_OF(Component);

    const u64 tags =
        TAG_NONE
        | TAG_POSITION
        | TAG_DIMENSION
        | TAG_COLOR
        | TAG_SPRITE
        | TAG_KINETIC
        | TAG_SMOOTH
        | TAG_COLLIDER
        | TAG_WALKER
        | TAG_DAMAGE;

    Vector2 position = Vector2Create(x, y);

    ADD_COMPONENT(CPosition, ((CPosition)
    {
        .value = position,
    }));

    ADD_COMPONENT(CDimension, ((CDimension)
    {
        .width = 16,
        .height = 16,
    }));

    ADD_COMPONENT(CColor, ((CColor)
    {
        .value = COLOR_WHITE,
    }));

    ADD_COMPONENT(CSprite, ((CSprite)
    {
        .source = (Rectangle) { 3 * 16, 5 * 16, 16, 16 },
        .offset = VECTOR2_ZERO,
    }));

    ADD_COMPONENT(CKinetic, ((CKinetic)
    {
        .velocity = Vector2Create(50, 0),
        .acceleration = Vector2Create(0, 1000),
    }));

    ADD_COMPONENT(CSmooth, ((CSmooth)
    {
        .previous = position,
    }));

    ADD_COMPONENT(CCollider, ((CCollider)
    {
        .layer = LAYER_ALL,
        .mask = LAYER_ALL,
        .onCollision = WalkerOnCollision,
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

