#include "./ecs/entities.h"
#include "./ecs/systems.h"
#include "./palette/p8.h"
#include "context.h"
#include "scene.h"
#include <assert.h>
#include <raymath.h>
#include <string.h>

typedef struct
{
    Scene* scene;
    Rectangle cameraBounds;
} RenderFnParams;

typedef void (*RenderFn)(const RenderFnParams*);

void SceneDeferEnableComponent(Scene* self, const usize entity, const usize tag)
{
    SceneSubmitCommand(self, CommandCreateEnableComponent(entity, tag));
}

void SceneDeferDisableComponent(Scene* self, const usize entity, const usize tag)
{
    SceneSubmitCommand(self, CommandCreateDisableComponent(entity, tag));
}

static usize SceneAllocateEntity(Scene* self)
{
    EntityManager* entityManager = &self->m_entityManager;

    if (DequeGetSize(&entityManager->m_recycledEntityIndices) != 0)
    {
        return DEQUE_POP_FRONT(&entityManager->m_recycledEntityIndices, usize);
    }

    // No used indices, use next available fresh one.
    usize next = MIN(entityManager->m_nextFreshEntityIndex, MAX_ENTITIES - 1);

    entityManager->m_nextFreshEntityIndex = entityManager->m_nextFreshEntityIndex + 1;
    entityManager->m_nextFreshEntityIndex = MIN(entityManager->m_nextFreshEntityIndex, MAX_ENTITIES);

    if (entityManager->m_nextFreshEntityIndex == MAX_ENTITIES)
    {
        TraceLog(LOG_WARNING, "Maximum amount of entities reached.");
    }

    return next;
}

usize SceneDeferAddEntity(Scene* self, EntityBuilder entityBuilder)
{
    const usize entity = SceneAllocateEntity(self);

    SceneSubmitCommand(self, CommandCreateSetTag(entity, entityBuilder.tags));

    for (usize i = 0; i < DequeGetSize(&entityBuilder.components); ++i)
    {
        const Component* component = &DEQUE_GET_UNCHECKED(&entityBuilder.components, Component, i);

        SceneSubmitCommand(self, CommandCreateSetComponent(entity, component));
    }

    DequeDestroy(&entityBuilder.components);

    return entity;
}

void SceneDeferDeallocateEntity(Scene* self, const usize entity)
{
    SceneSubmitCommand(self, CommandCreateDeallocateEntity(entity));
}

bool SceneEntityHasDependencies(const Scene* self, usize entity, u64 dependencies)
{
    return (self->components.tags[entity] & dependencies) == dependencies;
}

void SceneSubmitCommand(Scene* self, Command command)
{
    DequePushFront(&self->commands, &command);
}

static void SceneExecuteSetTag(Scene* self, const CommandSetTag* setTag)
{
    self->components.tags[setTag->entity] = setTag->tag;
}

static void SceneExecuteSetComponent(Scene* self, const CommandSetComponent* setCommand)
{
    usize entity = setCommand->entity;
    const Component* component = &setCommand->component;

    switch (component->tag)
    {
        case TAG_NONE:
        {
            break;
        }

        case TAG_POSITION:
        {
            SCENE_SET_COMPONENT(self, entity, component->position);
            break;
        }

        case TAG_DIMENSION:
        {
            SCENE_SET_COMPONENT(self, entity, component->dimension);
            break;
        }

        case TAG_COLOR:
        {
            SCENE_SET_COMPONENT(self, entity, component->color);
            break;
        }

        case TAG_SPRITE:
        {
            SCENE_SET_COMPONENT(self, entity, component->sprite);
            break;
        }

        case TAG_KINETIC:
        {
            SCENE_SET_COMPONENT(self, entity, component->kinetic);
            break;
        }

        case TAG_SMOOTH:
        {
            SCENE_SET_COMPONENT(self, entity, component->smooth);
            break;
        }

        case TAG_PLAYER:
        {
            SCENE_SET_COMPONENT(self, entity, component->player);
            break;
        }

        case TAG_COLLIDER:
        {
            SCENE_SET_COMPONENT(self, entity, component->collider);
            break;
        }

        case TAG_MORTAL:
        {
            SCENE_SET_COMPONENT(self, entity, component->mortal);
            break;
        }

        case TAG_DAMAGE:
        {
            SCENE_SET_COMPONENT(self, entity, component->damage);
            break;
        }

        case TAG_FLEETING:
        {
            SCENE_SET_COMPONENT(self, entity, component->fleeting);
            break;
        }
    }
}

void SceneExecuteDeallocateEntity(Scene* self, const CommandDeallocateEntity* deallocateEntity)
{
    const usize entity = deallocateEntity->entity;

    self->components.tags[entity] = TAG_NONE;
    DequePushFront(&self->m_entityManager.m_recycledEntityIndices, &entity);
}

void SceneExecuteEnableComponent(Scene* self, const CommandEnableComponent* enableCommand)
{
    self->components.tags[enableCommand->entity] |= enableCommand->componentTag;
}

void SceneExecuteRemoveComponent(Scene* self, const CommandDisableComponent* removeCommand)
{
    self->components.tags[removeCommand->entity] &= ~removeCommand->componentTag;
}

void SceneExecuteCommands(Scene* self)
{
    Deque* commands = &self->commands;

    for (usize i = 0; i < DequeGetSize(commands); ++i)
    {
        const Command* command = &DEQUE_GET_UNCHECKED(commands, Command, i);

        switch (command->type)
        {
            case CT_SET_TAG:
            {
                SceneExecuteSetTag(self, &command->setTag);
                break;
            }

            case CT_SET_COMPONENT:
            {
                SceneExecuteSetComponent(self, &command->setComponent);
                break;
            }

            case CT_DEALLOCATE_ENTITY:
            {
                SceneExecuteDeallocateEntity(self, &command->deallocateEntity);
                break;
            }

            case CT_ENABLE_COMPONENT:
            {
                SceneExecuteEnableComponent(self, &command->enableComponent);
                break;
            }

            case CT_DISABLE_COMPONENT:
            {
                SceneExecuteRemoveComponent(self, &command->disableComponent);
                break;
            }

            default:
            {
                break;
            }
        }
    }

    DequeClear(commands);
}

usize SceneGetEntityCount(const Scene* self)
{
    return self->m_entityManager.m_nextFreshEntityIndex;
}

static void SceneSetupContent(Scene* self)
{
#if defined(PLATFORM_WEB)
    self->atlas = LoadTexture("./build/content/atlas.png");
#else
    self->atlas = LoadTexture("./content/atlas.png");
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
    const char* levels[2] = { "./build/content/level_00.json", "./build/content/level_01.json" };
#else
    const char* levels[2] = { "./content/level_00.json", "./content/level_01.json" };
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

    // TODO(austin0209): Use clear instead of re-initializing?
    self->commands = DEQUE_OF(Command);

    // Initialize EntityManager.
    {
        self->m_entityManager.m_nextFreshEntityIndex = 0;
        self->m_entityManager.m_recycledEntityIndices = DEQUE_WITH_CAPACITY(usize, MAX_ENTITIES);
    }

    // Populate level.
    {
        Vector2 offset = VECTOR2_ZERO;

        for (usize i = 0; i < self->segmentsLength; ++i)
        {
            for (usize j = 0; j < self->segments[i].collidersLength; ++j)
            {
                LevelCollider collider = self->segments[i].colliders[j];

                Rectangle aabb = (Rectangle)
                {
                    .x = collider.aabb.x + offset.x,
                    .y = collider.aabb.y + offset.y,
                    .width = collider.aabb.width,
                    .height = collider.aabb.height,
                };

                SceneDeferAddEntity
                (
                    self,
                    BlockCreate(aabb, collider.resolutionSchema, collider.layer)
                );
            }

            offset.x += self->segments[i].bounds.width;
        }
    }

    // TODO(thismarvin): Put this into level.json somehow...
    self->player = SceneDeferAddEntity(self, PlayerCreate(16 * 1, 16 * -4));
    self->fog = SceneDeferAddEntity(self, FogCreate());

    SceneDeferAddEntity(self, WalkerCreate(16 * 16, 16 * 6));
    SceneDeferAddEntity(self, WalkerCreate(16 * 16, 16 * 7));
    SceneDeferAddEntity(self, WalkerCreate(16 * 16, 16 * 8));
}

static void SceneReset(Scene* self)
{
    DequeDestroy(&self->commands);
    DequeDestroy(&self->m_entityManager.m_recycledEntityIndices);

    SceneStart(self);

    self->resetRequested = false;
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

static void SceneCheckEndCondition(Scene* self)
{
    assert(SceneEntityHasDependencies(self, self->player, TAG_POSITION));
    assert(SceneEntityHasDependencies(self, self->fog, TAG_POSITION));

    const CPosition* fogPosition = SCENE_GET_COMPONENT_PTR(self, fogPosition, self->fog);
    const CPosition* playerPosition = SCENE_GET_COMPONENT_PTR(self, playerPosition, self->player);

    const f32 distance = fogPosition->value.x - playerPosition->value.x;

    if (distance > CTX_VIEWPORT_WIDTH * 0.5f)
    {
        SceneDeferReset(self);
    }
}

void SceneUpdate(Scene* self)
{
    InputHandlerUpdate(&self->input);

    if (IsKeyPressed(KEY_EQUAL))
    {
        self->debugging = !self->debugging;
    }

    if (self->resetRequested)
    {
        SceneReset(self);
    }

    SceneExecuteCommands(self);
    SceneCheckEndCondition(self);

    for (usize i = 0; i < SceneGetEntityCount(self); ++i)
    {
        SFleetingUpdate(self, i);

        SSmoothUpdate(self, i);

        PlayerInputUpdate(self, i);

        SKineticUpdate(self, i);

        SCollisionUpdate(self, i);

        PlayerPostCollisionUpdate(self, i);
        PlayerMortalUpdate(self, i);

        FogUpdate(self, i);
    }
}

// Return a Rectangle that is within the scene's bounds and centered on a given entity.
static Rectangle SceneCalculateActionCameraBounds(const Scene* self, const usize targetEntity)
{
    if ((self->components.tags[targetEntity] & (TAG_POSITION)) != (TAG_POSITION))
    {
        return self->trueResolution;
    }

    Vector2 cameraPosition = self->components.positions[targetEntity].value;

    if ((self->components.tags[targetEntity] & (TAG_SMOOTH)) == (TAG_SMOOTH))
    {
        const CSmooth* smooth = &self->components.smooths[self->player];

        cameraPosition = Vector2Lerp(smooth->previous, cameraPosition, ContextGetAlpha());
    }

    if ((self->components.tags[targetEntity] & (TAG_DIMENSION)) == (TAG_DIMENSION))
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

    // Camera y-axis collision.
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

static void RenderRootLayer(UNUSED const RenderFnParams* params)
{
    ClearBackground(P8_BLUE);
}

static void RenderBackgroundLayer(UNUSED const RenderFnParams* params)
{
    ClearBackground(COLOR_TRANSPARENT);

    // TODO(thismarvin): Draw trees here.
}

static void RenderTargetLayer(const RenderFnParams* params)
{
    ClearBackground(COLOR_TRANSPARENT);

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
    ClearBackground(COLOR_TRANSPARENT);

    for (usize i = 0; i < SceneGetEntityCount(params->scene); ++i)
    {
        CloudParticleDraw(params->scene, i);
    }

    for (usize i = 0; i < SceneGetEntityCount(params->scene); ++i)
    {
        FogDraw(params->scene, i);
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

void SceneDeferReset(Scene* self)
{
    self->resetRequested = true;
}

void SceneDestroy(Scene* self)
{
    UnloadTexture(self->atlas);

    for (usize i = 0; i < self->segmentsLength; ++i)
    {
        LevelSegmentDestroy(&self->segments[i]);
    }

    DequeDestroy(&self->commands);
    DequeDestroy(&self->m_entityManager.m_recycledEntityIndices);

    UnloadRenderTexture(self->backgroundLayer);
    UnloadRenderTexture(self->targetLayer);
    UnloadRenderTexture(self->foregroundLayer);
}
