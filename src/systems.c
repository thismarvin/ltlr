#include "collider.h"
#include "components.h"
#include "context.h"
#include "entities.h"
#include "events.h"
#include "raymath.h"
#include "systems.h"
#include <assert.h>

#define REQUIRE_DEPS(mDependencies) if ((scene->components.tags[entity] & (mDependencies)) != (mDependencies)) return
#define ENTITY_HAS_DEPS(mEntity, mDependencies) ((scene->components.tags[mEntity] & (mDependencies)) == (mDependencies))
#define GET_COMPONENT(mValue, mEntity) SCENE_GET_COMPONENT_PTR(scene, mEntity, mValue)

void SSmoothUpdate(Scene* scene, const usize entity)
{
    REQUIRE_DEPS(TAG_POSITION | TAG_SMOOTH);

    const CPosition* position = GET_COMPONENT(position, entity);
    CSmooth* smooth = GET_COMPONENT(smooth, entity);

    smooth->previous = position->value;
}

void SKineticUpdate(Scene* scene, const usize entity)
{
    REQUIRE_DEPS(TAG_POSITION | TAG_KINETIC);

    CPosition* position = GET_COMPONENT(position, entity);
    CKinetic* kinetic = GET_COMPONENT(kinetic, entity);

    kinetic->velocity.x += kinetic->acceleration.x * CTX_DT;
    kinetic->velocity.y += kinetic->acceleration.y * CTX_DT;

    position->value.x += kinetic->velocity.x * CTX_DT;
    position->value.y += kinetic->velocity.y * CTX_DT;
}

static i8 sign(const f32 value)
{
    if (value < 0)
    {
        return -1;
    }

    if (value > 0)
    {
        return 1;
    }

    return 0;
}

// TODO(thismarvin): This following static collision stuff needs a better home...

static Vector2 ExtractResolution(const Vector2 resolution, const u64 layers)
{
    Vector2 result = VECTOR2_ZERO;

    if ((layers & LAYER_LEFT) != 0 && resolution.x < 0)
    {
        result.x = resolution.x;
    }

    if ((layers & LAYER_RIGHT) != 0 && resolution.x > 0)
    {
        result.x = resolution.x;
    }

    if ((layers & LAYER_UP) != 0 && resolution.y < 0)
    {
        result.y = resolution.y;
    }

    if ((layers & LAYER_DOWN) != 0 && resolution.y > 0)
    {
        result.y = resolution.y;
    }

    return result;
}

static OnCollisionResult SimulateCollisionOnAxis
(
    Scene* scene,
    const usize entity,
    const usize otherEntity,
    const Vector2 delta,
    const u8 step,
    const OnCollision onCollision
)
{
    // It is important that `delta` only consists of one axis, not both.
    assert(delta.x == 0 || delta.y == 0);

    assert(ENTITY_HAS_DEPS(entity, TAG_POSITION | TAG_DIMENSION));
    assert(ENTITY_HAS_DEPS(otherEntity, TAG_POSITION | TAG_DIMENSION | TAG_COLLIDER));

    const CPosition* positionPointer = GET_COMPONENT(positionPointer, entity);

    CPosition position = *positionPointer;
    const CDimension* dimension = GET_COMPONENT(dimension, entity);

    const CPosition* otherPosition = GET_COMPONENT(otherPosition, otherEntity);
    const CDimension* otherDimensions = GET_COMPONENT(otherDimensions, otherEntity);
    const CCollider* otherCollider = GET_COMPONENT(otherCollider, otherEntity);

    Rectangle aabb = (Rectangle)
    {
        .x = position.value.x,
        .y = position.value.y,
        .width = dimension->width,
        .height = dimension->height,
    };

    Rectangle otherAabb = (Rectangle)
    {
        .x = otherPosition->value.x,
        .y = otherPosition->value.y,
        .width = otherDimensions->width,
        .height = otherDimensions->height
    };

    Vector2 direction = Vector2Create(sign(delta.x), sign(delta.y));
    Vector2 remainder = Vector2Create(fabsf(delta.x), fabsf(delta.y));

    bool xAxisResolved = false;
    bool yAxisResolved = false;

    while (remainder.x > 0 || remainder.y > 0)
    {
        remainder.x -= step * fabsf(direction.x);
        remainder.y -= step * fabsf(direction.y);

        position.value.x += step * direction.x;
        position.value.y += step * direction.y;

        aabb.x = position.value.x;
        aabb.y = position.value.y;

        if (CheckCollisionRecs(aabb, otherAabb))
        {
            Vector2 rawResolution = Vector2Create(-direction.x, -direction.y);
            Vector2 resolution = ExtractResolution(rawResolution, otherCollider->layer);

            // Check if extracting the resolution also invalidated the resolution.
            if (resolution.x == 0 && resolution.y == 0)
            {
                continue;
            }

            Rectangle overlap = GetCollisionRec(aabb, otherAabb);

            // Make sure that the resolution is part of the axis with the least overlap.
            {
                if (resolution.x != 0 && overlap.width > overlap.height)
                {
                    continue;
                }

                if (resolution.y != 0 && overlap.height > overlap.width)
                {
                    continue;
                }
            }

            // Make sure that the resolution points in the direction of the minimum offset.
            {
                f32 left = RectangleLeft(aabb);
                f32 top = RectangleTop(aabb);

                f32 otherLeft = RectangleLeft(otherAabb);
                f32 otherRight = RectangleRight(otherAabb);
                f32 otherTop = RectangleTop(otherAabb);
                f32 otherBottom = RectangleBottom(otherAabb);

                f32 offsetLeft = (otherLeft - aabb.width) - left;
                f32 offsetRight = otherRight - left;
                f32 offsetDown = otherBottom - top;
                f32 offsetUp = (otherTop - aabb.height) - top;

                if (resolution.x < 0 && fabsf(offsetLeft) > fabsf(offsetRight))
                {
                    continue;
                }

                if (resolution.x > 0 && fabsf(offsetRight) > fabsf(offsetLeft))
                {
                    continue;
                }

                if (resolution.y < 0 && fabsf(offsetUp) > fabsf(offsetDown))
                {
                    continue;
                }

                if (resolution.y > 0 && fabsf(offsetDown) > fabsf(offsetUp))
                {
                    continue;
                }
            }

            OnCollisionParams params = (OnCollisionParams)
            {
                .scene = scene,
                .entity = entity,
                .aabb = aabb,
                .otherEntity = otherEntity,
                .otherAabb = otherAabb,
                .overlap = overlap,
                .resolution = resolution,
            };

            OnCollisionResult result = onCollision(&params);

            xAxisResolved |= result.xAxisResolved;
            yAxisResolved |= result.yAxisResolved;

            if ((direction.x != 0 && xAxisResolved) || (direction.y != 0 && yAxisResolved))
            {
                return (OnCollisionResult)
                {
                    .xAxisResolved = xAxisResolved,
                    .yAxisResolved = yAxisResolved,
                };
            }
        }
    }

    return (OnCollisionResult)
    {
        .xAxisResolved = xAxisResolved,
        .yAxisResolved = yAxisResolved,
    };
}

static void AdvancedCollision(Scene* scene, const usize entity, const usize otherEntity)
{
    const CSmooth* smooth = GET_COMPONENT(smooth, entity);
    CPosition* position = GET_COMPONENT(position, entity);
    const CCollider* collider = GET_COMPONENT(collider, entity);

    // TODO(thismarvin): Look into integer based collision.
    Vector2 current = position->value;
    Vector2 previous = smooth->previous;

    Vector2 delta = Vector2Subtract(current, previous);
    i8 step = 1;

    Vector2 original = position->value;
    position->value = previous;

    bool xAxisResolved = false;
    bool yAxisResolved = false;

    // Resolve collision in the x-axis.
    {
        OnCollisionResult result = SimulateCollisionOnAxis(scene, entity, otherEntity,
                                   Vector2Create(delta.x, 0), step, collider->onCollision);

        xAxisResolved |= result.xAxisResolved;
        yAxisResolved |= result.yAxisResolved;

        if (!xAxisResolved)
        {
            position->value.x = original.x;
        }
    }

    // Resolve collision in the y-axis.
    {
        OnCollisionResult result = SimulateCollisionOnAxis(scene, entity, otherEntity,
                                   Vector2Create(0, delta.y), step, collider->onCollision);

        xAxisResolved |= result.xAxisResolved;
        yAxisResolved |= result.yAxisResolved;

        if (!yAxisResolved)
        {
            position->value.y = original.y;
        }
    }
}

void SCollisionUpdate(Scene* scene, const usize entity)
{
    REQUIRE_DEPS(TAG_POSITION | TAG_DIMENSION | TAG_COLLIDER);

    const CPosition* position = GET_COMPONENT(position, entity);
    const CDimension* dimension = GET_COMPONENT(dimension, entity);
    const CCollider* collider = GET_COMPONENT(collider, entity);

    Rectangle aabb = (Rectangle)
    {
        .x = position->value.x,
        .y = position->value.y,
        .width = dimension->width,
        .height = dimension->height
    };

    for (usize i = 0; i < SceneGetEntityCount(scene); ++i)
    {
        if (i == entity || !ENTITY_HAS_DEPS(i, TAG_POSITION | TAG_DIMENSION | TAG_COLLIDER))
        {
            continue;
        }

        const CPosition* otherPosition = GET_COMPONENT(otherPosition, i);
        const CDimension* otherDimensions = GET_COMPONENT(otherDimensions, i);
        const CCollider* otherCollider = GET_COMPONENT(otherCollider, i);

        if ((collider->mask & otherCollider->layer) == 0)
        {
            continue;
        }

        Rectangle otherAabb = (Rectangle)
        {
            .x = otherPosition->value.x,
            .y = otherPosition->value.y,
            .width = otherDimensions->width,
            .height = otherDimensions->height
        };

        if (CheckCollisionRecs(aabb, otherAabb))
        {
            AdvancedCollision(scene, entity, i);

            // TODO(thismarvin): Should we just assume that the entity was resolved?
            aabb.x = position->value.x;
            aabb.y = position->value.y;
        }
    }
}

void SPlayerInputUpdate(Scene* scene, const usize entity)
{
    REQUIRE_DEPS(TAG_PLAYER | TAG_POSITION | TAG_DIMENSION | TAG_KINETIC);

    CPlayer* player = GET_COMPONENT(player, entity);
    const CPosition* position = GET_COMPONENT(position, entity);
    const CDimension* dimension = GET_COMPONENT(dimension, entity);
    CKinetic* kinetic = GET_COMPONENT(kinetic, entity);

    if (player->dead)
    {
        return;
    }

    bool coyoteTimeActive = player->coyoteTimer < player->coyoteDuration;

    // Maintenance.
    {
        player->groundedLastFrame = player->grounded;

        if (player->grounded)
        {
            kinetic->velocity.y = 0;
            player->jumping = false;
        }

        Vector2 gravityForce = Vector2Create(0, player->defaultGravity);

        if (player->jumping && kinetic->velocity.y < player->jumpVelocity)
        {
            gravityForce.y = player->jumpGravity;
        }

        kinetic->acceleration = gravityForce;

        if (coyoteTimeActive)
        {
            player->coyoteTimer += CTX_DT;
        }
    }

    // Lateral Movement.
    {
        i8 strafe = 0;

        if (InputHandlerPressing(&scene->input, "left"))
        {
            strafe = -1;
        }

        if (InputHandlerPressing(&scene->input, "right"))
        {
            strafe = 1;
        }

        kinetic->velocity.x = strafe * player->moveSpeed;
    }

    // Jumping.
    {
        if ((player->grounded || coyoteTimeActive) && !player->jumping
                && InputHandlerPressed(&scene->input, "jump"))
        {
            InputHandlerConsume(&scene->input, "jump");

            player->grounded = false;
            player->jumping = true;
            kinetic->velocity.y = -player->jumpVelocity;

            // Spawn cloud particles.
            {
                const Vector2 bottomCenter = (Vector2)
                {
                    .x = position->value.x + dimension->width * 0.5f,
                    .y = position->value.y + dimension->height
                };

                Vector2 anchorOffset = VECTOR2_ZERO;

                if (kinetic->velocity.x > 0)
                {
                    anchorOffset.x = -dimension->width * 0.5f;
                }

                if (kinetic->velocity.x < 0)
                {
                    anchorOffset.x = dimension->width * 0.5f;
                }

                const Vector2 anchor = Vector2Add(bottomCenter, anchorOffset);

                Vector2 direction = Vector2Normalize(kinetic->velocity);
                direction.x *= -1;

                const EventCloudParticleParams params = (EventCloudParticleParams)
                {
                    .scene = scene,
                    .entity = entity,
                    .anchor = anchor,
                    .direction = direction,
                    .spawnCount = GetRandomValue(10, 25),
                    .spread = dimension->width,
                };

                RaiseSpawnCloudParticleEvent(&params);
            }
        }

        // Variable Jump Height.
        if (InputHandlerReleased(&scene->input, "jump") && kinetic->velocity.y < 0)
        {
            InputHandlerConsume(&scene->input, "jump");

            player->jumping = false;
            kinetic->velocity.y = MAX(kinetic->velocity.y, -player->jumpVelocity * 0.5);
        }
    }

    // Assume that the player is not grounded; prove that it is later.
    player->grounded = false;

    // TODO(thismarvin): Should grounded logic really be in Input?
}

void SPlayerCollisionUpdate(Scene* scene, const usize entity)
{
    REQUIRE_DEPS(TAG_PLAYER | TAG_POSITION | TAG_DIMENSION);

    CPlayer* player = GET_COMPONENT(player, entity);
    CPosition* position = GET_COMPONENT(position, entity);
    const CDimension* dimension = GET_COMPONENT(dimension, entity);

    // General purpose player specific collision logic.
    {
        // Keep the player's x within the scene's bounds.
        if (position->value.x < scene->bounds.x)
        {
            position->value.x = scene->bounds.x;
        }
        else if (position->value.x + dimension->width > RectangleRight(scene->bounds))
        {
            position->value.x = RectangleRight(scene->bounds) - dimension->width;
        }
    }

    // Enable "Coyote Time" if the player walked off an edge.
    if (player->groundedLastFrame && !player->grounded)
    {
        player->coyoteTimer = 0;
    }
}

static bool PlayerIsVulnerable(const CPlayer* player)
{
    return player->invulnerableTimer >= player->invulnerableDuration;
}

static void PlayerFlashingLogic(Scene* scene, const usize entity)
{
    CPlayer* player = GET_COMPONENT(player, entity);

    player->invulnerableTimer += CTX_DT;

    const u32 totalFlashes = 5;
    f32 timeSlice = player->invulnerableDuration / (totalFlashes * 2.0f);

    if (!player->dead && !PlayerIsVulnerable(player))
    {
        u32 passedSlices = (u32)(player->invulnerableTimer / timeSlice);

        if (passedSlices % 2 == 0)
        {
            SceneDeferDisableComponent(scene, entity, TAG_SPRITE);
        }
        else
        {
            SceneDeferEnableComponent(scene, entity, TAG_SPRITE);
        }
    }
    else
    {
        // This is a pre-caution to make sure the last state isn't off.
        SceneDeferEnableComponent(scene, entity, TAG_SPRITE);
    }
}

void SPlayerMortalUpdate(Scene* scene, const usize entity)
{
    REQUIRE_DEPS(TAG_PLAYER | TAG_MORTAL | TAG_POSITION | TAG_KINETIC);

    CPlayer* player = GET_COMPONENT(player, entity);
    const CMortal* mortal = GET_COMPONENT(mortal, entity);
    const CPosition* position = GET_COMPONENT(position, entity);
    CKinetic* kinetic = GET_COMPONENT(kinetic, entity);

    if (position->value.y > CTX_VIEWPORT_HEIGHT * 2)
    {
        // TODO(thismarvin): Defer resetting the Scene somehow...
        SceneReset(scene);

        return;
    }

    if (!player->dead && mortal->hp <= 0)
    {
        player->dead = true;

        SceneDeferDisableComponent(scene, entity, TAG_COLLIDER);

        kinetic->velocity = (Vector2)
        {
            .x = kinetic->velocity.x,
            .y = -250,
        };
    }

    PlayerFlashingLogic(scene, entity);
}

void SFleetingUpdate(Scene* scene, const usize entity)
{
    REQUIRE_DEPS(TAG_FLEETING);

    CFleeting* fleeting = GET_COMPONENT(fleeting, entity);

    fleeting->age += CTX_DT;

    if (fleeting->age > fleeting->lifetime)
    {
        SceneDeferDeallocateEntity(scene, entity);
    }
}

void SCloudParticleDraw(const Scene* scene, const usize entity)
{
    REQUIRE_DEPS(TAG_POSITION | TAG_DIMENSION | TAG_FLEETING | TAG_COLOR | TAG_SMOOTH);

    const CPosition* position = GET_COMPONENT(position, entity);
    const CColor* color = GET_COMPONENT(color, entity);
    const CFleeting* fleeting = GET_COMPONENT(fleeting, entity);
    const CDimension* dimension = GET_COMPONENT(dimension, entity);
    const CSmooth* smooth = GET_COMPONENT(smooth, entity);

    const f32 drawSize = dimension->width * (fleeting->lifetime - fleeting->age) / fleeting->lifetime;

    Vector2 interpolated = Vector2Lerp(smooth->previous, position->value, ContextGetAlpha());

    Vector2 center = (Vector2)
    {
        .x = interpolated.x + dimension->width * 0.5f,
        .y = interpolated.y + dimension->height * 0.5f,
    };

    DrawCircleV(center, drawSize * 0.5f, color->value);
}

void SSpriteDraw(const Scene* scene, const usize entity)
{
    REQUIRE_DEPS(TAG_POSITION | TAG_COLOR | TAG_SPRITE);

    const CPosition* position = GET_COMPONENT(position, entity);
    const CColor* color = GET_COMPONENT(color, entity);
    const CSprite* sprite = GET_COMPONENT(sprite, entity);

    if (ENTITY_HAS_DEPS(entity, TAG_SMOOTH))
    {
        const CSmooth* smooth = GET_COMPONENT(smooth, entity);

        Vector2 interpolated = Vector2Lerp(smooth->previous, position->value, ContextGetAlpha());
        Vector2 drawPosition = Vector2Add(interpolated, sprite->offset);

        DrawTextureRec(scene->atlas, sprite->source, drawPosition, color->value);
    }
    else
    {
        Vector2 drawPosition = Vector2Add(position->value, sprite->offset);

        DrawTextureRec(scene->atlas, sprite->source, drawPosition, color->value);
    }
}

void SDebugDraw(const Scene* scene, const usize entity)
{
    REQUIRE_DEPS(TAG_POSITION | TAG_DIMENSION);

    const CPosition* position = GET_COMPONENT(position, entity);
    const CDimension* dimension = GET_COMPONENT(dimension, entity);

    Rectangle bounds = (Rectangle)
    {
        .x = position->value.x,
        .y = position->value.y,
        .width = dimension->width,
        .height = dimension->height
    };

    DrawRectangleLinesEx(bounds, 4, COLOR_RED);
}
