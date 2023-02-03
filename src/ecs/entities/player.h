#pragma once

#include "common.h"

#define PLAYER_MAX_HIT_POINTS (5)

typedef struct
{
	usize entity;
	f32 x;
	f32 y;
} PlayerBuilder;

void PlayerBuild(Scene* scene, const void* params);

void PlayerInputUpdate(Scene* scene, usize entity);
void PlayerPostCollisionUpdate(Scene* scene, usize entity);
void PlayerMortalUpdate(Scene* scene, usize entity);
void PlayerAnimationUpdate(Scene* scene, usize entity);
void PlayerTrailUpdate(Scene* scene, usize entity);
void PlayerShadowUpdate(Scene* scene, usize entity);

void PlayerDebugDraw(const Scene* scene, usize entity);
