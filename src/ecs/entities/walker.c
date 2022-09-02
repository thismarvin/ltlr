#include "../../animation.h"
#include "common.h"
#include "walker.h"

static OnResolutionResult WalkerOnResolution(const OnResolutionParams* params)
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

    return (OnResolutionResult)
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
        | TAG_ANIMATION
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
        .width = 18,
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
        .mirroring = FLIP_NONE,
    }));

    // TODO(thismarvin): Add CAnimationFromWalkerIdle() somewhere.
    ADD_COMPONENT(CAnimation, ((CAnimation)
    {
        .frameTimer = 0,
        .frameDuration = ANIMATION_WALKER_IDLE_FRAME_DURATION,
        .offset = (Vector2) { -15, 0 },
        .mirroring = FLIP_NONE,
        .frame = 0,
        .length = ANIMATION_WALKER_IDLE_LENGTH,
        .handle = ANIMATION_WALKER_IDLE,
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
        .resolutionSchema = RESOLVE_ALL,
        .layer = LAYER_LETHAL,
        .mask = LAYER_TERRAIN | LAYER_LETHAL,
        .onCollision = OnCollisionNoop,
        .onResolution = WalkerOnResolution,
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
