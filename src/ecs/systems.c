#include "../animation.h"
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

typedef struct
{
    Rectangle simulatedAabb;
    bool xModified;
    bool yModified;
} SimulateCollisionOnAxisResult;

typedef struct
{
    Scene* scene;
    usize entity;
    Rectangle aabb;
    CCollider* collider;
    Vector2 delta;
    u8 step;
    OnResolution onResolution;
} SimulateCollisionOnAxisParams;

typedef struct
{
    Scene* scene;
    usize entity;
    Rectangle currentAabb;
    Rectangle previousAabb;
    CCollider* collider;
    OnResolution onResolution;
} AdvancedCollisionParams;

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

// TODO(thismarvin): This following static collision stuff needs a better home...

static Vector2 ExtractResolution(const Vector2 resolution, const u64 layers)
{
    Vector2 result = VECTOR2_ZERO;

    if ((layers & RESOLVE_LEFT) != 0 && resolution.x < 0)
    {
        result.x = resolution.x;
    }

    if ((layers & RESOLVE_RIGHT) != 0 && resolution.x > 0)
    {
        result.x = resolution.x;
    }

    if ((layers & RESOLVE_UP) != 0 && resolution.y < 0)
    {
        result.y = resolution.y;
    }

    if ((layers & RESOLVE_DOWN) != 0 && resolution.y > 0)
    {
        result.y = resolution.y;
    }

    return result;
}

static SimulateCollisionOnAxisResult SimulateCollisionOnAxis
(
    const SimulateCollisionOnAxisParams* params
)
{
    // It is important that `delta` only consists of one axis, not both.
    assert(params->delta.x == 0 || params->delta.y == 0);

    Rectangle simulatedAabb = params->aabb;

    Vector2 direction = Vector2Create(SIGN(params->delta.x), SIGN(params->delta.y));
    Vector2 remainder = Vector2Create(fabsf(params->delta.x), fabsf(params->delta.y));

    bool xModified = false;
    bool yModified = false;

    while (remainder.x > 0 || remainder.y > 0)
    {
        remainder.x -= params->step * fabsf(direction.x);
        remainder.y -= params->step * fabsf(direction.y);

        simulatedAabb.x += params->step * direction.x;
        simulatedAabb.y += params->step * direction.y;

        for (usize i = 0; i < SceneGetEntityCount(params->scene); ++i)
        {
            const u64 dependencies = TAG_POSITION | TAG_DIMENSION | TAG_COLLIDER;

            if (i == params->entity || !SceneEntityHasDependencies(params->scene, i, dependencies))
            {
                continue;
            }

            const CPosition* otherPosition = SCENE_GET_COMPONENT_PTR(params->scene, otherPosition, i);
            const CDimension* otherDimension = SCENE_GET_COMPONENT_PTR(params->scene, otherDimension, i);
            const CCollider* otherCollider = SCENE_GET_COMPONENT_PTR(params->scene, otherCollider, i);

            if ((params->collider->mask & otherCollider->layer) == 0)
            {
                continue;
            }

            Rectangle otherAabb = (Rectangle)
            {
                .x = otherPosition->value.x,
                .y = otherPosition->value.y,
                .width = otherDimension->width,
                .height = otherDimension->height
            };

            if (CheckCollisionRecs(simulatedAabb, otherAabb))
            {
                Vector2 rawResolution = Vector2Create(-direction.x, -direction.y);
                Vector2 resolution = ExtractResolution(rawResolution, otherCollider->resolutionSchema);

                // Check if extracting the resolution also invalidated the resolution.
                if (resolution.x == 0 && resolution.y == 0)
                {
                    continue;
                }

                Rectangle overlap = GetCollisionRec(simulatedAabb, otherAabb);

                // Make sure that the resolution is part of the axis with the least overlap.
                {
                    if (resolution.x != 0 && overlap.width >= overlap.height)
                    {
                        continue;
                    }

                    if (resolution.y != 0 && overlap.height >= overlap.width)
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

                OnResolutionParams onResolutionParams = (OnResolutionParams)
                {
                    .scene = params->scene,
                    .entity = params->entity,
                    .aabb = simulatedAabb,
                    .otherEntity = i,
                    .otherAabb = otherAabb,
                    .overlap = overlap,
                    .resolution = resolution,
                };

                OnResolutionResult result = params->onResolution(&onResolutionParams);

                xModified |= result.aabb.x != simulatedAabb.x;
                yModified |= result.aabb.y != simulatedAabb.y;

                simulatedAabb = result.aabb;
            }
        }

        if ((direction.x != 0 && xModified) || (direction.y != 0 && yModified))
        {
            break;
        }
    }

    return (SimulateCollisionOnAxisResult)
    {
        .simulatedAabb = simulatedAabb,
        .xModified = xModified,
        .yModified = yModified,
    };
}

static Rectangle AdvancedCollision(const AdvancedCollisionParams* params)
{
    Vector2 previousPosition = (Vector2)
    {
        .x = params->previousAabb.x,
        .y = params->previousAabb.y,
    };
    Vector2 currentPosition = (Vector2)
    {
        .x = params->currentAabb.x,
        .y = params->currentAabb.y,
    };

    Vector2 delta = Vector2Subtract(currentPosition, previousPosition);

    Rectangle simulatedAabb = params->previousAabb;
    bool xModified = false;
    bool yModified = false;

    const u8 step = 1;

    // Simulate just the x-axis.
    {
        Vector2 xDelta = (Vector2)
        {
            .x = delta.x,
            .y = 0,
        };

        SimulateCollisionOnAxisParams onAxisParams = (SimulateCollisionOnAxisParams)
        {
            .scene = (Scene*)params->scene,
            .entity = params->entity,
            .aabb = simulatedAabb,
            .collider = params->collider,
            .delta = xDelta,
            .step = step,
            .onResolution = params->onResolution,
        };

        SimulateCollisionOnAxisResult result = SimulateCollisionOnAxis(&onAxisParams);

        simulatedAabb = result.simulatedAabb;
        xModified |= result.xModified;
        yModified |= result.yModified;

        // If the aabb was not resolved then fallback to its original x-position.
        if (!xModified)
        {
            simulatedAabb.x = params->currentAabb.x;
        }
    }

    // Simulate just the y-axis.
    {
        Vector2 yDelta = (Vector2)
        {
            .x = 0,
            .y = delta.y,
        };

        SimulateCollisionOnAxisParams onAxisParams = (SimulateCollisionOnAxisParams)
        {
            .scene = (Scene*)params->scene,
            .entity = params->entity,
            .aabb = simulatedAabb,
            .collider = params->collider,
            .delta = yDelta,
            .step = step,
            .onResolution = params->onResolution,
        };

        SimulateCollisionOnAxisResult result = SimulateCollisionOnAxis(&onAxisParams);

        simulatedAabb = result.simulatedAabb;
        xModified |= result.xModified;
        yModified |= result.yModified;

        // If the aabb was not resolved then fallback to its original y-position.
        if (!yModified)
        {
            simulatedAabb.y = params->currentAabb.y;
        }
    }

    return simulatedAabb;
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
    Rectangle currentAabb = (Rectangle)
    {
        .x = position->value.x,
        .y = position->value.y,
        .width = dimension->width,
        .height = dimension->height
    };

    AdvancedCollisionParams params = (AdvancedCollisionParams)
    {
        .scene = scene,
        .entity = entity,
        .currentAabb = currentAabb,
        .previousAabb = previousAabb,
        .collider = (CCollider*)collider,
        .onResolution = collider->onResolution
    };

    Rectangle resolvedAabb = AdvancedCollision(&params);

    position->value.x = resolvedAabb.x;
    position->value.y = resolvedAabb.y;
}

void SPostCollisionUpdate(Scene* scene, const usize entity)
{
    REQUIRE_DEPS(TAG_POSITION | TAG_DIMENSION | TAG_COLLIDER);

    CPosition* position = GET_COMPONENT(position, entity);
    const CDimension* dimension = GET_COMPONENT(dimension, entity);
    const CCollider* collider = GET_COMPONENT(collider, entity);

    const Rectangle aabb = (Rectangle)
    {
        .x = position->value.x,
        .y = position->value.y,
        .width = dimension->width,
        .height = dimension->height,
    };

    for (usize i = 0; i < SceneGetEntityCount(scene); ++i)
    {
        const u64 dependencies = TAG_POSITION | TAG_DIMENSION | TAG_COLLIDER;

        if (i == entity || !SceneEntityHasDependencies(scene, i, dependencies))
        {
            continue;
        }

        const CPosition* otherPosition = SCENE_GET_COMPONENT_PTR(scene, otherPosition, i);
        const CDimension* otherDimension = SCENE_GET_COMPONENT_PTR(scene, otherDimension, i);
        const CCollider* otherCollider = SCENE_GET_COMPONENT_PTR(scene, otherCollider, i);

        if ((collider->mask & otherCollider->layer) == 0)
        {
            continue;
        }

        Rectangle otherAabb = (Rectangle)
        {
            .x = otherPosition->value.x,
            .y = otherPosition->value.y,
            .width = otherDimension->width,
            .height = otherDimension->height
        };

        if (CheckCollisionRecs(aabb, otherAabb))
        {
            Rectangle overlap = GetCollisionRec(aabb, otherAabb);

            OnCollisionParams onCollisionParams = (OnCollisionParams)
            {
                .scene = scene,
                .entity = entity,
                .aabb = aabb,
                .otherEntity = i,
                .otherAabb = otherAabb,
                .overlap = overlap,
            };

            collider->onCollision(&onCollisionParams);
        }
    }
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

void SAnimationUpdate(Scene* scene, const usize entity)
{
    REQUIRE_DEPS(TAG_ANIMATION);

    CAnimation* animation = GET_COMPONENT(animation, entity);

    animation->frameTimer += CTX_DT;

    if (animation->frameTimer >= animation->frameDuration)
    {
        animation->frameTimer = 0.0f;
        animation->frame = (animation->frame + 1) % animation->length;
    }
}

void SSpriteDraw(const Scene* scene, const usize entity)
{
    REQUIRE_DEPS(TAG_POSITION | TAG_SPRITE);

    const CPosition* position = GET_COMPONENT(position, entity);
    const CSprite* sprite = GET_COMPONENT(sprite, entity);

    Vector2 drawPosition = Vector2Add(position->value, sprite->offset);

    if (ENTITY_HAS_DEPS(entity, TAG_SMOOTH))
    {
        const CSmooth* smooth = GET_COMPONENT(smooth, entity);

        const Vector2 interpolated = Vector2Lerp(smooth->previous, position->value, ContextGetAlpha());
        drawPosition = Vector2Add(interpolated, sprite->offset);
    }

    Rectangle source = sprite->source;

    if ((sprite->mirroring & FLIP_HORIZONTAL) != 0)
    {
        source.width = -sprite->source.width;
    }

    if ((sprite->mirroring & FLIP_VERTICAL) != 0)
    {
        source.height = -sprite->source.height;
    }

    if (ENTITY_HAS_DEPS(entity, TAG_COLOR))
    {
        const CColor* color = GET_COMPONENT(color, entity);
        DrawTextureRec(scene->atlasTexture, source, drawPosition, color->value);
    }
    else
    {
        DrawTextureRec(scene->atlasTexture, source, drawPosition, COLOR_WHITE);
    }
}

void SAnimationDraw(const Scene* scene, const usize entity)
{
    REQUIRE_DEPS(TAG_POSITION | TAG_ANIMATION);

    const CPosition* position = GET_COMPONENT(position, entity);
    const CAnimation* animation = SCENE_GET_COMPONENT_PTR(scene, animation, entity);

    Vector2 drawPosition = Vector2Add(position->value, animation->offset);

    if (ENTITY_HAS_DEPS(entity, TAG_SMOOTH))
    {
        const CSmooth* smooth = GET_COMPONENT(smooth, entity);

        const Vector2 interpolated = Vector2Lerp(smooth->previous, position->value, ContextGetAlpha());
        drawPosition = Vector2Add(interpolated, animation->offset);
    }

    const char* name = ANIMATIONS[animation->handle][animation->frame];
    const AtlasSprite* atlasSprite = AtlasGet(&scene->atlas, name);

    Rectangle source = (Rectangle)
    {
        .x = atlasSprite->destination.x,
        .y = atlasSprite->destination.y,
        .width = atlasSprite->destination.width,
        .height = atlasSprite->destination.height,
    };

    if ((animation->mirroring & FLIP_HORIZONTAL) != 0)
    {
        source.width = -source.width;
    }

    if ((animation->mirroring & FLIP_VERTICAL) != 0)
    {
        source.height = -source.height;
    }

    if (ENTITY_HAS_DEPS(entity, TAG_COLOR))
    {
        const CColor* color = GET_COMPONENT(color, entity);
        DrawTextureRec(scene->atlasTexture, source, drawPosition, color->value);
    }
    else
    {
        DrawTextureRec(scene->atlasTexture, source, drawPosition, COLOR_WHITE);
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
