#pragma once

#include "../../atlas.h"
#include "../../scene.h"
#include "../entity_builder.h"

EntityBuilder BatteryCreate(f32 x, f32 y);
void BatteryUpdate(Scene* scene, usize entity);
