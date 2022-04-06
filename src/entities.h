#pragma once

#include "common.h"
#include "scene.h"

usize ECreatePlayer(Scene* scene, const f32 x, const f32 y);
usize ECreateBlock(Scene* scene, const f32 x, const f32 y, const f32 width, const f32 height);
usize ECreateWalker(Scene* scene, const f32 x, const f32 y);
usize ECreateCloudParticle
(
    Scene* scene,
    const f32 centerX,
    const f32 centerY,
    const Vector2 direction
);
