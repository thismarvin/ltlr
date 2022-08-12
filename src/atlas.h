#pragma once

#include "common.h"

#define MAX_SPRITE_NAME_LENGTH 16

typedef struct
{
    u16 width;
    u16 height;
} AtlasTrimRect;

typedef struct
{
    char name[MAX_SPRITE_NAME_LENGTH];
    u16 x;
    u16 y;
    u16 width;
    u16 height;
    AtlasTrimRect trimRect;
} AtlasSprite;

typedef struct
{
    char name[5];
    u16 size;
} AtlasMetadata;

typedef struct
{
    AtlasMetadata metadata;
    AtlasSprite* sprites;
    usize spritesLength;
} Atlas;

void AtlasInit(Atlas* self, const char* path);
AtlasSprite* AtlasGet(const Atlas* self, const char* name);
// TODO(thismarvin): void AtlasDrawSprite(const Atlas* self, AtlasSprite* sprite);
void AtlasDestroy(Atlas* self);
