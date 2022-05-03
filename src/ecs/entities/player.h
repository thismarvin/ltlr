#pragma once

#include "../../scene.h"
#include "../entity_builder.h"

EntityBuilder PlayerCreate(f32 x, f32 y);

void PlayerInputUpdate(Scene* scene, usize entity);
void PlayerPostCollisionUpdate(Scene* scene, usize entity);
void PlayerMortalUpdate(Scene* scene, usize entity);
