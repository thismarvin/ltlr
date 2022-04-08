#pragma once

#include "scene.h"

void SSmoothUpdate(Scene* scene, usize entity);
void SKineticUpdate(Scene* scene, usize entity);
void SCollisionUpdate(Scene* scene, usize entity);

void SPlayerInputUpdate(Scene* scene, usize entity);
void SPlayerCollisionUpdate(Scene* scene, usize entity);
void SPlayerMortalUpdate(Scene* scene, usize entity);

void SWalkerCollisionUpdate(Scene* scene, usize entity);

void SFleetingUpdate(Scene* scene, usize entity);

void SGenericCollisionUpdate(Scene* scene, usize entity);

void SCloudParticleCollisionUpdate(Scene* scene, usize entity);
void SCloudParticleSpawnUpdate(Scene* scene, usize entity);

void SCloudParticleDraw(const Scene* scene, usize entity);

void SSpriteDraw(const Scene* scene, usize entity);
void SDebugDraw(const Scene* scene, usize entity);
