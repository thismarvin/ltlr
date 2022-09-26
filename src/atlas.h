#pragma once

#include "common.h"

#define MAX_SPRITE_NAME_LENGTH 16

typedef struct
{
    u16 width;
    u16 height;
} AtlasSpriteDimension;

typedef struct
{
    u16 x;
    u16 y;
    u16 width;
    u16 height;
} AtlasSpriteImageRegion;

typedef struct
{
    char name[MAX_SPRITE_NAME_LENGTH];
    AtlasSpriteDimension untrimmed;
    AtlasSpriteImageRegion source;
    AtlasSpriteImageRegion destination;
} AtlasSprite;

typedef struct
{
    AtlasSprite* sprites;
    usize spritesLength;
} Atlas;

void AtlasInit(Atlas* self, const char* path);
AtlasSprite* AtlasGet(const Atlas* self, const char* name);
// TODO(thismarvin): void AtlasDrawSprite(const Atlas* self, AtlasSprite* sprite);
void AtlasDestroy(Atlas* self);
