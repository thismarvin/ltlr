#pragma once

#include "common.h"
#include "sprites_generated.h"

typedef struct
{
	u16 width;
	u16 height;
} AtlasEntryDimension;

typedef struct
{
	AtlasEntryDimension untrimmed;
	Rectangle source;
	Rectangle destination;
} AtlasEntry;

typedef struct
{
	Texture2D texture;
	AtlasEntry* entries;
	usize entriesLength;
} Atlas;

typedef struct
{
	Sprite sprite;
	Vector2 position;
	Vector2 scale;
	Rectangle intramural;
	Reflection reflection;
	Color tint;
} AtlasDrawParams;

Atlas AtlasCreate(const char* path);
void AtlasDraw(const Atlas* self, const AtlasDrawParams* params);
void AtlasDestroy(Atlas* self);
