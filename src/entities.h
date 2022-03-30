#pragma once

#include "common.h"
#include "scene.h"

usize ECreatePlayer(Scene* scene, f32 x, f32 y);
usize ECreateBlock(Scene* scene, f32 x, f32 y, f32 width, f32 height);
usize ECreateWalker(Scene* scene, f32 x, f32 y);
usize ECreateCloudParticle(Scene* scene, f32 x, f32 y, Vector2 direction);
