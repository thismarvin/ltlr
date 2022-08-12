#include "atlas.h"
#include "string.h"
#include <cJSON.h>
#include <stdio.h>

void AtlasInit(Atlas* self, const char* path)
{
    char* buffer = LoadFileText(path);

    cJSON* parentObj = cJSON_Parse(buffer);

    // Get Metadata.
    {
        const cJSON* metadataObj = cJSON_GetObjectItem(parentObj, "METADATA");

        const cJSON* nameObj = cJSON_GetObjectItem(metadataObj, "name");
        const cJSON* sizeObj = cJSON_GetObjectItem(metadataObj, "size");

        const char* name = cJSON_GetStringValue(nameObj);
        strncpy(self->metadata.name, name, 5);
        self->metadata.size = (u16)cJSON_GetNumberValue(sizeObj);
    }

    // Initialize sprites.
    {
        const cJSON* spritesArray = cJSON_GetObjectItem(parentObj, "sprites");
        usize spritesLength = cJSON_GetArraySize(spritesArray);

        self->sprites = malloc(sizeof(AtlasSprite) * spritesLength);
        self->spritesLength = spritesLength;

        for (usize i = 0; i < spritesLength; ++i)
        {
            const cJSON* spriteObj = cJSON_GetArrayItem(spritesArray, i);

            const cJSON* nameObj = cJSON_GetObjectItem(spriteObj, "name");
            const cJSON* xObj = cJSON_GetObjectItem(spriteObj, "x");
            const cJSON* yObj = cJSON_GetObjectItem(spriteObj, "y");
            const cJSON* widthObj = cJSON_GetObjectItem(spriteObj, "width");
            const cJSON* heightObj = cJSON_GetObjectItem(spriteObj, "height");
            const cJSON* trimRectObj = cJSON_GetObjectItem(spriteObj, "trimRect");
            const cJSON* trimRectWidthObj = cJSON_GetObjectItem(trimRectObj, "width");
            const cJSON* trimRectHeightObj = cJSON_GetObjectItem(trimRectObj, "height");

            char* name = cJSON_GetStringValue(nameObj);
            const u16 x = (u16)cJSON_GetNumberValue(xObj);
            const u16 y = (u16)cJSON_GetNumberValue(yObj);
            const u16 width = (u16)cJSON_GetNumberValue(widthObj);
            const u16 height = (u16)cJSON_GetNumberValue(heightObj);
            const u16 trimRectWidth = (u16)cJSON_GetNumberValue(trimRectWidthObj);
            const u16 trimRectHeight = (u16)cJSON_GetNumberValue(trimRectHeightObj);

            self->sprites[i] = (AtlasSprite)
            {
                .x = x,
                .y = y,
                .width = width,
                .height = height,
                .trimRect = (AtlasTrimRect)
                {
                    .width = trimRectWidth,
                    .height = trimRectHeight,
                }
            };
            strncpy(self->sprites[i].name, name, MAX_SPRITE_NAME_LENGTH);
        }
    }

    cJSON_Delete(parentObj);
    free(buffer);
}

AtlasSprite* AtlasGet(const Atlas* self, const char* name)
{
    // TODO(thismarvin): Look into code generation to get around not having a HashMap.

    for (usize i = 0; i < self->spritesLength; ++i)
    {
        const AtlasSprite* sprite = &self->sprites[i];

        if (strcmp(sprite->name, name) != 0)
        {
            continue;
        }

        return (AtlasSprite*)sprite;
    }

    printf("The given name is not in the given atlas.\n");
    exit(EXIT_FAILURE);
}

void AtlasDestroy(Atlas* self)
{
    free(self->sprites);
}
