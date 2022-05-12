#pragma once

#include "common.h"

typedef struct
{
    Rectangle aabb;
    u8 resolutionSchema;
    u64 layer;
} LevelCollider;

typedef struct
{
    u16* sprites;
    usize spritesLength;
    LevelCollider* colliders;
    usize collidersLength;
    u32 tilemapWidth;
    u32 tilemapHeight;
    Rectangle bounds;
    u16 tilesetColumns;
    u16 tileWidth;
    u16 tileHeight;
} LevelSegment;

void LevelSegmentInit(LevelSegment* self, const char* path);
void LevelSegmentDestroy(LevelSegment* self);
