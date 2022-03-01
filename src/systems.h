#pragma once

#include "scene.h"

void SSmoothUpdate(Components*, usize entity);
void SPlayerUpdate(Scene* scene, usize entity);
void SKineticUpdate(Components*, usize entity);
void SCollisionUpdate(Components* components, usize entityCount, usize entity);
void SVulenerableUpdate(Scene* components, usize entity);
void SWalkerUpdate(Components* components, usize entity);

void SSpriteDraw(Components* components, Texture2D* atlas, usize entity);
void SDebugDraw(Components* components, usize entity);
