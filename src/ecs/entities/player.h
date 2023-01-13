#pragma once

#include "../../scene.h"
#include "../entity_builder.h"

#define PLAYER_MAX_HIT_POINTS (5)

EntityBuilder PlayerCreate(f32 x, f32 y);

void PlayerInputUpdate(Scene* scene, usize entity);
void PlayerPostCollisionUpdate(Scene* scene, usize entity);
void PlayerMortalUpdate(Scene* scene, usize entity);
void PlayerAnimationUpdate(Scene* scene, usize entity);
