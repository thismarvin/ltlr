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

// TODO(thismarvin): Ideally the following would be in something like `src/entities/player.c`.

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

static bool PlayerIsVulnerable(const CPlayer* player)
{
    return player->invulnerableTimer >= player->invulnerableDuration;
}

static void PlayerOnDamage(const OnDamageParams* params)
{
    assert(ENTITY_HAS_DEPS(params->entity, TAG_PLAYER | TAG_MORTAL));

    CPlayer* player = GET_COMPONENT(player, params->entity);
    CMortal* mortal = GET_COMPONENT(mortal, params->entity);

    const CDamage* otherDamage = GET_COMPONENT(otherDamage, params->otherEntity);

    if (!PlayerIsVulnerable(player))
    {
        return;
    }

    mortal->hp -= otherDamage->value;
    player->invulnerableTimer = 0;
}

static OnCollisionResult PlayerOnCollision(const OnCollisionParams* params)
{
    assert(ENTITY_HAS_DEPS(params->entity, TAG_PLAYER | TAG_POSITION | TAG_KINETIC | TAG_MORTAL));

    CPlayer* player = GET_COMPONENT(player, params->entity);
    CPosition* position = GET_COMPONENT(position, params->entity);
    CKinetic* kinetic = GET_COMPONENT(kinetic, params->entity);
    const CMortal* mortal = GET_COMPONENT(mortal, params->entity);

    // Collision specific logic that will not resolve the player.
    {
        if (ENTITY_HAS_DEPS(params->otherEntity, TAG_WALKER | TAG_DAMAGE))
        {
            OnDamageParams onDamageParams = (OnDamageParams)
            {
                .scene = params->scene,
                .entity = params->entity,
                .otherEntity = params->otherEntity,
            };

            mortal->onDamage(&onDamageParams);

            return ON_COLLISION_RESULT_NONE;
        }
    }

    // Collision leeway.
    {
        // Check if the player hit its head on the bottom of a collider.
        if (params->resolution.y > 0 && fabsf(params->overlap.width) <= 4)
        {
            if (params->aabb.x < params->otherAabb.x)
            {
                position->value.x = RectangleLeft(params->otherAabb) - params->aabb.width;
            }
            else
            {
                position->value.x = RectangleRight(params->otherAabb);
            }

            return (OnCollisionResult)
            {
                .xAxisResolved = true,
                .yAxisResolved = false,
            };
        }
    }

    // Resolve collision.
    ApplyResolutionPerfectly(position, params->aabb, params->otherAabb, params->resolution);

    // Resolution specific player logic.
    {
        if ((params->resolution.x < 0 && kinetic->velocity.x > 0)
                || (params->resolution.x > 0 && kinetic->velocity.x < 0))
        {
            kinetic->velocity.x = 0;
        }

        if ((params->resolution.y < 0 && kinetic->velocity.y > 0)
                || (params->resolution.y > 0 && kinetic->velocity.y < 0))
        {
            kinetic->velocity.y = 0;
        }

        if (params->resolution.y < 0)
        {
            player->grounded = true;
        }
    }

    return (OnCollisionResult)
    {
        .xAxisResolved = params->resolution.x != 0,
        .yAxisResolved = params->resolution.y != 0,
    };
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

static OnCollisionResult WalkerOnCollision(const OnCollisionParams* params)
{
    assert(ENTITY_HAS_DEPS(params->entity, TAG_POSITION | TAG_KINETIC));

    CPosition* position = GET_COMPONENT(position, params->entity);
    CKinetic* kinetic = GET_COMPONENT(kinetic, params->entity);

    // Resolve collision.
    ApplyResolutionPerfectly(position, params->aabb, params->otherAabb, params->resolution);

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
        .xAxisResolved = params->resolution.x != 0,
        .yAxisResolved = params->resolution.y != 0,
    };
}

EntityBuilder ECreatePlayer(const f32 x, const f32 y)
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
        | TAG_PLAYER
        | TAG_MORTAL;

    Vector2 position = Vector2Create(x, y);

    ADD_COMPONENT(CPosition, ((CPosition)
    {
        .value = position,
    }));

    ADD_COMPONENT(CDimension, ((CDimension)
    {
        .width = 15,
        .height = 35,
    }));

    ADD_COMPONENT(CColor, ((CColor)
    {
        .value = COLOR_WHITE,
    }));

    ADD_COMPONENT(CSprite, ((CSprite)
    {
        .source = (Rectangle) { 16, 0, 32, 48 },
        .offset = Vector2Create(-8, -13),
    }));

    ADD_COMPONENT(CKinetic, ((CKinetic)
    {
        .velocity = VECTOR2_ZERO,
        .acceleration = Vector2Create(0, 1000),
    }));

    ADD_COMPONENT(CSmooth, ((CSmooth)
    {
        .previous = position,
    }));

    ADD_COMPONENT(CCollider, ((CCollider)
    {
        .layer = LAYER_NONE,
        .mask = LAYER_ALL,
        .onCollision = PlayerOnCollision,
    }));

    ADD_COMPONENT(CMortal, ((CMortal)
    {
        .hp = 2,
        .onDamage = PlayerOnDamage,
    }));

    f32 jumpHeight = 16 * 3 + 6;
    f32 jumpDuration = 0.4;

    f32 jumpGravity = (2 * jumpHeight) / (jumpDuration * jumpDuration);
    f32 defaultGravity = jumpGravity * 1.5;

    f32 jumpVelocity = jumpGravity * jumpDuration;

    f32 coyoteDuration = CTX_DT * 6;
    f32 invulnerableDuration = 1.5f;

    ADD_COMPONENT(CPlayer, ((CPlayer)
    {
        .groundedLastFrame = false,
        .grounded = false,
        .coyoteTimer = coyoteDuration,
        .coyoteDuration = coyoteDuration,
        .moveSpeed = 200,
        .jumping = false,
        .dead = false,
        .jumpVelocity = jumpVelocity,
        .jumpGravity = jumpGravity,
        .defaultGravity = defaultGravity,
        .invulnerableTimer = invulnerableDuration,
        .invulnerableDuration = invulnerableDuration,
    }));

    return (EntityBuilder)
    {
        .tags = tags,
        .components = components,
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

EntityBuilder ECreateWalker(const f32 x, const f32 y)
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
