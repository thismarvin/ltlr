// This file was auto-generated; any changes you make may be overwritten.

#include "level.h"
#include "segments_generated.h"
#include <stdio.h>
#include <stdlib.h>

LevelSegmentBuilder LevelSegmentBuilderCreate(const u16 type, const Vector2 offset)
{
    switch (type)
    {
        case 0:
        {
            return LevelSegmentBuilderCreate_segment_0(offset);
        }

        case 1:
        {
            return LevelSegmentBuilderCreate_segment_1(offset);
        }

        case 2:
        {
            return LevelSegmentBuilderCreate_segment_2(offset);
        }
    }

    fprintf(stderr, "The given type is not a valid LevelSegmentType");
    exit(EXIT_FAILURE);
}

void LevelSegmentDraw(const LevelSegment* self, const Atlas* atlas, const Vector2 offset)
{
    switch (self->type)
    {
        case 0:
        {
            LevelSegmentDraw_segment_0(atlas, offset);
            break;
        }

        case 1:
        {
            LevelSegmentDraw_segment_1(atlas, offset);
            break;
        }

        case 2:
        {
            LevelSegmentDraw_segment_2(atlas, offset);
            break;
        }
    }
}
