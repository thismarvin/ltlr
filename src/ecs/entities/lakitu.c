#include "common.h"
#include "lakitu.h"

EntityBuilder LakituCreate(void)
{
    Deque components = DEQUE_OF(Component);

    const u64 tags =
        TAG_NONE
        | TAG_POSITION
        | TAG_KINETIC
        | TAG_SMOOTH;

    const Vector2 position = (Vector2)
    {
        .x = CTX_VIEWPORT_WIDTH * 0.5,
        .y = CTX_VIEWPORT_HEIGHT * 0.5,
    };

    ADD_COMPONENT(CPosition, ((CPosition)
    {
        .value = position,
    }));

    ADD_COMPONENT(CKinetic, ((CKinetic)
    {
        .velocity = Vector2Create(100, 0),
        .acceleration = VECTOR2_ZERO,
    }));

    ADD_COMPONENT(CSmooth, ((CSmooth)
    {
        .previous = position,
    }));

    return (EntityBuilder)
    {
        .tags = tags,
        .components = components,
    };
}
