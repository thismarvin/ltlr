#pragma once

#include "../../scene.h"
#include "../entity_builder.h"

EntityBuilder CloudParticleCreate
(
    Vector2 position,
    f32 radius,
    Vector2 initialVelocity,
    Vector2 acceleration,
    f32 lifetime
);

void CloudParticleDraw(const Scene* scene, usize entity);
