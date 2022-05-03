#pragma once

#include "../../common.h"
#include "../../context.h"
#include "../../scene.h"
#include "../components.h"
#include <assert.h>

#define ADD_COMPONENT(mType, mValue) DEQUE_PUSH_FRONT(&components, Component, ComponentCreate##mType(mValue))
#define ENTITY_HAS_DEPS(mEntity, mDependencies) ((params->scene->components.tags[mEntity] & (mDependencies)) == (mDependencies))
#define GET_COMPONENT(mValue, mEntity) SCENE_GET_COMPONENT_PTR(params->scene, mValue, mEntity)

void ApplyResolutionPerfectly
(
    CPosition* position,
    Rectangle aabb,
    Rectangle otherAabb,
    Vector2 resolution
);
