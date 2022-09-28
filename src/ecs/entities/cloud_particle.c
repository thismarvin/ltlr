#include "cloud_particle.h"
#include "common.h"
#include <raymath.h>

// TODO(thismarvin): The collider no longer scales...
static OnResolutionResult CloudParticleOnResolution(const OnResolutionParams* params)
{
    assert(ENTITY_HAS_DEPS(params->entity, TAG_POSITION));

    const CPosition* position = GET_COMPONENT(position, params->entity);

    // If the aabb is completely within another collider then remove it.
    if (params->overlap.width >= params->aabb.width && params->overlap.height >= params->aabb.height)
    {
        SceneDeferDeallocateEntity(params->scene, params->entity);

        return (OnResolutionResult)
        {
            .aabb = params->aabb,
        };
    }

    // Resolve collision.
    const Rectangle resolvedAabb = ApplyResolutionPerfectly(params->aabb, params->otherAabb,
                                   params->resolution);

    return (OnResolutionResult)
    {
        .aabb = resolvedAabb,
    };
}

EntityBuilder CloudParticleCreate
(
    const Vector2 position,
    const f32 radius,
    const Vector2 initialVelocity,
    const Vector2 acceleration,
    const f32 lifetime
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
        | TAG_FLEETING
        | TAG_CLOUD_PARTICLE;

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

    ADD_COMPONENT(CKinetic, ((CKinetic)
    {
        .velocity = initialVelocity,
        .acceleration = acceleration,
    }));

    ADD_COMPONENT(CSmooth, ((CSmooth)
    {
        .previous = position,
    }));

    ADD_COMPONENT(CCollider, ((CCollider)
    {
        .resolutionSchema = RESOLVE_NONE,
        .layer = LAYER_NONE,
        .mask = LAYER_TERRAIN,
        .onCollision = OnCollisionNoop,
        .onResolution = CloudParticleOnResolution,
    }));

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
    const u64 dependencies =
        TAG_NONE
        | TAG_POSITION
        | TAG_DIMENSION
        | TAG_FLEETING
        | TAG_COLOR
        | TAG_SMOOTH
        | TAG_CLOUD_PARTICLE;

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

    const Vector2 interpolated = Vector2Lerp(smooth->previous, position->value, ContextGetAlpha());

    const Vector2 center = (Vector2)
    {
        .x = interpolated.x + dimension->width * 0.5f,
        .y = interpolated.y + dimension->height * 0.5f,
    };

    DrawCircleV(center, drawSize * 0.5f, color->value);
}

