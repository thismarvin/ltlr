#pragma once

#include "../scene.h"

void SSmoothUpdate(Scene* scene, usize entity);
void SKineticUpdate(Scene* scene, usize entity);
void SCollisionUpdate(Scene* scene, usize entity);
void SFleetingUpdate(Scene* scene, usize entity);
void SAnimationUpdate(Scene* scene, usize entity);

void SSpriteDraw(const Scene* scene, usize entity);
void SDebugDraw(const Scene* scene, usize entity);
