#include "../../animation.h"
#include "common.h"
#include "walker.h"

static OnResolutionResult WalkerOnResolution(const OnResolutionParams* params)
{
    static const u64 dependencies = TAG_KINETIC;
    assert(SceneEntityHasDependencies(params->scene, params->entity, dependencies));

    CKinetic* kinetic = &params->scene->components.kinetics[params->entity];

    // Resolve collision.
    const Rectangle resolvedAabb = ApplyResolutionPerfectly(params->aabb, params->otherAabb,
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
        | TAG_ANIMATION
        | TAG_KINETIC
        | TAG_SMOOTH
        | TAG_COLLIDER
        | TAG_WALKER
        | TAG_DAMAGE;

    const Vector2 position = Vector2Create(x, y);
    const Rectangle intramural = (Rectangle)
    {
        .x = 14,
        .y = 0,
        .width = 20,
        .height = 16,
    };

    ADD_COMPONENT(CPosition, ((CPosition)
    {
        .value = position,
    }));

    ADD_COMPONENT(CDimension, ((CDimension)
    {
        .width = intramural.width,
        .height = intramural.height,
    }));

    // TODO(thismarvin): Add CAnimationFromWalkerIdle() somewhere.
    ADD_COMPONENT(CAnimation, ((CAnimation)
    {
        .frameTimer = 0,
        .frameDuration = ANIMATION_WALKER_IDLE_FRAME_DURATION,
        .intramural = intramural,
        .reflection = REFLECTION_NONE,
        .frame = 0,
        .length = ANIMATION_WALKER_IDLE_LENGTH,
        .type = ANIMATION_WALKER_IDLE,
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
        .mask = LAYER_TERRAIN | LAYER_INVISIBLE | LAYER_LETHAL,
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
