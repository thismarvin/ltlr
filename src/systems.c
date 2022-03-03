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

void SPlayerCollisionUpdate(Scene* scene, usize entity)
{
    Components* components = &scene->components;

    REQUIRE_DEPS(tagPlayer | tagPosition | tagDimension | tagCollider | tagKinetic);

    CPlayer* player = &components->players[entity];
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

    // Assume that the player is not grounded; prove that it is later.
    player->grounded = false;

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
            TraceLog(LOG_WARNING, "Illegal collision has occured.");

            continue;
        }

        if (ENTITY_HAS_DEPS(collisionInner->otherEntity, tagWalker))
        {
            // TODO(thismarvin): Decrement health etc. etc.

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

        Vector2 rawResolution = RectangleRectangleResolution(aabb, otherAabb);
        Vector2 resolution = ExtractResolution(rawResolution, otherCollider.layer);

        if ((resolution.x < 0 && kinetic->velocity.x > 0) || (resolution.x > 0 && kinetic->velocity.x < 0))
        {
            kinetic->velocity.x = 0;
        }

        if ((resolution.y < 0 && kinetic->velocity.y > 0) || (resolution.y > 0 && kinetic->velocity.y < 0))
        {
            kinetic->velocity.y = 0;
        }

        if (resolution.y < 0)
        {
            player->grounded = true;
        }

        position->value = Vector2Add(position->value, resolution);

        aabb.x += resolution.x;
        aabb.y += resolution.y;
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
            TraceLog(LOG_WARNING, "Illegal collision has occured.");

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
