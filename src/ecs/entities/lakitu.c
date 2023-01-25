#include "lakitu.h"

void LakituCreateHelper(Scene* scene, const LakituBuilder* builder)
{
    const Vector2 position = (Vector2)
    {
        .x = CTX_VIEWPORT_WIDTH * 0.5,
        .y = CTX_VIEWPORT_HEIGHT * 0.5,
    };

    scene->components.tags[builder->entity] =
        TAG_NONE
        | TAG_POSITION
        | TAG_KINETIC
        | TAG_SMOOTH;

    scene->components.positions[builder->entity] = (CPosition)
    {
        .value = position,
    };

    scene->components.kinetics[builder->entity] = (CKinetic)
    {
        .velocity = Vector2Create(100, 0),
        .acceleration = VECTOR2_ZERO,
    };

    scene->components.smooths[builder->entity] = (CSmooth)
    {
        .previous = position,
    };
}

void LakituCreate(Scene* scene, const void* params)
{
    LakituCreateHelper(scene, params);
}
