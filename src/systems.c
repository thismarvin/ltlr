#include "collider.h"
#include "components.h"
#include "context.h"
#include "raymath.h"
#include "systems.h"

#define REQUIRE_DEPS(dependencies) if ((components->tags[entity] & (dependencies)) != (dependencies)) return
#define HAS_DEPS(dependencies) ((components->tags[entity] & (dependencies)) == (dependencies))

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

void SPlayerUpdate(Components* components, usize entity)
{
    REQUIRE_DEPS(tagPlayer | tagKinetic | tagBody);

    CKinetic* kinetic = &components->kinetics[entity];
    CBody* body = &components->bodies[entity];
    CPlayer* player = &components->players[entity];

    // TODO(thismarvin): Extract some variable (e.g. speed) into their own component.

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

        kinetic->velocity.x = strafe * 500;
    }

    if (body->grounded)
    {
        kinetic->velocity.y = 0;
    }

    // Variable Jump Height.
    {
        if (body->grounded && !player->jumping && IsKeyDown(KEY_SPACE))
        {
            kinetic->velocity.y = -300;
            player->jumping = true;
            body->grounded = false;
        }

        if (player->jumping && !IsKeyDown(KEY_SPACE) && kinetic->velocity.y < 0)
        {
            kinetic->velocity.y = -50;
            player->jumping = false;
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

            if (!body->grounded && resolution.y < 0)
            {
                body->grounded = true;

                // TODO(thismarvin): Hacky? Maybe, but it works!
                if (HAS_DEPS(tagPlayer))
                {
                    CPlayer* player = &components->players[entity];

                    player->jumping = false;
                }
            }
        }
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
