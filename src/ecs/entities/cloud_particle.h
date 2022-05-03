#pragma once

#include "../../scene.h"
#include "../entity_builder.h"

EntityBuilder CloudParticleCreate(f32 centerX, f32 centerY, Vector2 direction);

void CloudParticleDraw(const Scene* scene, usize entity);
