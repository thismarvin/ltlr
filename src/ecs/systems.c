#include "../context.h"
#include "../palette/p8.h"
#include "components.h"
#include "systems.h"
#include <assert.h>
#include <raymath.h>

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
    static const u64 dependencies = TAG_POSITION | TAG_SMOOTH;

    if (!SceneEntityHasDependencies(scene, entity, dependencies))
    {
        return;
    }

    const CPosition* position = &scene->components.positions[entity];
    CSmooth* smooth = &scene->components.smooths[entity];

    smooth->previous = position->value;
}

void SKineticUpdate(Scene* scene, const usize entity)
{
    static const u64 dependencies = TAG_POSITION | TAG_KINETIC;

    if (!SceneEntityHasDependencies(scene, entity, dependencies))
    {
        return;
    }

    CPosition* position = &scene->components.positions[entity];
    CKinetic* kinetic = &scene->components.kinetics[entity];

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

    const Vector2 direction = Vector2Create(SIGN(params->delta.x), SIGN(params->delta.y));
    Vector2 remainder = Vector2Create(fabsf(params->delta.x), fabsf(params->delta.y));

    bool xModified = false;
    bool yModified = false;

    while (remainder.x > 0 || remainder.y > 0)
    {
        remainder.x -= params->step * fabsf(direction.x);
        remainder.y -= params->step * fabsf(direction.y);

        simulatedAabb.x += params->step * direction.x;
        simulatedAabb.y += params->step * direction.y;

        for (usize i = 0; i < SceneGetTotalAllocatedEntities(params->scene); ++i)
        {
            const u64 dependencies = TAG_POSITION | TAG_DIMENSION | TAG_COLLIDER;

            if (i == params->entity || !SceneEntityHasDependencies(params->scene, i, dependencies))
            {
                continue;
            }

            const CPosition* otherPosition = &params->scene->components.positions[i];
            const CDimension* otherDimension = &params->scene->components.dimensions[i];
            const CCollider* otherCollider = &params->scene->components.colliders[i];

            if ((params->collider->mask & otherCollider->layer) == 0)
            {
                continue;
            }

            const Rectangle otherAabb = (Rectangle)
            {
                .x = otherPosition->value.x,
                .y = otherPosition->value.y,
                .width = otherDimension->width,
                .height = otherDimension->height
            };

            if (CheckCollisionRecs(simulatedAabb, otherAabb))
            {
                const Vector2 rawResolution = Vector2Create(-direction.x, -direction.y);
                const Vector2 resolution = ExtractResolution(rawResolution, otherCollider->resolutionSchema);

                // Check if extracting the resolution also invalidated the resolution.
                if (resolution.x == 0 && resolution.y == 0)
                {
                    continue;
                }

                const Rectangle overlap = GetCollisionRec(simulatedAabb, otherAabb);

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
                    const f32 left = RectangleLeft(simulatedAabb);
                    const f32 top = RectangleTop(simulatedAabb);

                    const f32 otherLeft = RectangleLeft(otherAabb);
                    const f32 otherRight = RectangleRight(otherAabb);
                    const f32 otherTop = RectangleTop(otherAabb);
                    const f32 otherBottom = RectangleBottom(otherAabb);

                    const f32 offsetLeft = (otherLeft - simulatedAabb.width) - left;
                    const f32 offsetRight = otherRight - left;
                    const f32 offsetDown = otherBottom - top;
                    const f32 offsetUp = (otherTop - simulatedAabb.height) - top;

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

                const OnResolutionParams onResolutionParams = (OnResolutionParams)
                {
                    .scene = params->scene,
                    .entity = params->entity,
                    .aabb = simulatedAabb,
                    .otherEntity = i,
                    .otherAabb = otherAabb,
                    .overlap = overlap,
                    .resolution = resolution,
                };

                const OnResolutionResult result = params->onResolution(&onResolutionParams);

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
    const Vector2 previousPosition = (Vector2)
    {
        .x = params->previousAabb.x,
        .y = params->previousAabb.y,
    };
    const Vector2 currentPosition = (Vector2)
    {
        .x = params->currentAabb.x,
        .y = params->currentAabb.y,
    };

    const Vector2 delta = Vector2Subtract(currentPosition, previousPosition);

    Rectangle simulatedAabb = params->previousAabb;
    bool xModified = false;
    bool yModified = false;

    static const u8 step = 1;

    // Simulate just the x-axis.
    {
        const Vector2 xDelta = (Vector2)
        {
            .x = delta.x,
            .y = 0,
        };

        const SimulateCollisionOnAxisParams onAxisParams = (SimulateCollisionOnAxisParams)
        {
            .scene = (Scene*)params->scene,
            .entity = params->entity,
            .aabb = simulatedAabb,
            .collider = params->collider,
            .delta = xDelta,
            .step = step,
            .onResolution = params->onResolution,
        };

        const SimulateCollisionOnAxisResult result = SimulateCollisionOnAxis(&onAxisParams);

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
        const Vector2 yDelta = (Vector2)
        {
            .x = 0,
            .y = delta.y,
        };

        const SimulateCollisionOnAxisParams onAxisParams = (SimulateCollisionOnAxisParams)
        {
            .scene = (Scene*)params->scene,
            .entity = params->entity,
            .aabb = simulatedAabb,
            .collider = params->collider,
            .delta = yDelta,
            .step = step,
            .onResolution = params->onResolution,
        };

        const SimulateCollisionOnAxisResult result = SimulateCollisionOnAxis(&onAxisParams);

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
    static const u64 dependencies = TAG_SMOOTH | TAG_POSITION | TAG_DIMENSION | TAG_COLLIDER;

    if (!SceneEntityHasDependencies(scene, entity, dependencies))
    {
        return;
    }

    const CSmooth* smooth = &scene->components.smooths[entity];
    CPosition* position = &scene->components.positions[entity];
    const CDimension* dimension = &scene->components.dimensions[entity];
    const CCollider* collider = &scene->components.colliders[entity];

    const Rectangle previousAabb = (Rectangle)
    {
        .x = smooth->previous.x,
        .y = smooth->previous.y,
        .width = dimension->width,
        .height = dimension->height
    };
    const Rectangle currentAabb = (Rectangle)
    {
        .x = position->value.x,
        .y = position->value.y,
        .width = dimension->width,
        .height = dimension->height
    };

    const AdvancedCollisionParams params = (AdvancedCollisionParams)
    {
        .scene = scene,
        .entity = entity,
        .currentAabb = currentAabb,
        .previousAabb = previousAabb,
        .collider = (CCollider*)collider,
        .onResolution = collider->onResolution
    };

    const Rectangle resolvedAabb = AdvancedCollision(&params);

    position->value.x = resolvedAabb.x;
    position->value.y = resolvedAabb.y;
}

void SPostCollisionUpdate(Scene* scene, const usize entity)
{
    static const u64 dependencies = TAG_POSITION | TAG_DIMENSION | TAG_COLLIDER;

    if (!SceneEntityHasDependencies(scene, entity, dependencies))
    {
        return;
    }

    CPosition* position = &scene->components.positions[entity];
    const CDimension* dimension = &scene->components.dimensions[entity];
    const CCollider* collider = &scene->components.colliders[entity];

    const Rectangle aabb = (Rectangle)
    {
        .x = position->value.x,
        .y = position->value.y,
        .width = dimension->width,
        .height = dimension->height,
    };

    for (usize i = 0; i < SceneGetTotalAllocatedEntities(scene); ++i)
    {
        const u64 otherDependencies = TAG_POSITION | TAG_DIMENSION | TAG_COLLIDER;

        if (i == entity || !SceneEntityHasDependencies(scene, i, otherDependencies))
        {
            continue;
        }

        const CPosition* otherPosition = &scene->components.positions[i];
        const CDimension* otherDimension = &scene->components.dimensions[i];
        const CCollider* otherCollider = &scene->components.colliders[i];

        if ((collider->mask & otherCollider->layer) == 0)
        {
            continue;
        }

        const Rectangle otherAabb = (Rectangle)
        {
            .x = otherPosition->value.x,
            .y = otherPosition->value.y,
            .width = otherDimension->width,
            .height = otherDimension->height
        };

        if (CheckCollisionRecs(aabb, otherAabb))
        {
            const Rectangle overlap = GetCollisionRec(aabb, otherAabb);

            const OnCollisionParams onCollisionParams = (OnCollisionParams)
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
    static const u64 dependencies = TAG_FLEETING;

    if (!SceneEntityHasDependencies(scene, entity, dependencies))
    {
        return;
    }

    CFleeting* fleeting = &scene->components.fleetings[entity];

    fleeting->age += CTX_DT;

    if (fleeting->age >= fleeting->lifetime)
    {
        SceneDeferDeallocateEntity(scene, entity);
    }
}

void SAnimationUpdate(Scene* scene, const usize entity)
{
    static const u64 dependencies = TAG_ANIMATION;

    if (!SceneEntityHasDependencies(scene, entity, dependencies))
    {
        return;
    }

    CAnimation* animation = &scene->components.animations[entity];

    animation->frameTimer += CTX_DT;

    if (animation->frameTimer >= animation->frameDuration)
    {
        animation->frameTimer = 0.0f;
        animation->frame = (animation->frame + 1) % animation->length;
    }
}

static void DrawCSprite
(
    const Atlas* atlas,
    const CSprite* sprite,
    const Vector2 position,
    const Color tint
)
{
    const AtlasDrawParams params = (AtlasDrawParams)
    {
        .sprite = sprite->type,
        .position = position,
        .intramural = sprite->intramural,
        .reflection = sprite->reflection,
        .tint = tint,
    };
    AtlasDraw(atlas, &params);
}

void SSpriteDraw(const Scene* scene, const usize entity)
{
    static const u64 dependencies = TAG_POSITION | TAG_SPRITE;

    if (!SceneEntityHasDependencies(scene, entity, dependencies))
    {
        return;
    }

    const CPosition* position = &scene->components.positions[entity];
    const CSprite* sprite = &scene->components.sprites[entity];

    Vector2 drawPosition = position->value;

    if (SceneEntityHasDependencies(scene, entity, TAG_SMOOTH))
    {
        const CSmooth* smooth = &scene->components.smooths[entity];

        const Vector2 interpolated = Vector2Lerp(smooth->previous, position->value, ContextGetAlpha());
        drawPosition = interpolated;
    }

    if (SceneEntityHasDependencies(scene, entity, TAG_COLOR))
    {
        const CColor* color = &scene->components.colors[entity];

        DrawCSprite(&scene->atlas, sprite, drawPosition, color->value);
    }
    else
    {
        DrawCSprite(&scene->atlas, sprite, drawPosition, COLOR_WHITE);
    }
}

static void DrawCAnimation
(
    const Atlas* atlas,
    const CAnimation* animation,
    const Vector2 position,
    const Color tint
)
{
    const AtlasDrawParams params = (AtlasDrawParams)
    {
        .sprite = ANIMATIONS[animation->type][animation->frame],
        .position = position,
        .intramural = animation->intramural,
        .reflection = animation->reflection,
        .tint = tint,
    };
    AtlasDraw(atlas, &params);
}

void SAnimationDraw(const Scene* scene, const usize entity)
{
    static const u64 dependencies = TAG_POSITION | TAG_ANIMATION;

    if (!SceneEntityHasDependencies(scene, entity, dependencies))
    {
        return;
    }

    const CPosition* position = &scene->components.positions[entity];
    const CAnimation* animation = &scene->components.animations[entity];

    Vector2 drawPosition = position->value;

    if (SceneEntityHasDependencies(scene, entity, TAG_SMOOTH))
    {
        const CSmooth* smooth = &scene->components.smooths[entity];

        const Vector2 interpolated = Vector2Lerp(smooth->previous, position->value, ContextGetAlpha());
        drawPosition = interpolated;
    }

    if (SceneEntityHasDependencies(scene, entity, TAG_COLOR))
    {
        const CColor* color = &scene->components.colors[entity];

        DrawCAnimation(&scene->atlas, animation, drawPosition, color->value);
    }
    else
    {
        DrawCAnimation(&scene->atlas, animation, drawPosition, COLOR_WHITE);
    }
}

static void ColliderDrawLayerBoundaries(const CCollider* self, const Rectangle aabb)
{
    static const f32 alpha = 0.75;

    if ((self->layer & LAYER_TERRAIN) == LAYER_TERRAIN)
    {
        const Color color = P8_DARK_GREEN;
        DrawRectangleRec(aabb, ColorAlpha(color, alpha));
        DrawRectangleLinesEx(aabb, 4, color);
    }

    if ((self->layer & LAYER_LETHAL) == LAYER_LETHAL)
    {
        const Color color = P8_YELLOW;
        DrawRectangleRec(aabb, ColorAlpha(color, alpha));
        DrawRectangleLinesEx(aabb, 1, color);
    }

    if ((self->layer & LAYER_INTERACTABLE) == LAYER_INTERACTABLE)
    {
        const Color color = P8_ORANGE;
        DrawRectangleRec(aabb, ColorAlpha(color, alpha));
        DrawRectangleLinesEx(aabb, 2, color);
    }

    if ((self->layer & LAYER_INVISIBLE) == LAYER_INVISIBLE)
    {
        const Color color = P8_LAVENDER;
        DrawRectangleRec(aabb, ColorAlpha(color, alpha));
        DrawRectangleLinesEx(aabb, 2, color);
    }
}

static void ColliderDrawResolutionSchema
(
    const CCollider* self,
    const Rectangle aabb,
    const Color color
)
{
    static const usize thickness = 2;

    if (self->resolutionSchema == RESOLVE_ALL)
    {
        DrawRectangleLinesEx(aabb, thickness, color);

        return;
    }

    if ((self->resolutionSchema & RESOLVE_UP) == RESOLVE_UP)
    {
        const Rectangle edge = (Rectangle)
        {
            .x = aabb.x,
            .y = aabb.y,
            .width = aabb.width,
            .height = thickness,
        };
        DrawRectangleRec(edge, color);
    }

    if ((self->resolutionSchema & RESOLVE_RIGHT) == RESOLVE_RIGHT)
    {
        const Rectangle edge = (Rectangle)
        {
            .x = RectangleRight(aabb) - thickness,
            .y = aabb.y,
            .width = thickness,
            .height = aabb.height,
        };
        DrawRectangleRec(edge, color);
    }

    if ((self->resolutionSchema & RESOLVE_DOWN) == RESOLVE_DOWN)
    {
        const Rectangle edge = (Rectangle)
        {
            .x = aabb.x,
            .y = RectangleBottom(aabb),
            .width = aabb.width,
            .height = thickness,
        };
        DrawRectangleRec(edge, color);
    }

    if ((self->resolutionSchema & RESOLVE_LEFT) == RESOLVE_LEFT)
    {
        const Rectangle edge = (Rectangle)
        {
            .x = aabb.x,
            .y = aabb.y,
            .width = thickness,
            .height = aabb.height,
        };
        DrawRectangleRec(edge, color);
    }
}

void SDebugColliderDraw(const Scene* scene, const usize entity)
{
    static const u64 dependencies = TAG_POSITION | TAG_DIMENSION | TAG_COLLIDER;

    if (!SceneEntityHasDependencies(scene, entity, dependencies))
    {
        return;
    }

    const CPosition* position = &scene->components.positions[entity];
    const CDimension* dimension = &scene->components.dimensions[entity];
    const CCollider* collider = &scene->components.colliders[entity];

    const Rectangle aabb = (Rectangle)
    {
        .x = position->value.x,
        .y = position->value.y,
        .width = dimension->width,
        .height = dimension->height
    };

    ColliderDrawLayerBoundaries(collider, aabb);

    if ((collider->layer & LAYER_TERRAIN) == LAYER_TERRAIN)
    {
        ColliderDrawResolutionSchema(collider, aabb, P8_GREEN);
    }
}
