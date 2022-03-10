#include "context.h"
#include "entities.h"
#include "raymath.h"
#include "scene.h"
#include "systems.h"
#include <assert.h>
#include <string.h>

static void EntityManagerPushFree(Scene* self, usize value)
{
    EntityManager* entityManager = &self->entityManager;

    assert(entityManager->nextFreeSlot < MAX_ENTITIES);

    entityManager->freeSlots[entityManager->nextFreeSlot] = value;
    entityManager->nextFreeSlot += 1;
}

static usize EntityManagerPopFree(Scene* self)
{
    EntityManager* entityManager = &self->entityManager;

    assert(entityManager->nextFreeSlot > 0);

    usize slot = entityManager->freeSlots[entityManager->nextFreeSlot - 1];
    entityManager->nextFreeSlot -= 1;

    return slot;
}

static void EventManagerPushFree(Scene* self, usize value)
{
    EventManager* eventManager = &self->eventManager;

    assert(eventManager->nextFreeSlot < MAX_EVENTS);

    eventManager->freeSlots[eventManager->nextFreeSlot] = value;
    eventManager->nextFreeSlot += 1;
}

static usize EventManagerPopFree(Scene* self)
{
    EventManager* eventManager = &self->eventManager;

    assert(eventManager->nextFreeSlot < MAX_EVENTS);

    usize slot = eventManager->freeSlots[eventManager->nextFreeSlot - 1];
    eventManager->nextFreeSlot -= 1;

    return slot;
}

usize SceneAllocateEntity(Scene* self)
{
    EntityManager* entityManager = &self->entityManager;

    if (entityManager->nextFreeSlot == 0)
    {
        usize next = MIN(entityManager->nextEntity, MAX_ENTITIES - 1);

        entityManager->nextEntity = entityManager->nextEntity + 1;
        entityManager->nextEntity = MIN(entityManager->nextEntity, MAX_ENTITIES);

        if (entityManager->nextEntity == MAX_ENTITIES)
        {
            TraceLog(LOG_WARNING, "Maximum amount of entities reached.");
        }

        return next;
    }

    return EntityManagerPopFree(self);
}

void SceneDeallocateEntity(Scene* self, usize entity)
{
    self->components.tags[entity] = 0;
    EntityManagerPushFree(self, entity);
}

void SceneRaiseEvent(Scene* self, Event* event)
{
    if (self->eventManager.nextFreeSlot == 0)
    {
        usize next = MIN(self->eventManager.nextEvent, MAX_EVENTS - 1);

        self->eventManager.nextEvent = self->eventManager.nextEvent + 1;
        self->eventManager.nextEvent = MIN(self->eventManager.nextEvent, MAX_EVENTS);

        memcpy(&self->eventManager.events[next], event, sizeof(Event));

        if (self->eventManager.nextEvent == MAX_EVENTS)
        {
            TraceLog(LOG_WARNING, "Maximum amount of events reached.");
        }

        return;
    }

    usize next = EventManagerPopFree(self);
    memcpy(&self->eventManager.events[next], event, sizeof(Event));
}

void SceneConsumeEvent(Scene* self, usize eventIndex)
{
    self->eventManager.events[eventIndex].tag = EVENT_NONE;
    EventManagerPushFree(self, eventIndex);
}

usize SceneGetEntityCount(Scene* self)
{
    return self->entityManager.nextEntity;
}

usize SceneGetEventCount(Scene* self)
{
    return self->eventManager.nextEvent;
}

void SceneInit(Scene* self)
{
    memset(&self->components.tags, 0, sizeof(u64) * MAX_ENTITIES);

    // TODO(thismarvin): Do we need to initialize positions, dimensions, etc.?

    // Initialize EntityManager.
    {
        self->entityManager.nextEntity = 0;
        self->entityManager.nextFreeSlot = 0;
    }

    // Initialize EventManager
    {
        self->eventManager.nextEvent = 0;
        self->eventManager.nextFreeSlot = 0;

        for (usize i = 0; i < MAX_EVENTS; ++i)
        {
            self->eventManager.events[i].tag = EVENT_NONE;
        }
    }

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
    self->player = ECreatePlayer(self, 16 * 1, 16 * - 4);

    ECreateWalker(self, 16 * 16, 8 * 16);
    ECreateWalker(self, 16 * 16, 0 * 16);
    ECreateWalker(self, 16 * 16, 4 * 16);
}

void SceneUpdate(Scene* self)
{
    if (IsKeyPressed(KEY_EQUAL))
    {
        self->debugging = !self->debugging;
    }

    for (usize i = 0; i < SceneGetEntityCount(self); ++i)
    {
        SSmoothUpdate(self, i);

        SPlayerInputUpdate(self, i);

        SKineticUpdate(self, i);

        SCollisionUpdate(self, i);

        SPlayerCollisionUpdate(self, i);
        SWalkerCollisionUpdate(self, i);
    }
}

void SceneDraw(Scene* self, Texture2D* atlas)
{
    // TODO(thismarvin): This should ultimately be a System... somehow...
    // Update Camera
    {
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

    for (usize i = 0; i < SceneGetEntityCount(self); ++i)
    {
        SSpriteDraw(self, atlas, i);

        if (self->debugging)
        {
            SDebugDraw(self, i);
        }
    }

    EndMode2D();

    if (self->debugging)
    {
        DrawFPS(8, 8);
    }
}

void SceneReset(Scene* self)
{
    SceneDestroy(self);
    SceneInit(self);
}

void SceneDestroy(Scene* self)
{
    for (usize i = 0; i < self->segmentsLength; ++i)
    {
        LevelSegmentDestroy(
            &self->segments[i]);
    }
}
