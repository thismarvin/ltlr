#pragma once

#include "common.h"

typedef struct
{
    u16* sprites;
    usize spritesLength;
    Rectangle* colliders;
    usize collidersLength;
    u32 tilemapWidth;
    u32 tilemapHeight;
    Rectangle bounds;
    u16 tilesetColumns;
    u16 tileWidth;
    u16 tileHeight;
} LevelSegment;

LevelSegment LevelSegmentCreate(char* path);
void LevelSegmentDestroy(LevelSegment* self);
