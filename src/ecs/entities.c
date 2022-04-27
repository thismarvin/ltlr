#include "../context.h"
#include "../geometry/collider.h"
#include "../scene.h"
#include "components.h"
#include "entities.h"
#include <assert.h>
#include <raymath.h>

#define ADD_COMPONENT(mType, mValue) DEQUE_PUSH_FRONT(&components, Component, ComponentCreate##mType(mValue))

// TODO(thismarvin): We need some of the macros from systems.c...
#define GET_COMPONENT(mValue, mEntity) SCENE_GET_COMPONENT_PTR(params->scene, mEntity, mValue)
#define ENTITY_HAS_DEPS(mEntity, mDependencies) ((params->scene->components.tags[mEntity] & (mDependencies)) == (mDependencies))

static void ApplyResolutionPerfectly
(
    CPosition* position,
    const Rectangle aabb,
    const Rectangle otherAabb,
    const Vector2 resolution
)
{
    if (resolution.x < 0)
    {
        position->value.x = RectangleLeft(otherAabb) - aabb.width;
    }
    else if (resolution.x > 0)
    {
        position->value.x = RectangleRight(otherAabb);
    }

    if (resolution.y < 0)
    {
        position->value.y = RectangleTop(otherAabb) - aabb.height;
    }
    else if (resolution.y > 0)
    {
        position->value.y = RectangleBottom(otherAabb);
    }
}

