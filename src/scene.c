#include "context.h"
#include "entities.h"
#include "raymath.h"
#include "scene.h"
#include "systems.h"
#include <string.h>

typedef struct
{
    Scene* scene;
    Rectangle cameraBounds;
} RenderFnParams;

typedef void (*RenderFn)(const RenderFnParams*);

Components* SceneGetComponents(const Scene* self)
{
    return (Components*)&self->components;
}

void SceneEnableComponent(Scene* self, const usize entity, const usize tag)
{
    self->components.tags[entity] |= tag;
}

void SceneDisableComponent(Scene* self, const usize entity, const usize tag)
{
    self->components.tags[entity] &= ~tag;
}

usize SceneAllocateEntity(Scene* self)
{
    EntityManager* entityManager = &self->entityManager;

    if (UsizeDequeGetSize(&entityManager->recycledEntityIndices) != 0)
    {
        return UsizeDequePopFront(&entityManager->recycledEntityIndices);
    }

    // No used indices, use next available fresh one.
    usize next = MIN(entityManager->nextFreshEntityIndex, MAX_ENTITIES - 1);

    entityManager->nextFreshEntityIndex = entityManager->nextFreshEntityIndex + 1;
    entityManager->nextFreshEntityIndex = MIN(entityManager->nextFreshEntityIndex, MAX_ENTITIES);

    if (entityManager->nextFreshEntityIndex == MAX_ENTITIES)
    {
        TraceLog(LOG_WARNING, "Maximum amount of entities reached.");
    }

    return next;
}

void SceneDeferDeallocateEntity(Scene* self, const usize entity)
{
    UsizeDequePushFront(&self->entityManager.deferredDeallocations, entity);
}

void SceneFlushEntities(Scene* self)
{
    while (UsizeDequeGetSize(&self->entityManager.deferredDeallocations) > 0)
    {
        usize entity = UsizeDequePopFront(&self->entityManager.deferredDeallocations);

        self->components.tags[entity] = 0;
        UsizeDequePushFront(&self->entityManager.recycledEntityIndices, entity);
    }
}

void SceneRaiseEvent(Scene* self, const Event* event)
{
    if (UsizeDequeGetSize(&self->eventManager.recycledEventIndices) != 0)
    {
        usize next = UsizeDequePopFront(&self->eventManager.recycledEventIndices);
        memcpy(&self->eventManager.events[next], event, sizeof(Event));

        return;
    }

    // No used indices, use next available fresh one.
    usize next = MIN(self->eventManager.nextFreshEventIndex, MAX_EVENTS - 1);

    self->eventManager.nextFreshEventIndex = self->eventManager.nextFreshEventIndex + 1;
    self->eventManager.nextFreshEventIndex = MIN(self->eventManager.nextFreshEventIndex, MAX_EVENTS);

    memcpy(&self->eventManager.events[next], event, sizeof(Event));

    if (self->eventManager.nextFreshEventIndex == MAX_EVENTS)
    {
        TraceLog(LOG_WARNING, "Maximum amount of events reached.");
    }
}

void SceneConsumeEvent(Scene* self, const usize eventIndex)
{
    self->eventManager.events[eventIndex].tag = EVENT_NONE;
    UsizeDequePushFront(&self->eventManager.recycledEventIndices, eventIndex);
}

usize SceneGetEntityCount(const Scene* self)
{
    return self->entityManager.nextFreshEntityIndex;
}

usize SceneGetEventCount(const Scene* self)
{
    return self->eventManager.nextFreshEventIndex;
}

static void SceneSetupContent(Scene* self)
{
#if defined(PLATFORM_WEB)
    self->atlas = LoadTexture("./src/resources/build/atlas.png");
#else
    self->atlas = LoadTexture("./resources/build/atlas.png");
#endif
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

static Rectangle RectangleFromRenderTexture(const RenderTexture renderTexture)
{
    return (Rectangle)
    {
        .x = 0,
        .y = 0,
        .width = renderTexture.texture.width,
        .height = renderTexture.texture.height,
    };
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

static void SceneSetupLayers(Scene* self)
{
    self->trueResolution = (Rectangle)
    {
        .x = 0,
        .y = 0,
        .width = CTX_VIEWPORT_WIDTH,
        .height = CTX_VIEWPORT_HEIGHT,
    };

    // TODO(thismarvin): Expose a "Render Resolution" option.

    // Use the monitor's resolution as the default render resolution.
    const int currentMonitor = GetCurrentMonitor();
    int width = GetMonitorWidth(currentMonitor);
    int height = GetMonitorHeight(currentMonitor);

    // The following is always true for every platform except desktop.
    if (width == 0 || height == 0)
    {
        width = DEFAULT_WINDOW_WIDTH;
        height = DEFAULT_WINDOW_HEIGHT;
    }

    self->renderResolution = (Rectangle)
    {
        .x = 0,
        .y = 0,
        .width = width,
        .height = height,
    };

    f32 zoom = CalculateZoom(self->trueResolution, self->renderResolution);

    // Ensure that the render resolution uses integer scaling.
    zoom = floor(zoom);

    self->rootLayer = LoadRenderTexture(1, 1);
    self->backgroundLayer = LoadRenderTexture(CTX_VIEWPORT_WIDTH, CTX_VIEWPORT_HEIGHT);
    self->targetLayer = LoadRenderTexture(CTX_VIEWPORT_WIDTH * zoom, CTX_VIEWPORT_HEIGHT * zoom);
    self->foregroundLayer = LoadRenderTexture(CTX_VIEWPORT_WIDTH, CTX_VIEWPORT_HEIGHT);
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
        self->entityManager.nextFreshEntityIndex = 0;
        self->entityManager.deferredDeallocations = UsizeDequeCreate(MAX_ENTITIES);
        self->entityManager.recycledEntityIndices = UsizeDequeCreate(MAX_ENTITIES);
    }

    // Initialize EventManager.
    {
        self->eventManager.nextFreshEventIndex = 0;
        self->eventManager.recycledEventIndices = UsizeDequeCreate(MAX_EVENTS);

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
    SceneSetupContent(self);
    SceneSetupInput(self);
    SceneSetupLayers(self);
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
        SFleetingUpdate(self, i);

        SSmoothUpdate(self, i);

        SPlayerInputUpdate(self, i);
        SCloudParticleSpawnUpdate(self, i);

        SKineticUpdate(self, i);

        SCollisionUpdate(self, i);

        SPlayerCollisionUpdate(self, i);
        SWalkerCollisionUpdate(self, i);
        SPlayerMortalUpdate(self, i);
        SCloudParticleCollisionUpdate(self, i);

        SGenericCollisionUpdate(self, i);
    }

    SceneFlushEntities(self);
}

// Return a Rectangle that is within the scene's bounds and centered on a given entity.
static Rectangle SceneCalculateActionCameraBounds(const Scene* self, const usize targetEntity)
{
    if ((self->components.tags[targetEntity] & (tagPosition)) != (tagPosition))
    {
        return self->trueResolution;
    }

    Vector2 cameraPosition = self->components.positions[targetEntity].value;

    if ((self->components.tags[targetEntity] & (tagSmooth)) == (tagSmooth))
    {
        const CSmooth* smooth = &self->components.smooths[self->player];

        cameraPosition = Vector2Lerp(smooth->previous, cameraPosition, ContextGetAlpha());
    }

    if ((self->components.tags[targetEntity] & (tagDimension)) == (tagDimension))
    {
        const CDimension* dimension = &self->components.dimensions[self->player];

        const Vector2 offset = (Vector2)
        {
            .x = dimension->width * 0.5,
            .y = dimension->height * 0.5,
        };

        cameraPosition = Vector2Add(cameraPosition, offset);
    }

    // Camera x-axis collision.
    {
        f32 min = RectangleLeft(self->bounds) + CTX_VIEWPORT_WIDTH * 0.5;
        f32 max = RectangleRight(self->bounds) - CTX_VIEWPORT_WIDTH * 0.5;

        cameraPosition.x = MAX(min, cameraPosition.x);
        cameraPosition.x = MIN(max, cameraPosition.x);
    }

    // Camera y-axis collison.
    {
        f32 min = RectangleTop(self->bounds) + CTX_VIEWPORT_HEIGHT * 0.5;
        f32 max = RectangleBottom(self->bounds) - CTX_VIEWPORT_HEIGHT * 0.5;

        cameraPosition.y = MAX(min, cameraPosition.y);
        cameraPosition.y = MIN(max, cameraPosition.y);
    }

    return (Rectangle)
    {
        .x = cameraPosition.x - CTX_VIEWPORT_WIDTH * 0.5,
        .y = cameraPosition.y - CTX_VIEWPORT_HEIGHT * 0.5,
        .width = CTX_VIEWPORT_WIDTH,
        .height = CTX_VIEWPORT_HEIGHT,
    };
}

static void SceneDrawTilemap(const Scene* self)
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

            DrawTextureRec(self->atlas, source, position, WHITE);
        }

        offset.x += self->segments[i].bounds.width;
    }
}

static void SceneRenderLayer
(
    const RenderTexture* renderTexture,
    const RenderFn fn,
    const RenderFnParams* params
)
{
    const Rectangle bounds = RectangleFromRenderTexture(*renderTexture);
    const f32 zoom = CalculateZoom(params->scene->trueResolution, bounds);

    const Vector2 cameraCenter = (Vector2)
    {
        .x = params->cameraBounds.x + CTX_VIEWPORT_WIDTH * 0.5,
        .y = params->cameraBounds.y + CTX_VIEWPORT_HEIGHT * 0.5,
    };

    const Camera2D camera = (Camera2D)
    {
        .zoom = zoom,
        .offset = Vector2Scale(cameraCenter, -zoom),
        .target = (Vector2)
        {
            .x = -CTX_VIEWPORT_WIDTH * 0.5,
            .y = -CTX_VIEWPORT_HEIGHT * 0.5,
        },
        .rotation = 0,
    };

    BeginTextureMode(*renderTexture);
    BeginMode2D(camera);
    {
        fn(params);
    }
    EndMode2D();
    EndTextureMode();
}

static void SceneDrawLayers(const Scene* self)
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

    // Draw root layer.
    {
        Rectangle source = RectangleFromRenderTexture(self->rootLayer);
        source.height *= -1;

        DrawTexturePro(self->rootLayer.texture, source, destination, origin, 0, WHITE);
    }

    // Draw background layer.
    {
        Rectangle source = RectangleFromRenderTexture(self->backgroundLayer);
        source.height *= -1;

        DrawTexturePro(self->backgroundLayer.texture, source, destination, origin, 0, WHITE);
    }

    // Draw target layer.
    {
        Rectangle source = RectangleFromRenderTexture(self->targetLayer);
        source.height *= -1;

        DrawTexturePro(self->targetLayer.texture, source, destination, origin, 0, WHITE);
    }

    // Draw foreground layer.
    {
        Rectangle source = RectangleFromRenderTexture(self->foregroundLayer);
        source.height *= -1;

        DrawTexturePro(self->foregroundLayer.texture, source, destination, origin, 0, WHITE);
    }

    EndDrawing();
}

static void RenderRootLayer()
{
    ClearBackground((Color)
    {
        41, 173, 255, 255
    });
}

static void RenderBackgroundLayer()
{
    ClearBackground((Color)
    {
        0, 0, 0, 0
    });

    // TODO(thismarvin): Draw trees here.
}

static void RenderTargetLayer(const RenderFnParams* params)
{
    ClearBackground((Color)
    {
        0, 0, 0, 0
    });

    SceneDrawTilemap(params->scene);

    for (usize i = 0; i < SceneGetEntityCount(params->scene); ++i)
    {
        SSpriteDraw(params->scene, i);

        if (params->scene->debugging)
        {
            SDebugDraw(params->scene, i);
        }
    }
}

static void RenderForegroundLayer(const RenderFnParams* params)
{
    ClearBackground((Color)
    {
        0, 0, 0, 0
    });

    for (usize i = 0; i < SceneGetEntityCount(params->scene); ++i)
    {
        SCloudParticleDraw(params->scene, i);
    }
}

void SceneDraw(const Scene* self)
{
    Rectangle actionCameraBounds = SceneCalculateActionCameraBounds(self, self->player);

    const RenderFnParams params = (RenderFnParams)
    {
        .scene = (Scene*)self,
        .cameraBounds = actionCameraBounds,
    };

    SceneRenderLayer(&self->rootLayer, RenderRootLayer, &params);
    SceneRenderLayer(&self->backgroundLayer, RenderBackgroundLayer, &params);
    SceneRenderLayer(&self->targetLayer, RenderTargetLayer, &params);
    SceneRenderLayer(&self->foregroundLayer, RenderForegroundLayer, &params);

    SceneDrawLayers(self);
}

void SceneReset(Scene* self)
{
    UsizeDequeDestroy(&self->entityManager.recycledEntityIndices);
    UsizeDequeDestroy(&self->entityManager.deferredDeallocations);
    UsizeDequeDestroy(&self->eventManager.recycledEventIndices);

    SceneStart(self);
}

void SceneDestroy(Scene* self)
{
    UnloadTexture(self->atlas);

    for (usize i = 0; i < self->segmentsLength; ++i)
    {
        LevelSegmentDestroy(&self->segments[i]);
    }

    UsizeDequeDestroy(&self->entityManager.recycledEntityIndices);
    UsizeDequeDestroy(&self->entityManager.deferredDeallocations);
    UsizeDequeDestroy(&self->eventManager.recycledEventIndices);

    UnloadRenderTexture(self->backgroundLayer);
    UnloadRenderTexture(self->targetLayer);
    UnloadRenderTexture(self->foregroundLayer);
}
