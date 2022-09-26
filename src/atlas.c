#include "atlas.h"
#include "string.h"
#include <cJSON.h>
#include <stdio.h>

void AtlasInit(Atlas* self, const char* path)
{
    char* buffer = LoadFileText(path);

    cJSON* spritesArray = cJSON_Parse(buffer);
    usize spritesLength = cJSON_GetArraySize(spritesArray);

    self->sprites = malloc(sizeof(AtlasSprite) * spritesLength);
    self->spritesLength = spritesLength;

    for (usize i = 0; i < spritesLength; ++i)
    {
        const cJSON* spriteObj = cJSON_GetArrayItem(spritesArray, i);

        const cJSON* nameObj = cJSON_GetObjectItem(spriteObj, "name");
        const cJSON* untrimmedObj = cJSON_GetObjectItem(spriteObj, "untrimmed");
        const cJSON* sourceObj = cJSON_GetObjectItem(spriteObj, "source");
        const cJSON* destinationObj = cJSON_GetObjectItem(spriteObj, "destination");

        const cJSON* untrimmedWidthObj = cJSON_GetObjectItem(untrimmedObj, "width");
        const cJSON* untrimmedHeightObj = cJSON_GetObjectItem(untrimmedObj, "height");
        const cJSON* sourceXObj = cJSON_GetObjectItem(sourceObj, "x");
        const cJSON* sourceYObj = cJSON_GetObjectItem(sourceObj, "y");
        const cJSON* sourceWidthObj = cJSON_GetObjectItem(sourceObj, "width");
        const cJSON* sourceHeightObj = cJSON_GetObjectItem(sourceObj, "height");
        const cJSON* destinationXObj = cJSON_GetObjectItem(destinationObj, "x");
        const cJSON* destinationYObj = cJSON_GetObjectItem(destinationObj, "y");
        const cJSON* destinationWidthObj = cJSON_GetObjectItem(destinationObj, "width");
        const cJSON* destinationHeightObj = cJSON_GetObjectItem(destinationObj, "height");

        char* name = cJSON_GetStringValue(nameObj);
        const u16 untrimmedWidth = (u16)cJSON_GetNumberValue(untrimmedWidthObj);
        const u16 untrimmedHeight = (u16)cJSON_GetNumberValue(untrimmedHeightObj);
        const u16 sourceX = (u16)cJSON_GetNumberValue(sourceXObj);
        const u16 sourceY = (u16)cJSON_GetNumberValue(sourceYObj);
        const u16 sourceWidth = (u16)cJSON_GetNumberValue(sourceWidthObj);
        const u16 sourceHeight = (u16)cJSON_GetNumberValue(sourceHeightObj);
        const u16 destinationX = (u16)cJSON_GetNumberValue(destinationXObj);
        const u16 destinationY = (u16)cJSON_GetNumberValue(destinationYObj);
        const u16 destinationWidth = (u16)cJSON_GetNumberValue(destinationWidthObj);
        const u16 destinationHeight = (u16)cJSON_GetNumberValue(destinationHeightObj);

        self->sprites[i] = (AtlasSprite)
        {
            .untrimmed = (AtlasSpriteDimension)
            {
                .width = untrimmedWidth,
                .height = untrimmedHeight,
            },
            .source = (AtlasSpriteImageRegion)
            {
                .x = sourceX,
                .y = sourceY,
                .width = sourceWidth,
                .height = sourceHeight,
            },
            .destination = (AtlasSpriteImageRegion)
            {
                .x = destinationX,
                .y = destinationY,
                .width = destinationWidth,
                .height = destinationHeight,
            },
        };
        strncpy(self->sprites[i].name, name, MAX_SPRITE_NAME_LENGTH);
    }

    cJSON_Delete(spritesArray);
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

    fprintf(stderr, "The given name is not in the given atlas.\n");
    exit(EXIT_FAILURE);
}

void AtlasDestroy(Atlas* self)
{
    free(self->sprites);
}
