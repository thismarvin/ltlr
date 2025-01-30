#pragma once

#include "../../common.h"
#include "../../level.h"

#define FOG_LETHAL_DISTANCE (CTX_VIEWPORT_WIDTH * 0.75)

typedef struct
{
	usize entity;
} FogBuilder;

void FogBuild(Scene* scene, const void* params);

void FogUpdate(Scene* scene, usize entity);

void FogDraw(const Scene* scene, usize entity);
void FogDebugDraw(const Scene* scene, usize entity);
