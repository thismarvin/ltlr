#pragma once

#include "../../scene.h"
#include "../entity_builder.h"

EntityBuilder FogCreate(void);

void FogUpdate(Scene* scene, usize entity);
void FogDraw(const Scene* scene, usize entity);
