#pragma once

#include "entity_builder.h"

#include "entities/player.h"
EntityBuilder BlockCreate(f32 x, f32 y, f32 width, f32 height);
EntityBuilder WalkerCreate(f32 x, f32 y);
EntityBuilder CloudParticleCreate(f32 centerX, f32 centerY, Vector2 direction);
