#include "collider.h"
#include "components.h"
#include "context.h"
#include "raymath.h"
#include "systems.h"

#define REQUIRE_DEPS(dependencies) if ((components->tags[entity] & (dependencies)) != (dependencies)) return
#define HAS_DEPS(dependencies) ((components->tags[entity] & (dependencies)) == (dependencies))
#define ENTITY_HAS_DEPS(other, dependencies) ((components->tags[other] & (dependencies)) == (dependencies))

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

void SPlayerInputUpdate(Scene* scene, usize entity)
{
    Components* components = &scene->components;

    REQUIRE_DEPS(tagPlayer | tagBody | tagKinetic);

    CPlayer* player = &components->players[entity];
    CBody* body = &components->bodies[entity];
    CKinetic* kinetic = &components->kinetics[entity];

    // Maintenance.
    {
        if (body->grounded)
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
        if (body->grounded && !player->jumping && IsKeyDown(KEY_SPACE))
        {
            body->grounded = false;
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

void SPlayerCollisionUpdate(Scene* scene, usize entity)
{
    Components* components = &scene->components;

    REQUIRE_DEPS(tagPlayer | tagPosition | tagDimension | tagCollider | tagBody | tagKinetic);

    CPosition* position = &components->positions[entity];
    CDimension dimension = components->dimensions[entity];
    CBody* body = &components->bodies[entity];
    CKinetic* kinetic = &components->kinetics[entity];

    Rectangle aabb = (Rectangle)
    {
        .x = position->value.x,
        .y = position->value.y,
        .width = dimension.width,
        .height = dimension.height,
    };

    // Assume that the player is not grounded; prove that it is later.
    body->grounded = false;

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

        // This should always be false, but better safe than sorry!
        if (!ENTITY_HAS_DEPS(collisionInner->otherEntity, tagPosition | tagDimension | tagCollider))
        {
            continue;
        }

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

        Vector2 resolution = RectangleRectangleResolution(aabb, otherAabb);

        // Collided with SOLIDS.
        if ((otherCollider.layer & (1 << 2)) != 0)
        {
            position->value = Vector2Add(position->value, resolution);

            aabb.x += resolution.x;
            aabb.y += resolution.y;

            if (resolution.x != 0)
            {
                kinetic->velocity.x = 0;
            }

            if (resolution.y != 0)
            {
                kinetic->velocity.y = 0;

                if (resolution.y < 0)
                {
                    body->grounded = true;
                }
            }

            continue;
        }

        // Collided with ENEMIES.
        if ((otherCollider.layer & (1 << 3)) != 0)
        {
            continue;
        }
    }
}

void SCollisionUpdate(Scene* scene, usize entity)
{
    Components* components = &scene->components;

    u64 deps = tagPosition | tagDimension | tagCollider;
    REQUIRE_DEPS(deps);

    CPosition* position = &components->positions[entity];
    CDimension dimensions = components->dimensions[entity];
    CCollider collider = components->colliders[entity];

    Rectangle bounds = (Rectangle)
    {
        .x = position->value.x,
        .y = position->value.y,
        .width = dimensions.width,
        .height = dimensions.height
    };

    for (usize i = 0; i < SceneGetEntityCount(scene); ++i)
    {
        if (i == entity || (deps & components->tags[i]) != deps)
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

        Rectangle otherBounds = (Rectangle)
        {
            .x = otherPosition.value.x,
            .y = otherPosition.value.y,
            .width = otherDimensions.width,
            .height = otherDimensions.height
        };

        if (CheckCollisionRecs(bounds, otherBounds))
        {
            Event event;
            EventCollisionInit(&event, entity, i, GetCollisionRec(bounds, otherBounds));
            SceneRaiseEvent(scene, &event);
        }
    }
}

// TODO(thismarvin): Do we still neeed this system?
void SVulnerableUpdate(Scene* scene, usize entity)
{
    Components* components = &scene->components;

    u64 collisionDeps = tagPosition | tagDimension | tagCollider;
    u64 deps = collisionDeps | tagMortal;
    REQUIRE_DEPS(deps);

    CPosition position = components->positions[entity];
    CDimension dimensions = components->dimensions[entity];
    CCollider collider = components->colliders[entity];
    // CMortal* mortal = &components->mortals[entity];

    Rectangle bounds = (Rectangle)
    {
        .x = position.value.x,
        .y = position.value.y,
        .width = dimensions.width,
        .height = dimensions.height
    };

    for (usize i = 0; i < SceneGetEntityCount(scene); ++i)
    {
        if (i == entity || !ENTITY_HAS_DEPS(i, collisionDeps | tagDamage))
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

        Rectangle otherBounds = (Rectangle)
        {
            .x = otherPosition.value.x,
            .y = otherPosition.value.y,
            .width = otherDimensions.width,
            .height = otherDimensions.height
        };

        if (CheckCollisionRecs(bounds, otherBounds))
        {
            Event event;
            EventDamageInit(&event, entity, i);
            SceneRaiseEvent(scene, &event);
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

        // This should always be false, but better safe than sorry!
        if (!ENTITY_HAS_DEPS(collisionInner->otherEntity, tagPosition | tagDimension | tagCollider))
        {
            continue;
        }

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

        Vector2 resolution = RectangleRectangleResolution(aabb, otherAabb);

        // Collided with SOLIDS or ENEMIES..
        if ((otherCollider.layer & ((1 << 2) | (1 << 3))) != 0)
        {
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

        // Collided with PLAYER.
        if ((otherCollider.layer & (1 << 1)) != 0)
        {
            continue;
        }
    }
}

void SSpriteDraw(Scene* scene, Texture2D* atlas, usize entity)
{
    Components* components = &scene->components;

    REQUIRE_DEPS(tagPosition | tagColor | tagSprite);

    CPosition position = components->positions[entity];
    CColor color = components->colors[entity];
    CSprite sprite = components->sprites[entity];

    if (HAS_DEPS(tagSmooth))
    {
        CSmooth smooth = components->smooths[entity];
        Vector2 interpolated = Vector2Lerp(smooth.previous, position.value, ContextGetAlpha());

        Vector2 drawPosition = Vector2Add(interpolated, sprite.offset);

        DrawTextureRec(*atlas, sprite.source, drawPosition, color.value);
    }
    else
    {
        Vector2 drawPosition = Vector2Add(position.value, sprite.offset);

        DrawTextureRec(*atlas, sprite.source, drawPosition, color.value);
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
