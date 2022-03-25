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

Components* SceneGetComponents(Scene* self)
{
    return &self->components;
}

void SceneEnableComponent(Scene* self, usize entity, usize tag)
{
    self->components.tags[entity] |= tag;
}

void SceneDisableComponent(Scene* self, usize entity, usize tag)
{
    self->components.tags[entity] &= ~tag;
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

void SceneRaiseEvent(Scene* self, const Event* event)
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

static void SceneSetupInput(Scene* self)
{
    InputProfile profile = InputProfileCreate(5);

    // Keyboard.
    {
        {
            KeyboardBinding binding = KeyboardBindingCreate("left", 2);

            KeyboardBindingAddKey(&binding, KEY_LEFT);
            KeyboardBindingAddKey(&binding, KEY_A);

            InputProfileAddKeyboardBinding(&profile, binding);
        }

        {
            KeyboardBinding binding = KeyboardBindingCreate("right", 2);

            KeyboardBindingAddKey(&binding, KEY_RIGHT);
            KeyboardBindingAddKey(&binding, KEY_D);

            InputProfileAddKeyboardBinding(&profile, binding);
        }

        {
            KeyboardBinding binding = KeyboardBindingCreate("jump", 2);

            KeyboardBindingAddKey(&binding, KEY_Z);
            KeyboardBindingAddKey(&binding, KEY_SPACE);

            KeyboardBindingSetBuffer(&binding, CTX_DT * 8);

            InputProfileAddKeyboardBinding(&profile, binding);
        }
    }

    // Gamepad.
    {
        // Buttons.
        {
            {
                GamepadBinding binding = GamepadBindingCreate("left", 1);

                GamepadBindingAddButton(&binding, GAMEPAD_BUTTON_LEFT_FACE_LEFT);

                InputProfileAddGamepadBinding(&profile, binding);
            }

            {
                GamepadBinding binding = GamepadBindingCreate("right", 1);

                GamepadBindingAddButton(&binding, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);

                InputProfileAddGamepadBinding(&profile, binding);
            }

            {
                GamepadBinding binding = GamepadBindingCreate("jump", 2);

                GamepadBindingAddButton(&binding, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
                GamepadBindingAddButton(&binding, GAMEPAD_BUTTON_RIGHT_FACE_UP);

                GamepadBindingSetBuffer(&binding, CTX_DT * 8);

                InputProfileAddGamepadBinding(&profile, binding);
            }
        }

        // Axes.
        {
            f32 threshold = 0.25;

            {
                AxisBinding binding = AxisBindingCreate("left", 2, ORD_LESS, -threshold);

                AxisBindingAddAxis(&binding, GAMEPAD_AXIS_LEFT_X);
                AxisBindingAddAxis(&binding, GAMEPAD_AXIS_RIGHT_X);

                InputProfileAddAxisBinding(&profile, binding);
            }

            {
                AxisBinding binding = AxisBindingCreate("right", 2, ORD_GREATER, threshold);

                AxisBindingAddAxis(&binding, GAMEPAD_AXIS_LEFT_X);
                AxisBindingAddAxis(&binding, GAMEPAD_AXIS_RIGHT_X);

                InputProfileAddAxisBinding(&profile, binding);
            }
        }
    }

    self->input = InputHandlerCreate(0);

    InputHandlerSetProfile(&self->input, profile);
}

// Returns the maximum value the dimensions of a given region can be multiplied by and still fit
// within a given container.
static f32 CalculateZoom(const Rectangle region, const Rectangle container)
{
    // Assume we need letterboxing.
    f32 zoom = container.width / region.width;

    // Check if pillarboxing is more appropriate.
    if (region.height * zoom > container.height)
    {
        zoom = container.height / region.height;
    }

    return zoom;
}

static void SceneSetupTargetTexture(Scene* self)
{
    self->trueResolution = (Rectangle)
    {
        .x = 0,
        .y = 0,
        .width = CTX_VIEWPORT_WIDTH,
        .height = CTX_VIEWPORT_HEIGHT,
    };

    // TODO(thismarvin): Expose a "Render Resolution" option.
    Rectangle renderResolution = (Rectangle)
    {
        .x = 0,
        .y = 0,
        .width = GetMonitorWidth(GetCurrentMonitor()),
        .height = GetMonitorHeight(GetCurrentMonitor()),
    };

    f32 zoom = CalculateZoom(self->trueResolution, renderResolution);

    // Ensure that the render resolution uses integer scaling.
    zoom = floor(zoom);

    self->targetTexture = LoadRenderTexture(CTX_VIEWPORT_WIDTH * zoom, CTX_VIEWPORT_HEIGHT * zoom);

    // Setup Camera.
    {
        Vector2 offset = VECTOR2_ZERO;
        Vector2 target = VECTOR2_ZERO;

        self->camera.offset = offset;
        self->camera.target = target;
        self->camera.rotation = 0;
        self->camera.zoom = zoom;
    }
}

static void SceneSetupLevelSegments(Scene* self)
{
    // TODO(thismarvin): Should we crawl the resource directory to find levels?

#if defined(PLATFORM_WEB)
    const char* levels[2] = { "./src/resources/build/level_00.json", "./src/resources/build/level_01.json" };
#else
    const char* levels[2] = { "./resources/build/level_00.json", "./resources/build/level_01.json" };
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

    for (usize i = 0; i < self->segmentsLength; ++i)
    {
        LevelSegmentInit(&self->segments[i], levels[i]);
        self->bounds.width += self->segments[i].bounds.width;
    }
}

static void SceneStart(Scene* self)
{
    memset(&self->components.tags, 0, sizeof(u64) * MAX_ENTITIES);

    // Initialize EntityManager.
    {
        self->entityManager.nextEntity = 0;
        self->entityManager.nextFreeSlot = 0;
    }

    // Initialize EventManager.
    {
        self->eventManager.nextEvent = 0;
        self->eventManager.nextFreeSlot = 0;

        for (usize i = 0; i < MAX_EVENTS; ++i)
        {
            self->eventManager.events[i].tag = EVENT_NONE;
        }
    }

    // Populate level
    {
        Vector2 offset = VECTOR2_ZERO;

        for (usize i = 0; i < self->segmentsLength; ++i)
        {
            for (usize j = 0; j < self->segments[i].collidersLength; ++j)
            {
                Rectangle collider = self->segments[i].colliders[j];

                ECreateBlock(self, offset.x + collider.x, offset.y + collider.y, collider.width, collider.height);
            }

            offset.x += self->segments[i].bounds.width;
        }
    }

    // TODO(thismarvin): Put this into level.json somehow...
    self->player = ECreatePlayer(self, 16 * 1, 16 * - 4);

    ECreateWalker(self, 16 * 16, 8 * 16);
    ECreateWalker(self, 16 * 16, 0 * 16);
    ECreateWalker(self, 16 * 16, 4 * 16);
}

void SceneInit(Scene* self)
{
    SceneSetupInput(self);
    SceneSetupTargetTexture(self);
    SceneSetupLevelSegments(self);

    self->debugging = false;

    SceneStart(self);
}

void SceneUpdate(Scene* self)
{
    InputHandlerUpdate(&self->input);

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
        SPlayerMortalUpdate(self, i);
    }
}

static void SceneDrawTargetTexture(const Scene* self)
{
    BeginDrawing();

    Rectangle screenResolution = (Rectangle)
    {
        .x = 0,
        .y = 0,
        .width = GetScreenWidth(),
        .height = GetScreenHeight(),
    };

    f32 zoom = CalculateZoom(self->trueResolution, screenResolution);

    // TODO(thismarvin): Expose "preferIntegerScaling" option.
    // Prefer integer scaling.
    zoom = floor(zoom);

    i32 width = CTX_VIEWPORT_WIDTH * zoom;
    i32 height = CTX_VIEWPORT_HEIGHT * zoom;

    Rectangle source = (Rectangle)
    {
        .x = 0,
        .y = 0,
        .width = self->targetTexture.texture.width,
        .height = -self->targetTexture.texture.height,
    };

    Rectangle destination = (Rectangle)
    {
        .x = floor(screenResolution.width * 0.5),
        .y = floor(screenResolution.height * 0.5),
        .width = width,
        .height = height,
    };

    Vector2 origin = (Vector2)
    {
        .x = floor(width * 0.5),
        .y = floor(height * 0.5),
    };

    ClearBackground(BLACK);
    DrawTexturePro(self->targetTexture.texture, source, destination, origin, 0, WHITE);

    EndDrawing();
}

void SceneDraw(Scene* self, Texture2D* atlas)
{
    // TODO(thismarvin): This should ultimately be a System... somehow...
    // Update Camera
    {
        CDimension dimension = self->components.dimensions[self->player];

        if (!self->components.players[self->player].dead)
        {
            Vector2 previous = self->components.smooths[self->player].previous;
            Vector2 current = self->components.positions[self->player].value;

            Vector2 position = Vector2Lerp(previous, current, ContextGetAlpha());

            position = Vector2Add(position, Vector2Create(dimension.width * 0.5, dimension.height * 0.5));

            self->camera.offset.x = (-position.x + CTX_VIEWPORT_WIDTH * 0.5) * self->camera.zoom;
            self->camera.offset.y = (-position.y + CTX_VIEWPORT_HEIGHT * 0.5) * self->camera.zoom;

            self->camera.offset.x = MIN(RectangleLeft(self->bounds), self->camera.offset.x);
            self->camera.offset.x = MAX(-(RectangleRight(self->bounds) - CTX_VIEWPORT_WIDTH) *
                                        self->camera.zoom,
                                        self->camera.offset.x);

            self->camera.offset.y = MIN(RectangleTop(self->bounds), self->camera.offset.y);
            self->camera.offset.y = MAX(-(RectangleBottom(self->bounds) - CTX_VIEWPORT_HEIGHT) *
                                        self->camera.zoom,
                                        self->camera.offset.y);
        }
    }

    BeginTextureMode(self->targetTexture);
    BeginMode2D(self->camera);
    {
        ClearBackground(WHITE);

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
    }
    EndMode2D();
    EndTextureMode();

    SceneDrawTargetTexture(self);
}

void SceneReset(Scene* self)
{
    SceneStart(self);
}

void SceneDestroy(Scene* self)
{
    for (usize i = 0; i < self->segmentsLength; ++i)
    {
        LevelSegmentDestroy(&self->segments[i]);
    }

    UnloadRenderTexture(self->targetTexture);
}
