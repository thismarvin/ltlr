#pragma once

#include "scene.h"

void SSmoothUpdate(Scene* scene, usize entity);
void SKineticUpdate(Scene* scene, usize entity);
void SCollisionUpdate(Scene* scene, usize entity);
// TODO(thismarvin): Do we still neeed this system?
void SVulenerableUpdate(Scene* scene, usize entity);

void SPlayerInputUpdate(Scene* scene, usize entity);
void SPlayerCollisionUpdate(Scene* scene, usize entity);

void SWalkerCollisionUpdate(Scene* scene, usize entity);
void SWalkerUpdate(Scene* scene, usize entity);

void SSpriteDraw(Scene* scene, Texture2D* atlas, usize entity);
void SDebugDraw(Scene* scene, usize entity);
