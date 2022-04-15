#pragma once

#include "common.h"
#include "deque.h"

Deque ECreatePlayer(f32 x, f32 y);
Deque ECreateBlock(f32 x, f32 y, f32 width, f32 height);
Deque ECreateWalker(f32 x, f32 y);
Deque ECreateCloudParticle(f32 centerX, f32 centerY, Vector2 direction);
