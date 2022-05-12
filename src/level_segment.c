#include "level_segment.h"
#include <cJSON.h>

void LevelSegmentInit(LevelSegment* self, const char* path)
{
    char* buffer = LoadFileText(path);

    cJSON* parentObj = cJSON_Parse(buffer);

    // Get level boundaries.
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
        const cJSON* dataArray = cJSON_GetObjectItem(spritesObj, "data");
        usize dataLength = cJSON_GetArraySize(dataArray);

        self->sprites = malloc(sizeof(u16) * dataLength);
        self->spritesLength = dataLength;

        for (usize i = 0; i < dataLength; ++i)
        {
            const cJSON* spriteObj = cJSON_GetArrayItem(dataArray, i);

            u16 sprite = (u16)cJSON_GetNumberValue(spriteObj);

            self->sprites[i] = sprite;
        }
    }

    // Initialize colliders.
    {
        const cJSON* collidersObj = cJSON_GetArrayItem(layersArray, 1);
        const cJSON* objectsArray = cJSON_GetObjectItem(collidersObj, "objects");
        usize objectsLength = cJSON_GetArraySize(objectsArray);

        self->colliders = malloc(sizeof(LevelCollider) * objectsLength);
        self->collidersLength = objectsLength;

        for (usize i = 0; i < objectsLength; ++i)
        {
            const cJSON* colliderObj = cJSON_GetArrayItem(objectsArray, i);

            const cJSON* xObj = cJSON_GetObjectItem(colliderObj, "x");
            const cJSON* yObj = cJSON_GetObjectItem(colliderObj, "y");
            const cJSON* widthObj = cJSON_GetObjectItem(colliderObj, "width");
            const cJSON* heightObj = cJSON_GetObjectItem(colliderObj, "height");

            f32 x = (f32)cJSON_GetNumberValue(xObj);
            f32 y = (f32)cJSON_GetNumberValue(yObj);
            f32 width = (f32)cJSON_GetNumberValue(widthObj);
            f32 height = (f32)cJSON_GetNumberValue(heightObj);

            const cJSON* propertiesArray = cJSON_GetObjectItem(colliderObj, "properties");

            const cJSON* layerPropObj = cJSON_GetArrayItem(propertiesArray, 0);
            const cJSON* resolutionschemaPropObj = cJSON_GetArrayItem(propertiesArray, 1);

            const cJSON* layerObj = cJSON_GetObjectItem(layerPropObj, "value");
            const cJSON* resolutionschemaObj = cJSON_GetObjectItem(resolutionschemaPropObj, "value");

            u64 layer = (u64)cJSON_GetNumberValue(layerObj);
            u8 resolutionSchema = (u8)cJSON_GetNumberValue(resolutionschemaObj);

            // TODO(thismarvin): Support Polygons somehow...

            Rectangle aabb = (Rectangle)
            {
                .x = x,
                .y = y,
                .width = width,
                .height = height,
            };

            self->colliders[i] = (LevelCollider)
            {
                .aabb = aabb,
                .resolutionSchema = resolutionSchema,
                .layer = layer,
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
