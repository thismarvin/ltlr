#pragma once

#include "../../common.h"
#include "../../context.h"
#include "../../scene.h"
#include "../components.h"
#include <assert.h>

#define ADD_COMPONENT(mType, mValue) DEQUE_PUSH_FRONT(&components, Component, ComponentCreate##mType(mValue))

Rectangle ApplyResolutionPerfectly
(
    Rectangle aabb,
    Rectangle otherAabb,
    Vector2 resolution
);

void OnCollisionNoop(const OnCollisionParams* params);
OnResolutionResult OnResolutionNoop(const OnResolutionParams* params);
