#pragma once

#include "entity_builder.h"

#include "./entities/player.h"
#include "./entities/cloud_particle.h"

EntityBuilder BlockCreate(f32 x, f32 y, f32 width, f32 height);
EntityBuilder WalkerCreate(f32 x, f32 y);
