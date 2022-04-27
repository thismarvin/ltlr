#pragma once

#include "../scene.h"

void SSmoothUpdate(Scene* scene, usize entity);
void SKineticUpdate(Scene* scene, usize entity);
void SCollisionUpdate(Scene* scene, usize entity);

void SPlayerInputUpdate(Scene* scene, usize entity);
void SPlayerPostCollisionUpdate(Scene* scene, usize entity);
void SPlayerMortalUpdate(Scene* scene, usize entity);

void SFleetingUpdate(Scene* scene, usize entity);

void SCloudParticleDraw(const Scene* scene, usize entity);
void SSpriteDraw(const Scene* scene, usize entity);
void SDebugDraw(const Scene* scene, usize entity);
