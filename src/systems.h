#pragma once

#include "scene.h"

void SSmoothUpdate(Scene* scene, const usize entity);
void SKineticUpdate(Scene* scene, const usize entity);
void SCollisionUpdate(Scene* scene, const usize entity);

void SPlayerInputUpdate(Scene* scene, const usize entity);
void SPlayerCollisionUpdate(Scene* scene, const usize entity);
void SPlayerMortalUpdate(Scene* scene, const usize entity);

void SWalkerCollisionUpdate(Scene* scene, const usize entity);

void SFleetingUpdate(Scene* scene, const usize entity);

void SGenericCollisionUpdate(Scene* scene, const usize entity);

void SCloudParticleCollisionUpdate(Scene* scene, const usize entity);
void SCloudParticleSpawnUpdate(Scene* scene, const usize entity);

void SCloudParticleDraw(const Scene* scene, const usize entity);

void SSpriteDraw(const Scene* scene, const usize entity);
void SDebugDraw(const Scene* scene, const usize entity);
