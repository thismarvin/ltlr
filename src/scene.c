#include "./vendor/cJSON.h"
#include "entities.h"
#include "scene.h"
#include "systems.h"
#include <assert.h>
#include <string.h>

static void PushFree(Scene* self, usize value)
{
    assert(self->nextFreeSlot < MAX_ENTITIES);

    self->freeSlots[self->nextFreeSlot] = value;
    self->nextFreeSlot += 1;
}

static usize PopFree(Scene* self)
{
    assert(self->nextFreeSlot > 0);

    usize slot = self->freeSlots[self->nextFreeSlot - 1];
    self->nextFreeSlot -= 1;

    return slot;
}

usize SceneAllocateEntity(Scene* self)
{
    if (self->nextFreeSlot == 0)
    {
        usize next = MIN(self->nextEntity, MAX_ENTITIES - 1);

        self->nextEntity = self->nextEntity + 1;
        self->nextEntity = MIN(self->nextEntity, MAX_ENTITIES);

        return next;
    }
    else
    {
        return PopFree(self);
    }
}

void SceneDeallocateEntity(Scene* self, usize entity)
{
    self->components.tags[entity] = 0;
    PushFree(self, entity);
}

void SceneInit(Scene* self)
{
    memset(&self->components.tags, 0, sizeof(u64) * MAX_ENTITIES);

    // TODO(thismarvin): Do we need to initialize positions, dimensions, etc.?

    self->nextEntity = 0;

    self->nextFreeSlot = 0;
    memset(&self->freeSlots, 0, sizeof(u64));

    self->debugging = false;

    // Level Loading.
    {
#if defined(PLATFORM_WEB)
        char* buffer = LoadFileText("./src/resources/build/level.json");
#else
        char* buffer = LoadFileText("./resources/build/level.json");
#endif

        cJSON* level = cJSON_Parse(buffer);

        const cJSON* widthObj = cJSON_GetObjectItem(level, "width");
        const cJSON* heightObj = cJSON_GetObjectItem(level, "height");
        const cJSON* tilewidthObj = cJSON_GetObjectItem(level, "tilewidth");
        const cJSON* tileheightObj = cJSON_GetObjectItem(level, "tileheight");

        self->tilemapWidth = (u32)cJSON_GetNumberValue(widthObj);
        self->tilemapHeight = (u32)cJSON_GetNumberValue(heightObj);
        self->tileWidth = (u16)cJSON_GetNumberValue(tilewidthObj);
        self->tileHeight = (u16)cJSON_GetNumberValue(tileheightObj);

        const cJSON* layersObj = cJSON_GetObjectItem(level, "layers");

        const cJSON* spritesObj = cJSON_GetArrayItem(layersObj, 0);
        const cJSON* dataArray = cJSON_GetObjectItem(spritesObj, "data");
        usize dataLength = cJSON_GetArraySize(dataArray);

        self->tilelayer = malloc(sizeof(u16) * dataLength);
        self->tilelayerLength = dataLength;

        for (usize i = 0; i < dataLength; ++i)
        {
            const cJSON* spriteObj = cJSON_GetArrayItem(dataArray, i);
            u16 sprite = (u16)cJSON_GetNumberValue(spriteObj);

            self->tilelayer[i] = sprite;
        }

        const cJSON* colliders = cJSON_GetArrayItem(layersObj, 1);
        const cJSON* objects = cJSON_GetObjectItem(colliders, "objects");
        usize objectsLength = cJSON_GetArraySize(objects);

        for (usize i = 0; i < objectsLength; ++i)
        {
            const cJSON* collider = cJSON_GetArrayItem(objects, i);

            const cJSON* xObj = cJSON_GetObjectItem(collider, "x");
            const cJSON* yObj = cJSON_GetObjectItem(collider, "y");
            const cJSON* widthObj = cJSON_GetObjectItem(collider, "width");
            const cJSON* heightObj = cJSON_GetObjectItem(collider, "height");

            float x = (float)cJSON_GetNumberValue(xObj);
            float y = (float)cJSON_GetNumberValue(yObj);
            float width = (float)cJSON_GetNumberValue(widthObj);
            float height = (float)cJSON_GetNumberValue(heightObj);

            ECreateBlock(self, x, y, width, height);
        }

        cJSON_Delete(level);
        free(buffer);
    }

    // TODO(thismarvin): Put this into level.json somehow...
    ECreatePlayer(self, 8, 8);
}

usize SceneGetEntityCount(Scene* self)
{
    return self->nextEntity;
}

void SceneUpdate(Scene* self)
{
    if (IsKeyPressed(KEY_F3))
    {
        self->debugging = !self->debugging;
    }

    for (usize i = 0; i < self->nextEntity; ++i)
    {
        SSmoothUpdate(&self->components, i);
        SPlayerUpdate(&self->components, i);
        SKineticUpdate(&self->components, i);
        SCollisionUpdate(&self->components, self->nextEntity, i);
    }
}

void SceneDraw(Scene* self, Texture2D* atlas)
{
    // Draw Tilemap.
    for (usize i = 0; i < self->tilelayerLength; ++i)
    {
        if (self->tilelayer[i] == 0)
        {
            continue;
        }

        u16 sprite = self->tilelayer[i] - 1;

        Vector2 position = (Vector2)
        {
            .x = (i % self->tilemapWidth) * self->tileWidth,
            .y = (i / self->tilemapWidth) * self->tileHeight
        };

        Rectangle source = (Rectangle)
        {
            .x = (sprite % 8) * self->tileWidth,
            .y = (sprite / 8) * self->tileHeight,
            .width = self->tileWidth,
            .height = self->tileHeight
        };

        DrawTextureRec(*atlas, source, position, WHITE);
    }

    for (usize i = 0; i < self->nextEntity; ++i)
    {
        SSpriteDraw(&self->components, atlas, i);

        if (self->debugging)
        {
            SDebugDraw(&self->components, i);
        }
    }

    if (self->debugging)
    {
        DrawFPS(8, 8);
    }
}

void SceneDestroy(Scene* self)
{
    free(self->tilelayer);
}
