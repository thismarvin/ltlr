#pragma once

#include "../../common.h"
#include "../../context.h"
#include "../../scene.h"
#include "../components.h"

Rectangle ApplyResolutionPerfectly
(
    Rectangle aabb,
    Rectangle otherAabb,
    Vector2 resolution
);

void OnCollisionNoop(const OnCollisionParams* params);
OnResolutionResult OnResolutionNoop(const OnResolutionParams* params);
