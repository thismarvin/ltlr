#include "cloud_particle.h"
#include "common.h"
#include <raymath.h>

// TODO(thismarvin): The collider no longer scales...
static OnCollisionResult CloudParticleOnCollision(const OnCollisionParams* params)
{
    assert(ENTITY_HAS_DEPS(params->entity, TAG_POSITION));

    const CPosition* position = GET_COMPONENT(position, params->entity);

    // If the aabb is completely within another collider then remove it.
    if (params->overlap.width >= params->aabb.width && params->overlap.height >= params->aabb.height)
    {
        SceneDeferDeallocateEntity(params->scene, params->entity);

        return (OnCollisionResult)
        {
            .aabb = params->aabb,
        };
    }

    // Resolve collision.
    Rectangle resolvedAabb = ApplyResolutionPerfectly(params->aabb, params->otherAabb,
                             params->resolution);

    return (OnCollisionResult)
    {
        .aabb = resolvedAabb,
    };
}

EntityBuilder CloudParticleCreate
(
    const f32 centerX,
    const f32 centerY,
    const Vector2 direction
)
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
        | TAG_FLEETING;

    f32 radius = 1;

    if (GetRandomValue(1, 100) < 25)
    {
        radius = (f32) GetRandomValue(4, 5);
    }
    else
    {
        radius = (f32)GetRandomValue(1, 3);
    }

    Vector2 position = Vector2Create(centerX - radius, centerY - radius);

    ADD_COMPONENT(CPosition, ((CPosition)
    {
        .value = position,
    }));

    ADD_COMPONENT(CDimension, ((CDimension)
    {
        .width = radius * 2,
        .height = radius * 2,
    }));

    ADD_COMPONENT(CColor, ((CColor)
    {
        .value = COLOR_WHITE,
    }));

    f32 speed = (f32)GetRandomValue(5, 15);

    ADD_COMPONENT(CKinetic, ((CKinetic)
    {
        .velocity = Vector2Scale(direction, speed),
        .acceleration = Vector2Create(0, 15),
    }));

    ADD_COMPONENT(CSmooth, ((CSmooth)
    {
        .previous = position,
    }));

    ADD_COMPONENT(CCollider, ((CCollider)
    {
        .layer = LAYER_NONE,
        .mask = LAYER_ALL,
        .onCollision = CloudParticleOnCollision,
    }));

    f32 lifetime = MIN(1.0f, (f32)GetRandomValue(1, 100) * 0.03f);

    ADD_COMPONENT(CFleeting, ((CFleeting)
    {
        .lifetime = lifetime,
        .age = 0,
    }));

    return (EntityBuilder)
    {
        .tags = tags,
        .components = components,
    };
}

void CloudParticleDraw(const Scene* scene, const usize entity)
{
    const u64 dependencies = TAG_POSITION | TAG_DIMENSION | TAG_FLEETING | TAG_COLOR | TAG_SMOOTH;

    if (!SceneEntityHasDependencies((Scene*)scene, entity, dependencies))
    {
        return;
    }

    const CPosition* position = SCENE_GET_COMPONENT_PTR(scene, position, entity);
    const CColor* color = SCENE_GET_COMPONENT_PTR(scene, color, entity);
    const CFleeting* fleeting = SCENE_GET_COMPONENT_PTR(scene, fleeting, entity);
    const CDimension* dimension = SCENE_GET_COMPONENT_PTR(scene, dimension, entity);
    const CSmooth* smooth = SCENE_GET_COMPONENT_PTR(scene, smooth, entity);

    const f32 drawSize = dimension->width * (fleeting->lifetime - fleeting->age) / fleeting->lifetime;

    Vector2 interpolated = Vector2Lerp(smooth->previous, position->value, ContextGetAlpha());

    Vector2 center = (Vector2)
    {
        .x = interpolated.x + dimension->width * 0.5f,
        .y = interpolated.y + dimension->height * 0.5f,
    };

    DrawCircleV(center, drawSize * 0.5f, color->value);
}

