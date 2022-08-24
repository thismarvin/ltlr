#include "fog_particle.h"
#include "common.h"
#include <raymath.h>

static EntityBuilder FogBaseParticleCreate
(
    const Vector2 position,
    const f32 radius,
    const f32 lifetime,
    const Color color,
    const Vector2 velocity
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
        | TAG_FLEETING
        | TAG_FOG_PARTICLE;

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
        .value = color,
    }));

    ADD_COMPONENT(CKinetic, ((CKinetic)
    {
        .velocity = velocity,
        .acceleration = VECTOR2_ZERO,
    }));

    ADD_COMPONENT(CSmooth, ((CSmooth)
    {
        .previous = position,
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

EntityBuilder FogBreathingParticleCreate(Vector2 position, f32 radius, f32 lifetime)
{
    EntityBuilder result = FogBaseParticleCreate(position, radius, lifetime, COLOR_BLACK, VECTOR2_ZERO);

    result.tags |= TAG_FOG_BREATHING;

    return result;
}

EntityBuilder FogMovingParticleCreate(Vector2 position, Vector2 velocity, f32 radius, f32 lifetime)
{
    EntityBuilder result = FogBaseParticleCreate(position, radius, lifetime, COLOR_BLACK, velocity);

    result.tags |= TAG_FOG_MOVING;

    return result;
}

void FogParticleUpdate(Scene* scene, const usize entity)
{
    const u64 dependencies = TAG_FOG_PARTICLE | TAG_KINETIC;

    if (!SceneEntityHasDependencies(scene, entity, dependencies))
    {
        return;
    }

    assert(SceneEntityHasDependencies(scene, scene->fog, TAG_KINETIC));

    const CKinetic* fogKinetic = SCENE_GET_COMPONENT_PTR(scene, fogKinetic, scene->fog);

    CKinetic* kinetic = SCENE_GET_COMPONENT_PTR(scene, kinetic, entity);

    if (SceneEntityHasDependencies(scene, entity, TAG_FOG_MOVING))
    {
        return;
    }

    kinetic->velocity = fogKinetic->velocity;
}

void FogParticleDraw(const Scene* scene, const usize entity)
{
    const u64 dependencies =
        TAG_NONE
        | TAG_POSITION
        | TAG_DIMENSION
        | TAG_FLEETING
        | TAG_COLOR
        | TAG_SMOOTH
        | TAG_FOG_PARTICLE;

    if (!SceneEntityHasDependencies((Scene*)scene, entity, dependencies))
    {
        return;
    }

    const CPosition* position = SCENE_GET_COMPONENT_PTR(scene, position, entity);
    const CColor* color = SCENE_GET_COMPONENT_PTR(scene, color, entity);
    const CFleeting* fleeting = SCENE_GET_COMPONENT_PTR(scene, fleeting, entity);
    const CDimension* dimension = SCENE_GET_COMPONENT_PTR(scene, dimension, entity);
    const CSmooth* smooth = SCENE_GET_COMPONENT_PTR(scene, smooth, entity);

    const f32 progress = fleeting->age / fleeting->lifetime;
    const f32 scale = -4 * (progress * progress - progress);

    Vector2 interpolated = Vector2Lerp(smooth->previous, position->value, ContextGetAlpha());

    Vector2 center = (Vector2)
    {
        .x = interpolated.x,
        .y = interpolated.y,
    };

    // DrawCircleV(center, dimension->width * scale * 0.6f, COLOR_WHITE);
    DrawCircleV(center, dimension->width * scale * 0.5f, color->value);
}

