#include "collider.h"
#include "components.h"
#include "context.h"
#include "raymath.h"
#include "systems.h"

#define REQUIRE_DEPS(dependencies) if ((components->tags[entity] & (dependencies)) != (dependencies)) return
#define HAS_DEPS(dependencies) ((components->tags[entity] & (dependencies)) == (dependencies))
#define ENTITY_HAS_DEPS(other, dependencies) ((components->tags[other] & (dependencies)) == (dependencies))

void SSmoothUpdate(Components* components, usize entity)
{
    REQUIRE_DEPS(tagPosition | tagSmooth);

    CPosition position = components->positions[entity];
    CSmooth* smooth = &components->smooths[entity];

    smooth->previous = position.value;
}

void SKineticUpdate(Components* components, usize entity)
{
    REQUIRE_DEPS(tagPosition | tagKinetic);

    CPosition* position = &components->positions[entity];
    CKinetic* kinetic = &components->kinetics[entity];

    kinetic->velocity.x += kinetic->acceleration.x * CTX_DT;
    kinetic->velocity.y += kinetic->acceleration.y * CTX_DT;

    position->value.x += kinetic->velocity.x * CTX_DT;
    position->value.y += kinetic->velocity.y * CTX_DT;
}

void SPlayerUpdate(Scene* scene, usize entity)
{
    Components* components = &scene->components;
    REQUIRE_DEPS(tagPlayer | tagKinetic | tagBody | tagMortal);

    CKinetic* kinetic = &components->kinetics[entity];
    CBody* body = &components->bodies[entity];
    CPlayer* player = &components->players[entity];
    CMortal* mortal = &components->players[entity];

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

    bool grounded = body->resolution.y < 0;

    if (grounded)
    {
        kinetic->velocity.y = 0;
        player->jumping = false;
    }

    // Variable Jump Height.
    {
        if (grounded && !player->jumping && IsKeyDown(KEY_SPACE))
        {
            kinetic->velocity.y = -player->jumpVelocity;
            player->jumping = true;
            grounded = false;
        }

        if (player->jumping && !IsKeyDown(KEY_SPACE) && kinetic->velocity.y < 0)
        {
            kinetic->velocity.y = MAX(kinetic->velocity.y, -player->jumpVelocity * 0.5);
            player->jumping = false;
        }
    }

    {
        Vector2 gravityForce = Vector2Create(0, player->defaultGravity);

        if (player->jumping && kinetic->velocity.y < player->jumpVelocity)
        {
            gravityForce.y = player->jumpGravity;
        }

        kinetic->acceleration = gravityForce;
    }

    // Consume events
    {
        for (usize i = 0; i < scene->events.nextEvent; ++i)
        {
            Event* event = &scene->events.events[i];

            if (event->entity != entity)
            {
                continue;
            }

            switch (event->tag)
            {
                case EVENT_DAMAGE:
                {
                    EventDamageInner* damageInner = &event->damageInner;
                    CDamage damage = components->damages[damageInner->otherEntity];

                    mortal->hp -= damage.value;
                    printf("HIT: %d\n", mortal->hp);
                    SceneConsumeEvent(scene, i);
                    break;
                }
            }
        }
    }
}

void SCollisionUpdate(Components* components, usize entityCount, usize entity)
{
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

    if (HAS_DEPS(tagBody))
    {
        CBody* body = &components->bodies[entity];

        body->resolution = VECTOR2_ZERO;
    }

    for (usize i = 0; i < entityCount; ++i)
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

        Vector2 resolution = RectangleRectangleResolution(bounds, otherBounds);

        Vector2 newPos = Vector2Add(position->value, resolution);
        bounds.x = newPos.x;
        bounds.y = newPos.y;

        position->value.x = newPos.x;
        position->value.y = newPos.y;

        if (HAS_DEPS(tagBody))
        {
            CBody* body = &components->bodies[entity];

            // TODO(thismarvin): Look into this some more... is it actually working?
            body->resolution = Vector2Add(body->resolution, resolution);
        }
    }
}

void SVulnerableUpdate(Scene* scene, usize entity)
{
    Components* components = &scene->components;
    u64 collisionDeps = tagPosition | tagDimension | tagCollider;
    u64 deps = collisionDeps | tagMortal;
    REQUIRE_DEPS(deps);

    CPosition position = components->positions[entity];
    CDimension dimensions = components->dimensions[entity];
    CCollider collider = components->colliders[entity];
    CMortal* mortal = &components->mortals[entity];

    Rectangle bounds = (Rectangle)
    {
        .x = position.value.x,
        .y = position.value.y,
        .width = dimensions.width,
        .height = dimensions.height
    };

    for (usize i = 0; i < SceneGetEntityCount(scene); ++i)
    {
        if (i == entity || !ENTITY_HAS_DEPS(i, collisionDeps))
        {
            continue;
        }

        CPosition otherPosition = components->positions[i];
        CDimension otherDimensions = components->dimensions[i];
        CCollider otherCollider = components->colliders[i];

        Rectangle otherBounds = (Rectangle)
        {
            .x = otherPosition.value.x,
            .y = otherPosition.value.y,
            .width = otherDimensions.width,
            .height = otherDimensions.height
        };

        bool collides = CheckCollisionRecs(bounds, otherBounds);

        if (collides && ENTITY_HAS_DEPS(i, tagDamage))
        {
            // CDamage damage = components->damages[i];
            // mortal->hp -= damage.value;
            // printf("HIT: %d\n", mortal->hp);

            Event event;
            EventDamageInit(&event, entity, i);
            SceneRaiseEvent(scene, &event);
        }
    }
}

void SWalkerUpdate(Components* components, usize entity)
{
    REQUIRE_DEPS(tagWalker | tagKinetic | tagBody);

    CKinetic* kinetic = &components->kinetics[entity];
    CBody body = components->bodies[entity];

    bool grounded = body.resolution.y < 0;

    if (grounded)
    {
        kinetic->velocity.y = 0;
    }

    if (body.resolution.x != 0)
    {
        // Side collision
        kinetic->velocity.x *= -1;
    }
}

void SSpriteDraw(Components* components, Texture2D* atlas, usize entity)
{
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

void SDebugDraw(Components* components, usize entity)
{
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
