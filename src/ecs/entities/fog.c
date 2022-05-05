#include "../components.h"
#include "common.h"
#include "fog.h"
#include <math.h>
#include <raymath.h>

#define FOG_HEIGHT CTX_VIEWPORT_HEIGHT * 2
#define FOG_INITIAL_POSITION (Vector2) \
{ \
    .x = -CTX_VIEWPORT_WIDTH, \
    .y = -(FOG_HEIGHT - CTX_VIEWPORT_HEIGHT) * 0.5f, \
}

static OnCollisionResult FogOnCollision()
{
    // decrease player health?
}

EntityBuilder FogCreate()
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
        | TAG_FOG;

    ADD_COMPONENT(CPosition, ((CPosition)
    {
        .value = FOG_INITIAL_POSITION,
    }));

    ADD_COMPONENT(CDimension, ((CDimension)
    {
        .width = 32,
        .height = FOG_HEIGHT,
    }));

    ADD_COMPONENT(CColor, ((CColor)
    {
        .value = COLOR_BLACK,
    }));

    ADD_COMPONENT(CKinetic, ((CKinetic)
    {
        .velocity = Vector2Create(50, 0),
        .acceleration = VECTOR2_ZERO,
    }));

    ADD_COMPONENT(CSmooth, ((CSmooth)
    {
        .previous = FOG_INITIAL_POSITION,
    }));

    ADD_COMPONENT(CCollider, ((CCollider)
    {
        .layer = LAYER_ALL,
        .mask = LAYER_NONE,
        .onCollision = FogOnCollision,
    }));

    return (EntityBuilder)
    {
        .tags = tags,
        .components = components,
    };
}

void FogUpdate(Scene* scene, const usize entity)
{
    const u64 dependencies = TAG_FOG | TAG_POSITION;

    if (!SceneEntityHasDependencies(scene, entity, dependencies))
    {
        return;
    }

    CPosition* position = SCENE_GET_COMPONENT_PTR(scene, position, entity);

    position->value.y = sinf(position->value.x * CTX_DT) * 48 + FOG_INITIAL_POSITION.y;
}

void FogDraw(const Scene* scene, const usize entity)
{
    u64 dependencies = TAG_FOG | TAG_POSITION | TAG_DIMENSION | TAG_COLOR | TAG_SMOOTH;

    if (!SceneEntityHasDependencies(scene, entity, dependencies))
    {
        return;
    }

    const CPosition* position = SCENE_GET_COMPONENT_PTR(scene, position, entity);
    const CDimension* dimension = SCENE_GET_COMPONENT_PTR(scene, dimension, entity);
    const CColor* color = SCENE_GET_COMPONENT_PTR(scene, color, entity);
    const CSmooth* smooth = SCENE_GET_COMPONENT_PTR(scene, smooth, entity);

    const Vector2 interpolated = Vector2Lerp(smooth->previous, position->value, ContextGetAlpha());

    DrawRectangle(interpolated.x - CTX_VIEWPORT_WIDTH * 2, interpolated.y, CTX_VIEWPORT_WIDTH * 2,
                  dimension->height, color->value);

    Vector2 currentCenter = Vector2Create(interpolated.x + dimension->width * 0.5f, interpolated.y);

    const f32 radius = 32.0f;

    while (currentCenter.y <= position->value.y + dimension->height)
    {
        DrawCircle(currentCenter.x, currentCenter.y, radius, color->value);
        currentCenter.y += radius * 1.5f;
    }
}
