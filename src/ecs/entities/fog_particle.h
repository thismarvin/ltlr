#pragma once

#include "../../common.h"
#include "../../level.h"

#include <raylib.h>

typedef struct
{
	usize entity;
	Vector2 position;
	Vector2 velocity;
	f32 radius;
	f32 lifetime;
} FogParticleBuilder;

void FogParticleBuild(Scene* scene, const void* params);

void FogParticleDraw(const Scene* scene, usize entity);
