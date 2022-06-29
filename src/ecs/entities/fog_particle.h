#pragma once

#include "../../scene.h"
#include "../entity_builder.h"

EntityBuilder FogBreathingParticleCreate
(
    Vector2 position,
    f32 radius,
    f32 lifetime
);

void FogParticleUpdate(Scene* scene, const usize entity);
void FogParticleDraw(const Scene* scene, usize entity);
