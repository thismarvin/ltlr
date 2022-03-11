#include "collider.h"
#include "components.h"
#include "context.h"
#include "raymath.h"
#include "systems.h"
#include <assert.h>

#define REQUIRE_DEPS(dependencies) if ((components->tags[entity] & (dependencies)) != (dependencies)) return
#define ENTITY_HAS_DEPS(other, dependencies) ((components->tags[other] & (dependencies)) == (dependencies))

typedef struct
{
    usize entity;
    usize otherEntity;
    Rectangle aabb;
    Rectangle otherAabb;
    Vector2 resolution;
} CollisionParams;

static i8 sign(f32 value)
{
    if (value < 0)
    {
        return  -1;
    }

    if (value > 0)
    {
        return  1;
    }

    return 0;
}

void SSmoothUpdate(Scene* scene, usize entity)
{
    Components* components = &scene->components;

    REQUIRE_DEPS(tagPosition | tagSmooth);

    CPosition position = components->positions[entity];
    CSmooth* smooth = &components->smooths[entity];

    smooth->previous = position.value;
}

void SKineticUpdate(Scene* scene, usize entity)
{
    Components* components = &scene->components;

    REQUIRE_DEPS(tagPosition | tagKinetic);

    CPosition* position = &components->positions[entity];
    CKinetic* kinetic = &components->kinetics[entity];

    kinetic->velocity.x += kinetic->acceleration.x * CTX_DT;
    kinetic->velocity.y += kinetic->acceleration.y * CTX_DT;

    position->value.x += kinetic->velocity.x * CTX_DT;
    position->value.y += kinetic->velocity.y * CTX_DT;
}

void SCollisionUpdate(Scene* scene, usize entity)
{
    Components* components = &scene->components;

    u64 deps = tagPosition | tagDimension | tagCollider;
    REQUIRE_DEPS(deps);

    CPosition position = components->positions[entity];
    CDimension dimensions = components->dimensions[entity];
    CCollider collider = components->colliders[entity];

    Rectangle aabb = (Rectangle)
    {
        .x = position.value.x,
        .y = position.value.y,
        .width = dimensions.width,
        .height = dimensions.height
    };

    for (usize i = 0; i < SceneGetEntityCount(scene); ++i)
    {
        if (i == entity || !ENTITY_HAS_DEPS(i, tagPosition | tagDimension | tagCollider))
        {
            continue;
        }

        CPosition otherPosition = components->positions[i];
        CDimension otherDimensions = components->dimensions[i];
        CCollider otherCollider = components->colliders[i];

        if ((collider.mask & otherCollider.layer) == 0)
        {
            continue;
        }

        Rectangle otherAabb = (Rectangle)
        {
            .x = otherPosition.value.x,
            .y = otherPosition.value.y,
            .width = otherDimensions.width,
            .height = otherDimensions.height
        };

        if (CheckCollisionRecs(aabb, otherAabb))
        {
            Event event;
            EventCollisionInit(&event, entity, i);
            SceneRaiseEvent(scene, &event);
        }
    }
}

static Vector2 ExtractResolution(Vector2 resolution, u64 layers)
{
    Vector2 result = VECTOR2_ZERO;

    if ((layers & layerLeft) != 0 && resolution.x < 0)
    {
        result.x = resolution.x;
    }

    if ((layers & layerRight) != 0 && resolution.x > 0)
    {
        result.x = resolution.x;
    }

    if ((layers & layerUp) != 0 && resolution.y < 0)
    {
        result.y = resolution.y;
    }

    if ((layers & layerDown) != 0 && resolution.y > 0)
    {
        result.y = resolution.y;
    }

    return result;
}

void SPlayerInputUpdate(Scene* scene, usize entity)
{
    Components* components = &scene->components;

    REQUIRE_DEPS(tagPlayer | tagKinetic);

    CPlayer* player = &components->players[entity];
    CKinetic* kinetic = &components->kinetics[entity];

    // TODO(austin0209): this should be somewhere else...
    if (player->dead)
    {
        assert(ENTITY_HAS_DEPS(entity, tagPosition));
        if (scene->components.positions[entity].value.y > CTX_VIEWPORT_HEIGHT * 2)
        {
            SceneReset(scene);
        }

        return;
    }

    // Maintenance.
    {
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
    }

    // Lateral Movement.
    {
        i8 strafe = 0;

        if (IsKeyDown(KEY_LEFT))
        {
            strafe = -1;
        }

        if (IsKeyDown(KEY_RIGHT))
        {
            strafe = 1;
        }

        kinetic->velocity.x = strafe * player->moveSpeed;
    }

    // Jumping.
    {
        if (player->grounded && !player->jumping && IsKeyDown(KEY_SPACE))
        {
            player->grounded = false;
            player->jumping = true;
            kinetic->velocity.y = -player->jumpVelocity;
        }

        // Variable Jump Height.
        if (player->jumping && !IsKeyDown(KEY_SPACE) && kinetic->velocity.y < 0)
        {
            player->jumping = false;
            kinetic->velocity.y = MAX(kinetic->velocity.y, -player->jumpVelocity * 0.5);
        }
    }
}

static bool PlayerIsVulnerable(const CPlayer* player)
{
    return player->invulnerableTimer >= player->invulnerableDuration;
}

static bool PlayerOnCollision(Scene* scene, CollisionParams params)
{
    Components* components = &scene->components;

    assert(ENTITY_HAS_DEPS(params.entity, tagPlayer | tagPosition | tagKinetic));
    assert(ENTITY_HAS_DEPS(params.otherEntity, tagCollider));

    CPlayer* player = &scene->components.players[params.entity];
    CPosition* position = &scene->components.positions[params.entity];
    CKinetic* kinetic = &scene->components.kinetics[params.entity];

    CCollider otherCollider = scene->components.colliders[params.otherEntity];

    // Collision specific logic that will not resolve the player.
    {
        if (ENTITY_HAS_DEPS(params.entity, tagMortal) &&
            ENTITY_HAS_DEPS(params.otherEntity, tagWalker | tagDamage))
        {
            if (PlayerIsVulnerable(player))
            {
                CMortal* mortal = &scene->components.mortals[params.entity];
                CDamage otherDamage = scene->components.damages[params.otherEntity];

                mortal->hp -= otherDamage.value;
                player->invulnerableTimer = 0;

                if (mortal->hp == 0)
                {
                    player->dead = true;
                    // Remove collider component
                    // TODO(austin0209): Modifying tags like this is unsafe for multithreaded code
                    components->tags[params.entity] &= ~tagCollider;
                    kinetic->velocity = (Vector2)
                    {
                        .x = kinetic->velocity.x < 0 ? -50 : 50,
                        .y = -250,
                    };
                }
            }

            return false;
        }
    }

    Vector2 resolution = ExtractResolution(params.resolution, otherCollider.layer);

    if (resolution.x == 0 && resolution.y == 0)
    {
        return false;
    }

    // Make sure that the resolution is part of the axis with the least overlap.
    {
        Rectangle overlap = GetCollisionRec(params.aabb, params.otherAabb);

        if (resolution.x != 0 && overlap.width > overlap.height)
        {
            return false;
        }

        if (resolution.y != 0 && overlap.height > overlap.width)
        {
            return false;
        }
    }

    // Make sure that the resolution points in the direction of the minimum offset.
    {
        f32 offsetLeft = (RectangleLeft(params.otherAabb) - params.aabb.width) - RectangleLeft(params.aabb);
        f32 offsetRight = RectangleRight(params.otherAabb) - RectangleLeft(params.aabb);
        f32 offsetDown = RectangleBottom(params.otherAabb) - RectangleTop(params.aabb);
        f32 offsetUp = (RectangleTop(params.otherAabb) - params.aabb.height) - RectangleTop(params.aabb);

        if (resolution.x < 0 && fabsf(offsetLeft) > fabsf(offsetRight))
        {
            return false;
        }

        if (resolution.x > 0 && fabsf(offsetRight) > fabsf(offsetLeft))
        {
            return false;
        }

        if (resolution.y < 0 && fabsf(offsetUp) > fabsf(offsetDown))
        {
            return false;
        }

        if (resolution.y > 0 && fabsf(offsetDown) > fabsf(offsetUp))
        {
            return false;
        }
    }

    // Resolve collision.
    {
        if (resolution.x < 0)
        {
            position->value.x = RectangleLeft(params.otherAabb) - params.aabb.width;
        }
        else if (resolution.x > 0)
        {
            position->value.x = RectangleRight(params.otherAabb);
        }

        if (resolution.y < 0)
        {
            position->value.y = RectangleTop(params.otherAabb) - params.aabb.height;
        }
        else if (resolution.y > 0)
        {
            position->value.y = RectangleBottom(params.otherAabb);
        }
    }

    // Resolution specific player logic.
    {
        if ((resolution.x < 0 && kinetic->velocity.x > 0) || (resolution.x > 0
                && kinetic->velocity.x < 0))
        {
            kinetic->velocity.x = 0;
        }

        if ((resolution.y < 0 && kinetic->velocity.y > 0) || (resolution.y > 0
                && kinetic->velocity.y < 0))
        {
            kinetic->velocity.y = 0;
        }

        if (resolution.y < 0)
        {
            player->grounded = true;
        }
    }

    return true;
}

// TODO(thismarvin): Is it possible to have an `OnCollision` callback as a parameter?
static bool SimulateCollisionOnAxis(Scene* scene, usize entity, Vector2 delta, u8 step)
{
    // It is important that `delta` only consists of one axis, not both.
    assert(delta.x == 0 || delta.y == 0);

    Components* components = &scene->components;

    assert(ENTITY_HAS_DEPS(entity, tagPosition | tagDimension));

    CPosition* position = &components->positions[entity];
    CDimension dimension = components->dimensions[entity];

    Rectangle aabb = (Rectangle)
    {
        .x = position->value.x,
        .y = position->value.y,
        .width = dimension.width,
        .height = dimension.height,
    };

    Vector2 direction = Vector2Create(sign(delta.x), sign(delta.y));
    Vector2 remainder = Vector2Create(fabsf(delta.x), fabsf(delta.y));

    while (remainder.x > 0 || remainder.y > 0)
    {
        remainder.x -= step * fabsf(direction.x);
        remainder.y -= step * fabsf(direction.y);

        position->value.x += step * direction.x;
        position->value.y += step * direction.y;

        aabb.x = position->value.x;
        aabb.y = position->value.y;

        for (usize i = 0; i < SceneGetEventCount(scene); ++i)
        {
            Event* event = &scene->eventManager.events[i];

            if (event->entity != entity || event->tag != EVENT_COLLISION)
            {
                continue;
            }

            const EventCollisionInner* collisionInner = &event->collisionInner;

            assert(ENTITY_HAS_DEPS(collisionInner->otherEntity, tagPosition | tagDimension | tagCollider));

            CPosition otherPosition = components->positions[collisionInner->otherEntity];
            CDimension otherDimensions = components->dimensions[collisionInner->otherEntity];

            Rectangle otherAabb = (Rectangle)
            {
                .x = otherPosition.value.x,
                .y = otherPosition.value.y,
                .width = otherDimensions.width,
                .height = otherDimensions.height
            };

            if (CheckCollisionRecs(aabb, otherAabb))
            {
                CollisionParams params = (CollisionParams)
                {
                    .entity = entity,
                    .otherEntity = collisionInner->otherEntity,
                    .aabb = aabb,
                    .otherAabb = otherAabb,
                    .resolution = Vector2Create(-direction.x, -direction.y),
                };

                bool resolved = PlayerOnCollision(scene, params);

                if (resolved)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

void SPlayerCollisionUpdate(Scene* scene, usize entity)
{
    Components* components = &scene->components;

    REQUIRE_DEPS(tagPlayer | tagSmooth | tagPosition | tagDimension | tagCollider | tagKinetic);

    CPlayer* player = &components->players[entity];
    CSmooth smooth = components->smooths[entity];
    CPosition* position = &components->positions[entity];
    CDimension dimension = components->dimensions[entity];

    // General purpose player specific collision logic.
    {
        // Assume that the player is not grounded; prove that it is later.
        player->grounded = false;

        // Update collision-related timers
        player->invulnerableTimer += CTX_DT;

        // Keep the player's x within the scene's bounds.
        if (position->value.x < scene->bounds.x)
        {
            position->value.x = scene->bounds.x;
        }
        else if (position->value.x + dimension.width > RectangleRight(scene->bounds))
        {
            position->value.x = RectangleRight(scene->bounds) - dimension.width;
        }
    }

    // TODO(thismarvin): Look into integer based collision.
    Vector2 current = position->value;
    Vector2 previous = smooth.previous;

    Vector2 delta = Vector2Subtract(current, previous);
    i8 step = 1;

    Vector2 original = position->value;
    position->value = previous;

    // Resolve collision in the x-axis.
    {
        bool resolved = SimulateCollisionOnAxis(scene, entity, Vector2Create(delta.x, 0), step);

        if (!resolved)
        {
            position->value.x = original.x;
        }
    }

    // Resolve collision in the y-axis.
    {
        bool resolved = SimulateCollisionOnAxis(scene, entity, Vector2Create(0, delta.y), step);

        if (!resolved)
        {
            position->value.y = original.y;
        }
    }

    // Consume Collision events.
    {
        for (usize i = 0; i < SceneGetEventCount(scene); ++i)
        {
            Event* event = &scene->eventManager.events[i];

            if (event->entity != entity || event->tag != EVENT_COLLISION)
            {
                continue;
            }

            SceneConsumeEvent(scene, i);
        }
    }
}

void SWalkerCollisionUpdate(Scene* scene, usize entity)
{
    Components* components = &scene->components;

    REQUIRE_DEPS(tagWalker | tagPosition | tagDimension | tagCollider | tagKinetic);

    CPosition* position = &components->positions[entity];
    CDimension dimension = components->dimensions[entity];
    CKinetic* kinetic = &components->kinetics[entity];

    Rectangle aabb = (Rectangle)
    {
        .x = position->value.x,
        .y = position->value.y,
        .width = dimension.width,
        .height = dimension.height,
    };

    // TODO(thismarvin): A lot of this is copied from Player...

    // Consume Collision event.
    for (usize i = 0; i < SceneGetEventCount(scene); ++i)
    {
        Event* event = &scene->eventManager.events[i];

        if (event->entity != entity || event->tag != EVENT_COLLISION)
        {
            continue;
        }

        SceneConsumeEvent(scene, i);

        const EventCollisionInner* collisionInner = &event->collisionInner;

        assert(ENTITY_HAS_DEPS(collisionInner->otherEntity, tagPosition | tagDimension | tagCollider));

        CPosition otherPosition = components->positions[collisionInner->otherEntity];
        CDimension otherDimension = components->dimensions[collisionInner->otherEntity];
        CCollider otherCollider = components->colliders[collisionInner->otherEntity];

        Rectangle otherAabb = (Rectangle)
        {
            .x = otherPosition.value.x,
            .y = otherPosition.value.y,
            .width = otherDimension.width,
            .height = otherDimension.height,
        };

        Vector2 rawResolution = RectangleRectangleResolution(aabb, otherAabb);
        Vector2 resolution = ExtractResolution(rawResolution, otherCollider.layer);

        position->value = Vector2Add(position->value, resolution);

        aabb.x += resolution.x;
        aabb.y += resolution.y;

        // Walk side to side.
        if (resolution.x != 0)
        {
            kinetic->velocity.x *= -1;
        }

        if (resolution.y != 0)
        {
            kinetic->velocity.y = 0;
        }

        continue;
    }
}

void SSpriteDraw(Scene* scene, Texture2D* atlas, usize entity)
{
    Components* components = &scene->components;

    REQUIRE_DEPS(tagPosition | tagColor | tagSprite);

    CPosition position = components->positions[entity];
    CColor color = components->colors[entity];
    CSprite* sprite = &components->sprites[entity];

    if (ENTITY_HAS_DEPS(entity, tagPlayer))
    {
        // Fancy math for player flashing.
        const CPlayer* player = &components->players[entity];
        const u32 numFlashes = 5;
        f32 timeSlice = player->invulnerableDuration / (numFlashes * 2.0f);

        if (!player->dead && !PlayerIsVulnerable(player))
        {
            u32 numSlices = (u32)(player->invulnerableTimer / timeSlice);
            sprite->enabled = numSlices % 2 == 1;
        }
        else
        {
            sprite->enabled = true;
        }
    }

    if (!sprite->enabled) return;

    if (ENTITY_HAS_DEPS(entity, tagSmooth))
    {
        CSmooth smooth = components->smooths[entity];

        Vector2 interpolated = Vector2Lerp(smooth.previous, position.value, ContextGetAlpha());
        Vector2 drawPosition = Vector2Add(interpolated, sprite->offset);

        DrawTextureRec(*atlas, sprite->source, drawPosition, color.value);
    }
    else
    {
        Vector2 drawPosition = Vector2Add(position.value, sprite->offset);

        DrawTextureRec(*atlas, sprite->source, drawPosition, color.value);
    }
}

void SDebugDraw(Scene* scene, usize entity)
{
    Components* components = &scene->components;

    REQUIRE_DEPS(tagPosition | tagDimension);

    CPosition position = components->positions[entity];
    CDimension dimensions = components->dimensions[entity];

    Rectangle bounds = (Rectangle)
    {
        .x = position.value.x,
        .y = position.value.y,
        .width = dimensions.width,
        .height = dimensions.height
    };

    DrawRectangleLinesEx(bounds, 4, RED);
}
