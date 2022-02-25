#include "./vendor/cJSON.h"
#include "level_segment.h"

LevelSegment LevelSegmentCreate(char* path)
{
    LevelSegment segment;

    char* buffer = LoadFileText(path);

    cJSON* parentObj = cJSON_Parse(buffer);

    // Get segment boundaries.
    {
        const cJSON* widthObj = cJSON_GetObjectItem(parentObj, "width");
        const cJSON* heightObj = cJSON_GetObjectItem(parentObj, "height");
        const cJSON* tilewidthObj = cJSON_GetObjectItem(parentObj, "tilewidth");
        const cJSON* tileheightObj = cJSON_GetObjectItem(parentObj, "tileheight");

        segment.tilemapWidth = (u32)cJSON_GetNumberValue(widthObj);
        segment.tilemapHeight = (u32)cJSON_GetNumberValue(heightObj);
        segment.tileWidth = (u16)cJSON_GetNumberValue(tilewidthObj);
        segment.tileHeight = (u16)cJSON_GetNumberValue(tileheightObj);

        segment.bounds = (Rectangle)
        {
            .x = 0,
            .y = 0,
            .width = segment.tilemapWidth * segment.tileWidth,
            .height = segment.tilemapHeight * segment.tileHeight,
        };
    }

    // Initialize tileset.
    {
        const cJSON* tilesetsArray = cJSON_GetObjectItem(parentObj, "tilesets");

        const cJSON* tilesetObj = cJSON_GetArrayItem(tilesetsArray, 0);
        const cJSON* columnsObj = cJSON_GetObjectItem(tilesetObj, "columns");

        u16 columns = (u16)cJSON_GetNumberValue(columnsObj);

        segment.tilesetColumns = columns;
    }

    const cJSON* layersArray = cJSON_GetObjectItem(parentObj, "layers");

    // Initialize sprites.
    {
        const cJSON* spritesObj = cJSON_GetArrayItem(layersArray, 0);
        const cJSON* dataObj = cJSON_GetObjectItem(spritesObj, "data");
        usize dataLength = cJSON_GetArraySize(dataObj);

        segment.sprites = malloc(sizeof(u16) * dataLength);
        segment.spritesLength = dataLength;

        for (usize i = 0; i < dataLength; ++i)
        {
            const cJSON* spriteObj = cJSON_GetArrayItem(dataObj, i);

            u16 sprite = (u16)cJSON_GetNumberValue(spriteObj);

            segment.sprites[i] = sprite;
        }
    }

    // Initialize colliders.
    {
        const cJSON* collidersObj = cJSON_GetArrayItem(layersArray, 1);
        const cJSON* objectsObj = cJSON_GetObjectItem(collidersObj, "objects");
        usize objectsLength = cJSON_GetArraySize(objectsObj);

        segment.colliders = malloc(sizeof(Rectangle) * objectsLength);
        segment.collidersLength = objectsLength;

        for (usize i = 0; i < objectsLength; ++i)
        {
            const cJSON* colliderObj = cJSON_GetArrayItem(objectsObj, i);

            const cJSON* xObj = cJSON_GetObjectItem(colliderObj, "x");
            const cJSON* yObj = cJSON_GetObjectItem(colliderObj, "y");
            const cJSON* widthObj = cJSON_GetObjectItem(colliderObj, "width");
            const cJSON* heightObj = cJSON_GetObjectItem(colliderObj, "height");

            float x = (float)cJSON_GetNumberValue(xObj);
            float y = (float)cJSON_GetNumberValue(yObj);
            float width = (float)cJSON_GetNumberValue(widthObj);
            float height = (float)cJSON_GetNumberValue(heightObj);

            // TODO(thismarvin): Support or ignore Polygons somehow...
            // if (width == 0 || height == 0)
            // {
            //     continue;
            // }

            segment.colliders[i] = (Rectangle)
            {
                .x = x,
                .y = y,
                .width = width,
                .height = height,
            };
        }
    }

    cJSON_Delete(parentObj);
    free(buffer);

    return segment;
}

void LevelSegmentDestroy(LevelSegment* self)
{
    free(self->sprites);
    free(self->colliders);
}
