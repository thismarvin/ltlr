#pragma once

#include "scene.h"

void SCloudParticleEmitter(Scene* scene, usize eventIndex);

void SSmoothUpdate(Scene* scene, usize entity);
void SKineticUpdate(Scene* scene, usize entity);
void SCollisionUpdate(Scene* scene, usize entity);

void SPlayerInputUpdate(Scene* scene, usize entity);
// TODO(thismarvin): This system definitely needs a better name...
void SPlayerCollisionUpdate(Scene* scene, usize entity);
// TODO(thismarvin): This system also might need a new name?
void SPlayerMortalUpdate(Scene* scene, usize entity);

void SFleetingUpdate(Scene* scene, usize entity);

void SCloudParticleDraw(const Scene* scene, usize entity);
void SSpriteDraw(const Scene* scene, usize entity);
void SDebugDraw(const Scene* scene, usize entity);
