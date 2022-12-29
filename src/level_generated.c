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
            return LevelSegmentBuilderCreate_battery_0(offset);
        }

        case 1:
        {
            return LevelSegmentBuilderCreate_filler_0(offset);
        }

        case 2:
        {
            return LevelSegmentBuilderCreate_filler_1(offset);
        }

        case 3:
        {
            return LevelSegmentBuilderCreate_filler_2(offset);
        }

        case 4:
        {
            return LevelSegmentBuilderCreate_filler_3(offset);
        }

        case 5:
        {
            return LevelSegmentBuilderCreate_solar_0(offset);
        }

        case 6:
        {
            return LevelSegmentBuilderCreate_starter_0(offset);
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
            LevelSegmentDraw_battery_0(atlas, offset);
            break;
        }

        case 1:
        {
            LevelSegmentDraw_filler_0(atlas, offset);
            break;
        }

        case 2:
        {
            LevelSegmentDraw_filler_1(atlas, offset);
            break;
        }

        case 3:
        {
            LevelSegmentDraw_filler_2(atlas, offset);
            break;
        }

        case 4:
        {
            LevelSegmentDraw_filler_3(atlas, offset);
            break;
        }

        case 5:
        {
            LevelSegmentDraw_solar_0(atlas, offset);
            break;
        }

        case 6:
        {
            LevelSegmentDraw_starter_0(atlas, offset);
            break;
        }
    }
}
