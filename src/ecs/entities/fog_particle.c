#include "fog_particle.h"
#include <raymath.h>

void FogParticleBuildHelper(Scene* scene, const FogParticleBuilder* builder)
{
    scene->components.tags[builder->entity] =
        TAG_NONE
        | TAG_POSITION
        | TAG_DIMENSION
        | TAG_KINETIC
        | TAG_SMOOTH
        | TAG_FLEETING
        | TAG_FOG_PARTICLE;

    scene->components.positions[builder->entity] = (CPosition)
    {
        .value = builder->position,
    };

    scene->components.dimensions[builder->entity] = (CDimension)
    {
        .width = builder->radius * 2,
        .height = builder->radius * 2,
    };

    scene->components.kinetics[builder->entity] = (CKinetic)
    {
        .velocity = builder->velocity,
        .acceleration = VECTOR2_ZERO,
    };

    scene->components.smooths[builder->entity] = (CSmooth)
    {
        .previous = builder->position,
    };

    scene->components.fleetings[builder->entity] = (CFleeting)
    {
        .lifetime = builder->lifetime,
        .age = 0,
    };
}

void FogParticleBuild(Scene* scene, const void* params)
{
    FogParticleBuildHelper(scene, params);
}

void FogParticleDraw(const Scene* scene, const usize entity)
{
    static const u64 dependencies =
        TAG_NONE
        | TAG_POSITION
        | TAG_DIMENSION
        | TAG_FLEETING
        | TAG_SMOOTH
        | TAG_FOG_PARTICLE;

    if (!SceneEntityHasDependencies((Scene*)scene, entity, dependencies))
    {
        return;
    }

    const CPosition* position = &scene->components.positions[entity];
    const CFleeting* fleeting = &scene->components.fleetings[entity];
    const CDimension* dimension = &scene->components.dimensions[entity];
    const CSmooth* smooth = &scene->components.smooths[entity];

    const f32 progress = fleeting->age / fleeting->lifetime;
    const f32 scale = -4 * (progress * progress - progress);

    const Vector2 interpolated = Vector2Lerp(smooth->previous, position->value, ContextGetAlpha());

    const Vector2 center = (Vector2)
    {
        .x = interpolated.x + dimension->width * 0.5f,
        .y = interpolated.y + dimension->height * 0.5f,
    };

    const u32 sidesCount = 4 + entity % 3;

    f32 rotation = ContextGetTotalTime() * 100;

    if (sidesCount % 2 == 0)
    {
        rotation *= -1;
    }

    DrawPoly(center, sidesCount, dimension->width * scale * 0.5f, rotation, COLOR_BLACK);
}
