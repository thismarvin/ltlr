#pragma once

#include "../../scene.h"
#include "../entity_builder.h"

#define FOG_LETHAL_DISTANCE (CTX_VIEWPORT_WIDTH * 0.75)

EntityBuilder FogCreate(void);

void FogUpdate(Scene* scene, usize entity);
void FogDraw(const Scene* scene, usize entity);
