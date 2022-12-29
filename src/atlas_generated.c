// This file was auto-generated; any changes you make may be overwritten.

#include "atlas.h"

Atlas AtlasCreate(const char* path)
{
    static const usize length = 180;

    Atlas atlas = (Atlas)
    {
        .texture = LoadTexture(path),
        .entries = malloc(sizeof(AtlasEntry) * length),
        .entriesLength = length,
    };

    atlas.entries[SPRITE_BATTERY] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 32 },
        .source = (Rectangle) { 1, 0, 14, 32 },
        .destination = (Rectangle) { 171, 120, 14, 32 },
    };

    atlas.entries[SPRITE_BLOCK_0_0000] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 16, 16, -15, -15 },
        .destination = (Rectangle) { 24, 180, -15, -15 },
    };

    atlas.entries[SPRITE_BLOCK_0_0001] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 16, 16, -15, -15 },
        .destination = (Rectangle) { 13, 180, -15, -15 },
    };

    atlas.entries[SPRITE_BLOCK_0_0002] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 16, 16, -15, -15 },
        .destination = (Rectangle) { 2, 180, -15, -15 },
    };

    atlas.entries[SPRITE_BLOCK_0_0003] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 221, 120, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_0_0004] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 16, 16, -15, -15 },
        .destination = (Rectangle) { -9, 180, -15, -15 },
    };

    atlas.entries[SPRITE_BLOCK_0_0005] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 16, 16, -15, -15 },
        .destination = (Rectangle) { -20, 180, -15, -15 },
    };

    atlas.entries[SPRITE_BLOCK_0_0006] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 241, 120, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_0_0007] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 261, 120, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_0_0008] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 16, 16, -15, -15 },
        .destination = (Rectangle) { -31, 180, -15, -15 },
    };

    atlas.entries[SPRITE_BLOCK_0_0009] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 281, 120, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_0_0010] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 16, 16, -15, -15 },
        .destination = (Rectangle) { -42, 180, -15, -15 },
    };

    atlas.entries[SPRITE_BLOCK_0_0011] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 301, 120, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_0_0012] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 321, 120, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_0_0013] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 341, 120, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_0_0014] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 361, 120, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_0_0015] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 381, 120, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_1_0000] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 401, 120, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_1_0001] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 421, 120, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_1_0002] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 441, 120, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_1_0003] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 461, 120, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_1_0004] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 481, 120, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_1_0005] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 4, 160, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_1_0006] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 24, 160, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_1_0007] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 44, 160, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_1_0008] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 64, 160, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_1_0009] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 84, 160, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_1_0010] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 104, 160, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_1_0011] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 124, 160, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_1_0012] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 144, 160, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_1_0013] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 164, 160, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_1_0014] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 184, 160, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_1_0015] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 204, 160, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_2_0000] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 224, 160, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_2_0001] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 244, 160, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_2_0002] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 264, 160, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_2_0003] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 284, 160, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_2_0004] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 304, 160, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_2_0005] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 324, 160, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_2_0006] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 344, 160, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_2_0007] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 364, 160, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_2_0008] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 384, 160, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_2_0009] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 404, 160, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_2_0010] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 424, 160, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_2_0011] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 444, 160, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_2_0012] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 464, 160, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_2_0013] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 484, 160, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_2_0014] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 4, 180, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_2_0015] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 24, 180, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_3_0000] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 44, 180, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_3_0001] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 64, 180, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_3_0002] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 84, 180, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_3_0003] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 104, 180, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_3_0004] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 124, 180, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_3_0005] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 144, 180, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_3_0006] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 164, 180, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_3_0007] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 184, 180, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_3_0008] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 204, 180, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_3_0009] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 224, 180, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_3_0010] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 244, 180, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_3_0011] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 264, 180, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_3_0012] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 284, 180, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_3_0013] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 304, 180, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_3_0014] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 324, 180, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_3_0015] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 344, 180, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_4_0000] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 364, 180, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_4_0001] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 384, 180, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_4_0002] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 404, 180, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_4_0003] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 424, 180, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_4_0004] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 444, 180, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_4_0005] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 464, 180, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_4_0006] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 484, 180, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_4_0007] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 4, 200, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_4_0008] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 24, 200, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_4_0009] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 44, 200, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_4_0010] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 64, 200, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_4_0011] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 84, 200, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_4_0012] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 104, 200, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_4_0013] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 124, 200, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_4_0014] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 144, 200, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_4_0015] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 164, 200, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_5_0000] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 184, 200, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_5_0001] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 204, 200, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_5_0002] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 224, 200, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_5_0003] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 244, 200, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_5_0004] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 264, 200, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_5_0005] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 284, 200, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_5_0006] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 304, 200, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_5_0007] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 324, 200, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_5_0008] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 344, 200, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_5_0009] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 364, 200, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_5_0010] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 384, 200, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_5_0011] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 404, 200, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_5_0012] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 424, 200, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_5_0013] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 444, 200, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_5_0014] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 464, 200, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_5_0015] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 484, 200, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_6_0000] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 16, 16, -15, -15 },
        .destination = (Rectangle) { -53, 180, -15, -15 },
    };

    atlas.entries[SPRITE_BLOCK_6_0001] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 4, 220, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_6_0002] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 24, 220, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_6_0003] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 44, 220, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_6_0004] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 64, 220, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_6_0005] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 84, 220, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_6_0006] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 104, 220, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_6_0007] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 124, 220, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_6_0008] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 144, 220, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_6_0009] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 164, 220, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_6_0010] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 184, 220, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_6_0011] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 204, 220, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_6_0012] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 224, 220, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_6_0013] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 244, 220, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_6_0014] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 264, 220, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_6_0015] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 284, 220, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_7_0000] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 16, 16, -15, -15 },
        .destination = (Rectangle) { -64, 180, -15, -15 },
    };

    atlas.entries[SPRITE_BLOCK_7_0001] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 304, 220, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_7_0002] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 324, 220, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_7_0003] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 344, 220, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_7_0004] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 364, 220, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_7_0005] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 384, 220, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_7_0006] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 404, 220, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_7_0007] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 424, 220, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_7_0008] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 444, 220, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_7_0009] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 464, 220, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_7_0010] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 484, 220, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_7_0011] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 4, 240, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_7_0012] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 24, 240, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_7_0013] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 44, 240, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_7_0014] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 64, 240, 16, 16 },
    };

    atlas.entries[SPRITE_BLOCK_7_0015] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 16 },
        .destination = (Rectangle) { 84, 240, 16, 16 },
    };

    atlas.entries[SPRITE_GAME_OVER] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 67, 48 },
        .source = (Rectangle) { 0, 0, 67, 48 },
        .destination = (Rectangle) { 146, 4, 67, 48 },
    };

    atlas.entries[SPRITE_HEART_0000] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 32 },
        .source = (Rectangle) { 0, 0, 16, 17 },
        .destination = (Rectangle) { 372, 97, 16, 17 },
    };

    atlas.entries[SPRITE_HEART_0001] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 32 },
        .source = (Rectangle) { 0, 0, 16, 17 },
        .destination = (Rectangle) { 392, 97, 16, 17 },
    };

    atlas.entries[SPRITE_HEART_0002] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 32 },
        .source = (Rectangle) { 0, 2, 2, 7 },
        .destination = (Rectangle) { 504, 160, 2, 7 },
    };

    atlas.entries[SPRITE_HEART_0003] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 32 },
        .source = (Rectangle) { 3, 2, 13, 11 },
        .destination = (Rectangle) { 189, 140, 13, 11 },
    };

    atlas.entries[SPRITE_LOGO] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 138, 112 },
        .source = (Rectangle) { 0, 0, 138, 112 },
        .destination = (Rectangle) { 4, 4, 138, 112 },
    };

    atlas.entries[SPRITE_NUMBERS_0000] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 19, 19 },
        .source = (Rectangle) { 0, 0, 19, 19 },
        .destination = (Rectangle) { 146, 97, 19, 19 },
    };

    atlas.entries[SPRITE_NUMBERS_0001] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 19, 19 },
        .source = (Rectangle) { 2, 0, 15, 19 },
        .destination = (Rectangle) { 353, 97, 15, 19 },
    };

    atlas.entries[SPRITE_NUMBERS_0002] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 19, 19 },
        .source = (Rectangle) { 0, 0, 19, 19 },
        .destination = (Rectangle) { 169, 97, 19, 19 },
    };

    atlas.entries[SPRITE_NUMBERS_0003] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 19, 19 },
        .source = (Rectangle) { 0, 0, 19, 19 },
        .destination = (Rectangle) { 192, 97, 19, 19 },
    };

    atlas.entries[SPRITE_NUMBERS_0004] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 19, 19 },
        .source = (Rectangle) { 0, 0, 19, 19 },
        .destination = (Rectangle) { 215, 97, 19, 19 },
    };

    atlas.entries[SPRITE_NUMBERS_0005] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 19, 19 },
        .source = (Rectangle) { 0, 0, 19, 19 },
        .destination = (Rectangle) { 238, 97, 19, 19 },
    };

    atlas.entries[SPRITE_NUMBERS_0006] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 19, 19 },
        .source = (Rectangle) { 0, 0, 19, 19 },
        .destination = (Rectangle) { 261, 97, 19, 19 },
    };

    atlas.entries[SPRITE_NUMBERS_0007] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 19, 19 },
        .source = (Rectangle) { 0, 0, 19, 19 },
        .destination = (Rectangle) { 284, 97, 19, 19 },
    };

    atlas.entries[SPRITE_NUMBERS_0008] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 19, 19 },
        .source = (Rectangle) { 0, 0, 19, 19 },
        .destination = (Rectangle) { 307, 97, 19, 19 },
    };

    atlas.entries[SPRITE_NUMBERS_0009] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 19, 19 },
        .source = (Rectangle) { 0, 0, 19, 19 },
        .destination = (Rectangle) { 330, 97, 19, 19 },
    };

    atlas.entries[SPRITE_ONE_WAY_BASE] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 48, 32 },
        .source = (Rectangle) { 0, 0, 48, 32 },
        .destination = (Rectangle) { 119, 120, 48, 32 },
    };

    atlas.entries[SPRITE_ONE_WAY_PILLAR] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 2, 0, 12, 16 },
        .destination = (Rectangle) { 104, 240, 12, 16 },
    };

    atlas.entries[SPRITE_ONE_WAY_TOP] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 8 },
        .destination = (Rectangle) { 206, 140, 16, 8 },
    };

    atlas.entries[SPRITE_PLAYER_JUMP_0000] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 64, 80 },
        .source = (Rectangle) { 19, 28, 25, 36 },
        .destination = (Rectangle) { 290, 56, 25, 36 },
    };

    atlas.entries[SPRITE_PLAYER_JUMP_0001] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 64, 80 },
        .source = (Rectangle) { 19, 25, 25, 39 },
        .destination = (Rectangle) { 401, 4, 25, 39 },
    };

    atlas.entries[SPRITE_PLAYER_JUMP_0002] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 64, 80 },
        .source = (Rectangle) { 19, 25, 25, 39 },
        .destination = (Rectangle) { 430, 4, 25, 39 },
    };

    atlas.entries[SPRITE_PLAYER_JUMP_0003] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 64, 80 },
        .source = (Rectangle) { 19, 26, 25, 38 },
        .destination = (Rectangle) { 459, 4, 25, 38 },
    };

    atlas.entries[SPRITE_PLAYER_JUMP_0004] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 64, 80 },
        .source = (Rectangle) { 19, 27, 25, 37 },
        .destination = (Rectangle) { 146, 56, 25, 37 },
    };

    atlas.entries[SPRITE_PLAYER_RUN_0000] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 64, 80 },
        .source = (Rectangle) { 19, 28, 25, 36 },
        .destination = (Rectangle) { 319, 56, 25, 36 },
    };

    atlas.entries[SPRITE_PLAYER_RUN_0001] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 64, 80 },
        .source = (Rectangle) { 20, 26, 23, 37 },
        .destination = (Rectangle) { 175, 56, 23, 37 },
    };

    atlas.entries[SPRITE_PLAYER_RUN_0002] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 64, 80 },
        .source = (Rectangle) { 22, 28, 20, 36 },
        .destination = (Rectangle) { 488, 4, 20, 36 },
    };

    atlas.entries[SPRITE_PLAYER_RUN_0003] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 64, 80 },
        .source = (Rectangle) { 20, 26, 23, 37 },
        .destination = (Rectangle) { 202, 56, 23, 37 },
    };

    atlas.entries[SPRITE_PLAYER_SPIN_0000] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 64, 80 },
        .source = (Rectangle) { 19, 28, 24, 36 },
        .destination = (Rectangle) { 377, 56, 24, 36 },
    };

    atlas.entries[SPRITE_PLAYER_SPIN_0001] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 64, 80 },
        .source = (Rectangle) { 20, 28, 22, 36 },
        .destination = (Rectangle) { 432, 56, 22, 36 },
    };

    atlas.entries[SPRITE_PLAYER_SPIN_0002] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 64, 80 },
        .source = (Rectangle) { 22, 28, 19, 36 },
        .destination = (Rectangle) { 483, 56, 19, 36 },
    };

    atlas.entries[SPRITE_PLAYER_SPIN_0003] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 64, 80 },
        .source = (Rectangle) { 22, 28, 19, 36 },
        .destination = (Rectangle) { 4, 120, 19, 36 },
    };

    atlas.entries[SPRITE_PLAYER_SPIN_0004] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 64, 80 },
        .source = (Rectangle) { 22, 28, 19, 36 },
        .destination = (Rectangle) { 27, 120, 19, 36 },
    };

    atlas.entries[SPRITE_PLAYER_SPIN_0005] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 64, 80 },
        .source = (Rectangle) { 22, 28, 19, 36 },
        .destination = (Rectangle) { 50, 120, 19, 36 },
    };

    atlas.entries[SPRITE_PLAYER_SPIN_0006] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 64, 80 },
        .source = (Rectangle) { 22, 28, 19, 36 },
        .destination = (Rectangle) { 73, 120, 19, 36 },
    };

    atlas.entries[SPRITE_PLAYER_SPIN_0007] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 64, 80 },
        .source = (Rectangle) { 19, 28, 25, 36 },
        .destination = (Rectangle) { 348, 56, 25, 36 },
    };

    atlas.entries[SPRITE_PLAYER_SPIN_0008] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 64, 80 },
        .source = (Rectangle) { 19, 28, 26, 36 },
        .destination = (Rectangle) { 260, 56, 26, 36 },
    };

    atlas.entries[SPRITE_PLAYER_SPIN_0009] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 64, 80 },
        .source = (Rectangle) { 22, 28, 21, 36 },
        .destination = (Rectangle) { 458, 56, 21, 36 },
    };

    atlas.entries[SPRITE_PLAYER_SPIN_0010] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 64, 80 },
        .source = (Rectangle) { 22, 28, 19, 36 },
        .destination = (Rectangle) { 96, 120, 19, 36 },
    };

    atlas.entries[SPRITE_PLAYER_SPIN_0011] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 64, 80 },
        .source = (Rectangle) { 20, 28, 23, 36 },
        .destination = (Rectangle) { 405, 56, 23, 36 },
    };

    atlas.entries[SPRITE_PLAYER_SPIN_0012] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 64, 80 },
        .source = (Rectangle) { 18, 28, 27, 36 },
        .destination = (Rectangle) { 229, 56, 27, 36 },
    };

    atlas.entries[SPRITE_SOLAR_0000] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 96, 48 },
        .source = (Rectangle) { 4, 8, 88, 40 },
        .destination = (Rectangle) { 217, 4, 88, 40 },
    };

    atlas.entries[SPRITE_SOLAR_0001] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 96, 48 },
        .source = (Rectangle) { 4, 8, 88, 40 },
        .destination = (Rectangle) { 309, 4, 88, 40 },
    };

    atlas.entries[SPRITE_SPIKE_0000] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 11, 16, 5 },
        .destination = (Rectangle) { 226, 140, 16, 5 },
    };

    atlas.entries[SPRITE_SPIKE_0001] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 5, 16 },
        .destination = (Rectangle) { 501, 120, 5, 16 },
    };

    atlas.entries[SPRITE_SPIKE_0002] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 0, 0, 16, 5 },
        .destination = (Rectangle) { 246, 140, 16, 5 },
    };

    atlas.entries[SPRITE_SPIKE_0003] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 16, 16 },
        .source = (Rectangle) { 11, 0, 5, 16 },
        .destination = (Rectangle) { 120, 240, 5, 16 },
    };

    atlas.entries[SPRITE_WALKER_IDLE_0000] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 48, 16 },
        .source = (Rectangle) { 10, 0, 28, 16 },
        .destination = (Rectangle) { 412, 97, 28, 16 },
    };

    atlas.entries[SPRITE_WALKER_IDLE_0001] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 48, 16 },
        .source = (Rectangle) { 10, 0, 28, 16 },
        .destination = (Rectangle) { 444, 97, 28, 16 },
    };

    atlas.entries[SPRITE_WALKER_IDLE_0002] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 48, 16 },
        .source = (Rectangle) { 10, 0, 28, 16 },
        .destination = (Rectangle) { 476, 97, 28, 16 },
    };

    atlas.entries[SPRITE_WALKER_IDLE_0003] = (AtlasEntry)
    {
        .untrimmed = (AtlasEntryDimension) { 48, 16 },
        .source = (Rectangle) { 10, 0, 28, 16 },
        .destination = (Rectangle) { 189, 120, 28, 16 },
    };

    return atlas;
}
