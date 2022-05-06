#include "../context.h"
#include "../geometry/collider.h"
#include "components.h"
#include "entities.h"
#include "events.h"
#include "systems.h"
#include <assert.h>
#include <raymath.h>

#define REQUIRE_DEPS(mDependencies) if ((scene->components.tags[entity] & (mDependencies)) != (mDependencies)) return
#define ENTITY_HAS_DEPS(mEntity, mDependencies) ((scene->components.tags[mEntity] & (mDependencies)) == (mDependencies))
#define GET_COMPONENT(mValue, mEntity) SCENE_GET_COMPONENT_PTR(scene, mValue, mEntity)

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
    const Scene* scene,
    const usize entity,
    const usize otherEntity,
    const Rectangle aabb,
    const Rectangle otherAabb,
    const Vector2 delta,
    const u8 step,
    const OnCollision onCollision
)
{
    // It is important that `delta` only consists of one axis, not both.
    assert(delta.x == 0 || delta.y == 0);

    Rectangle simulatedAabb = aabb;

    assert(SceneEntityHasDependencies(scene, otherEntity, TAG_COLLIDER));

    const CCollider* otherCollider = GET_COMPONENT(otherCollider, otherEntity);

    Vector2 direction = Vector2Create(sign(delta.x), sign(delta.y));
    Vector2 remainder = Vector2Create(fabsf(delta.x), fabsf(delta.y));

    while (remainder.x > 0 || remainder.y > 0)
    {
        remainder.x -= step * fabsf(direction.x);
        remainder.y -= step * fabsf(direction.y);

        simulatedAabb.x += step * direction.x;
        simulatedAabb.y += step * direction.y;

        if (CheckCollisionRecs(simulatedAabb, otherAabb))
        {
            Vector2 rawResolution = Vector2Create(-direction.x, -direction.y);
            Vector2 resolution = ExtractResolution(rawResolution, otherCollider->layer);

            // Check if extracting the resolution also invalidated the resolution.
            if (resolution.x == 0 && resolution.y == 0)
            {
                continue;
            }

            Rectangle overlap = GetCollisionRec(simulatedAabb, otherAabb);

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
                f32 left = RectangleLeft(simulatedAabb);
                f32 top = RectangleTop(simulatedAabb);

                f32 otherLeft = RectangleLeft(otherAabb);
                f32 otherRight = RectangleRight(otherAabb);
                f32 otherTop = RectangleTop(otherAabb);
                f32 otherBottom = RectangleBottom(otherAabb);

                f32 offsetLeft = (otherLeft - simulatedAabb.width) - left;
                f32 offsetRight = otherRight - left;
                f32 offsetDown = otherBottom - top;
                f32 offsetUp = (otherTop - simulatedAabb.height) - top;

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
                .scene = (Scene*)scene,
                .entity = entity,
                .aabb = simulatedAabb,
                .otherEntity = otherEntity,
                .otherAabb = otherAabb,
                .overlap = overlap,
                .resolution = resolution,
            };

            OnCollisionResult result = onCollision(&params);

            // TODO(thismarvin): Can we just check if x/y changed here?

            simulatedAabb = result.aabb;

            if (result.stop)
            {
                return (OnCollisionResult)
                {
                    .aabb = simulatedAabb,
                    .stop = true,
                };
            }
        }
    }

    return (OnCollisionResult)
    {
        .aabb = aabb,
        .stop = false,
    };
}

static Rectangle AdvancedCollision
(
    const Scene* scene,
    const usize entity,
    const usize otherEntity,
    const Rectangle aabb,
    const Rectangle previousAabb,
    const Rectangle otherAabb,
    const OnCollision onCollision
)
{
    Vector2 previousPosition = (Vector2)
    {
        .x = previousAabb.x,
        .y = previousAabb.y,
    };
    Vector2 currentPosition = (Vector2)
    {
        .x = aabb.x,
        .y = aabb.y,
    };

    Vector2 delta = Vector2Subtract(currentPosition, previousPosition);

    Vector2 dx = (Vector2)
    {
        .x = delta.x,
        .y = 0,
    };
    Vector2 dy = (Vector2)
    {
        .x = 0,
        .y = delta.y,
    };

    bool xMoved = false;
    bool yMoved = false;

    Rectangle simulatedAabb = previousAabb;

    OnCollisionResult xAxisResult = SimulateCollisionOnAxis(scene, entity, otherEntity, simulatedAabb,
                                    otherAabb,
                                    dx, 1, onCollision);

    xMoved |= xAxisResult.aabb.x != simulatedAabb.x;
    yMoved |= xAxisResult.aabb.y != simulatedAabb.y;

    simulatedAabb = xAxisResult.aabb;

    if (!xAxisResult.stop)
    {
        simulatedAabb.x = aabb.x;
    }

    OnCollisionResult yAxisResult = SimulateCollisionOnAxis(scene, entity, otherEntity, simulatedAabb,
                                    otherAabb,
                                    dy, 1, onCollision);

    xMoved |= yAxisResult.aabb.x != simulatedAabb.x;
    yMoved |= yAxisResult.aabb.y != simulatedAabb.y;

    simulatedAabb = yAxisResult.aabb;

    if (!yAxisResult.stop)
    {
        simulatedAabb.y = aabb.y;
    }

    if (xAxisResult.stop || yAxisResult.stop)
    {
        return simulatedAabb;
    }

    // Fallback to original aabb if there was no collision.
    return aabb;
}

void SCollisionUpdate(Scene* scene, const usize entity)
{
    REQUIRE_DEPS(TAG_SMOOTH | TAG_POSITION | TAG_DIMENSION | TAG_COLLIDER);

    const CSmooth* smooth = GET_COMPONENT(smooth, entity);
    CPosition* position = GET_COMPONENT(position, entity);
    const CDimension* dimension = GET_COMPONENT(dimension, entity);
    const CCollider* collider = GET_COMPONENT(collider, entity);

    Rectangle previousAabb = (Rectangle)
    {
        .x = smooth->previous.x,
        .y = smooth->previous.y,
        .width = dimension->width,
        .height = dimension->height
    };
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
            aabb = AdvancedCollision(scene, entity, i, aabb, previousAabb, otherAabb, collider->onCollision);

            // TODO(thismarvin):
            // If this was resolved, do subsequent collision checks really need to use delta?
        }
    }

    position->value.x = aabb.x;
    position->value.y = aabb.y;
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
