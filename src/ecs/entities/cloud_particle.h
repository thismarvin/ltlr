#pragma once

#include "../../common.h"
#include "../../level.h"

#include <raylib.h>

typedef struct
{
	usize entity;
	Vector2 position;
	f32 radius;
	Vector2 initialVelocity;
	Vector2 acceleration;
	f32 lifetime;
} CloudParticleBuilder;

void CloudParticleBuild(Scene* scene, const void* params);

void CloudParticleDraw(const Scene* scene, usize entity);
