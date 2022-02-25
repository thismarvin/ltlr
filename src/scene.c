#include "context.h"
#include "entities.h"
#include "raymath.h"
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

    // Setup Camera.
    {
        Vector2 offset = { 0, 0 };
        Vector2 target = { 0, 0 };

        self->camera.offset = offset;
        self->camera.target = target;
        self->camera.rotation = 0;
        self->camera.zoom = CTX_ZOOM;
    }

    // Level Loading.
    {
        // TODO(thismarvin): Should we crawl the resource directory to find levels?

#if defined(PLATFORM_WEB)
        char* levels[2] = { "./src/resources/build/level_00.json", "./src/resources/build/level_01.json" };
#else
        char* levels[2] = { "./resources/build/level_00.json", "./resources/build/level_01.json" };
#endif

        self->segmentsLength = 2;
        self->segments = malloc(sizeof(LevelSegment) * self->segmentsLength);

        self->bounds = (Rectangle)
        {
            .x = 0,
            .y = 0,
            .width = 0,
            .height = 180,
        };

        Vector2 offset = VECTOR2_ZERO;

        for (usize i = 0; i < self->segmentsLength; ++i)
        {
            LevelSegmentInit(&self->segments[i], levels[i]);

            for (usize j = 0; j < self->segments[i].collidersLength; ++j)
            {
                Rectangle collider = self->segments[i].colliders[j];

                ECreateBlock(self, offset.x + collider.x, offset.y + collider.y, collider.width, collider.height);
            }

            self->bounds.width += self->segments[i].bounds.width;
            offset.x += self->bounds.width;
        }
    }

    // TODO(thismarvin): Put this into level.json somehow...
    self->player = ECreatePlayer(self, 8, 8);

    ECreateWalker(self, 16 * 16, 8 * 16);
    ECreateWalker(self, 16 * 16, 0 * 16);
    ECreateWalker(self, 16 * 16, 4 * 16);
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
        SWalkerUpdate(&self->components, i);
        SKineticUpdate(&self->components, i);
        SCollisionUpdate(&self->components, self->nextEntity, i);
    }
}

void SceneDraw(Scene* self, Texture2D* atlas)
{
    // TODO(thismarvin): This should ultimately be a System... somehow...
    // Update Camera
    {
        // TODO(thismarvin): Should this exist in Context?
        int viewportWidth = 320;
        int viewportHeight = 180;

        CDimension dimension = self->components.dimensions[self->player];

        Vector2 previous = self->components.smooths[self->player].previous;
        Vector2 current = self->components.positions[self->player].value;

        Vector2 position = Vector2Lerp(previous, current, ContextGetAlpha());

        position = Vector2Add(position, Vector2Create(dimension.width * 0.5, dimension.height * 0.5));

        self->camera.offset.x = (-position.x + viewportWidth * 0.5) * self->camera.zoom;
        self->camera.offset.y = (-position.y + viewportHeight * 0.5) * self->camera.zoom;

        self->camera.offset.x = MIN(RectangleLeft(self->bounds), self->camera.offset.x);
        self->camera.offset.x = MAX(-(RectangleRight(self->bounds) - viewportWidth) * self->camera.zoom,
                                    self->camera.offset.x);

        self->camera.offset.y = MIN(RectangleTop(self->bounds), self->camera.offset.y);
        self->camera.offset.y = MAX(-(RectangleBottom(self->bounds) - viewportHeight) * self->camera.zoom,
                                    self->camera.offset.y);
    }

    BeginMode2D(self->camera);

    // Draw Tilemap.
    {
        Vector2 offset = VECTOR2_ZERO;

        for (usize i = 0; i < self->segmentsLength; ++i)
        {
            for (usize j = 0; j < self->segments[i].spritesLength; ++j)
            {
                if (self->segments[i].sprites[j] == 0)
                {
                    continue;
                }

                u16 sprite = self->segments[i].sprites[j] - 1;

                Vector2 position = (Vector2)
                {
                    .x = (j % self->segments[i].tilemapWidth) * self->segments[i].tileWidth,
                    .y = (j / self->segments[i].tilemapWidth) * self->segments[i].tileHeight
                };

                position = Vector2Add(position, offset);

                Rectangle source = (Rectangle)
                {
                    .x = (sprite % self->segments[i].tilesetColumns) * self->segments[i].tileWidth,
                    .y = (sprite / self->segments[i].tilesetColumns) * self->segments[i].tileHeight,
                    .width = self->segments[i].tileWidth,
                    .height = self->segments[i].tileHeight
                };

                DrawTextureRec(*atlas, source, position, WHITE);
            }

            offset.x += self->segments[i].bounds.width;
        }
    }

    for (usize i = 0; i < self->nextEntity; ++i)
    {
        SSpriteDraw(&self->components, atlas, i);

        if (self->debugging)
        {
            SDebugDraw(&self->components, i);
        }
    }

    EndMode2D();

    if (self->debugging)
    {
        DrawFPS(8, 8);
    }
}

void SceneDestroy(Scene* self)
{
    for (usize i = 0; i < self->segmentsLength; ++i)
    {
        LevelSegmentDestroy(
            &self->segments[i]);
    }
}
