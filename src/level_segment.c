#include "./vendor/cJSON.h"
#include "level_segment.h"

void LevelSegmentInit(LevelSegment* self, char* path)
{
    char* buffer = LoadFileText(path);

    cJSON* parentObj = cJSON_Parse(buffer);

    // Get self->boundaries.
    {
        const cJSON* widthObj = cJSON_GetObjectItem(parentObj, "width");
        const cJSON* heightObj = cJSON_GetObjectItem(parentObj, "height");
        const cJSON* tilewidthObj = cJSON_GetObjectItem(parentObj, "tilewidth");
        const cJSON* tileheightObj = cJSON_GetObjectItem(parentObj, "tileheight");

        self->tilemapWidth = (u32)cJSON_GetNumberValue(widthObj);
        self->tilemapHeight = (u32)cJSON_GetNumberValue(heightObj);
        self->tileWidth = (u16)cJSON_GetNumberValue(tilewidthObj);
        self->tileHeight = (u16)cJSON_GetNumberValue(tileheightObj);

        self->bounds = (Rectangle)
        {
            .x = 0,
            .y = 0,
            .width = self->tilemapWidth * self->tileWidth,
            .height = self->tilemapHeight * self->tileHeight,
        };
    }

    // Initialize tileset.
    {
        const cJSON* tilesetsArray = cJSON_GetObjectItem(parentObj, "tilesets");

        const cJSON* tilesetObj = cJSON_GetArrayItem(tilesetsArray, 0);
        const cJSON* columnsObj = cJSON_GetObjectItem(tilesetObj, "columns");

        u16 columns = (u16)cJSON_GetNumberValue(columnsObj);

        self->tilesetColumns = columns;
    }

    const cJSON* layersArray = cJSON_GetObjectItem(parentObj, "layers");

    // Initialize sprites.
    {
        const cJSON* spritesObj = cJSON_GetArrayItem(layersArray, 0);
        const cJSON* dataObj = cJSON_GetObjectItem(spritesObj, "data");
        usize dataLength = cJSON_GetArraySize(dataObj);

        self->sprites = malloc(sizeof(u16) * dataLength);
        self->spritesLength = dataLength;

        for (usize i = 0; i < dataLength; ++i)
        {
            const cJSON* spriteObj = cJSON_GetArrayItem(dataObj, i);

            u16 sprite = (u16)cJSON_GetNumberValue(spriteObj);

            self->sprites[i] = sprite;
        }
    }

    // Initialize colliders.
    {
        const cJSON* collidersObj = cJSON_GetArrayItem(layersArray, 1);
        const cJSON* objectsObj = cJSON_GetObjectItem(collidersObj, "objects");
        usize objectsLength = cJSON_GetArraySize(objectsObj);

        self->colliders = malloc(sizeof(Rectangle) * objectsLength);
        self->collidersLength = objectsLength;

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

            // TODO(thismarvin): Support Polygons somehow...

            self->colliders[i] = (Rectangle)
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
}

void LevelSegmentDestroy(LevelSegment* self)
{
    free(self->sprites);
    free(self->colliders);
}
