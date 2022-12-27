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
    const usize next = MIN(entityManager->m_nextFreshEntityIndex, MAX_ENTITIES - 1);

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
    const usize entity = setCommand->entity;
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

        case TAG_ANIMATION:
        {
            SCENE_SET_COMPONENT(self, entity, component->animation);
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

static void SceneExecuteCommands(Scene* self)
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
    self->atlas = AtlasCreate("./content/atlas.png");
}

static void SceneSetupInput(Scene* self)
{
    self->defaultActionProfile = InputProfileCreate(5);

    // Keyboard.
    {
        {
            KeyboardBinding binding = KeyboardBindingCreate("left", 2);

            KeyboardBindingAddKey(&binding, KEY_LEFT);
            KeyboardBindingAddKey(&binding, KEY_A);

            InputProfileAddKeyboardBinding(&self->defaultActionProfile, binding);
        }

        {
            KeyboardBinding binding = KeyboardBindingCreate("right", 2);

            KeyboardBindingAddKey(&binding, KEY_RIGHT);
            KeyboardBindingAddKey(&binding, KEY_D);

            InputProfileAddKeyboardBinding(&self->defaultActionProfile, binding);
        }

        {
            KeyboardBinding binding = KeyboardBindingCreate("jump", 2);

            KeyboardBindingAddKey(&binding, KEY_Z);
            KeyboardBindingAddKey(&binding, KEY_SPACE);

            KeyboardBindingSetBuffer(&binding, CTX_DT * 8);

            InputProfileAddKeyboardBinding(&self->defaultActionProfile, binding);
        }
    }

    // Gamepad.
    {
        // Buttons.
        {
            {
                GamepadBinding binding = GamepadBindingCreate("left", 1);

                GamepadBindingAddButton(&binding, GAMEPAD_BUTTON_LEFT_FACE_LEFT);

                InputProfileAddGamepadBinding(&self->defaultActionProfile, binding);
            }

            {
                GamepadBinding binding = GamepadBindingCreate("right", 1);

                GamepadBindingAddButton(&binding, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);

                InputProfileAddGamepadBinding(&self->defaultActionProfile, binding);
            }

            {
                GamepadBinding binding = GamepadBindingCreate("jump", 2);

                GamepadBindingAddButton(&binding, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
                GamepadBindingAddButton(&binding, GAMEPAD_BUTTON_RIGHT_FACE_UP);

                GamepadBindingSetBuffer(&binding, CTX_DT * 8);

                InputProfileAddGamepadBinding(&self->defaultActionProfile, binding);
            }
        }

        // Axes.
        {
            static const f32 threshold = 0.25f;

            {
                AxisBinding binding = AxisBindingCreate("left", 2, ORD_LESS, -threshold);

                AxisBindingAddAxis(&binding, GAMEPAD_AXIS_LEFT_X);
                AxisBindingAddAxis(&binding, GAMEPAD_AXIS_RIGHT_X);

                InputProfileAddAxisBinding(&self->defaultActionProfile, binding);
            }

            {
                AxisBinding binding = AxisBindingCreate("right", 2, ORD_GREATER, threshold);

                AxisBindingAddAxis(&binding, GAMEPAD_AXIS_LEFT_X);
                AxisBindingAddAxis(&binding, GAMEPAD_AXIS_RIGHT_X);

                InputProfileAddAxisBinding(&self->defaultActionProfile, binding);
            }
        }
    }

    self->input = InputHandlerCreate(0);

    InputHandlerSetProfile(&self->input, &self->defaultActionProfile);
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

static RenderTexture GenerateTreeTexture(void)
{
    const RenderTexture renderTexture = LoadRenderTexture(CTX_VIEWPORT_WIDTH, CTX_VIEWPORT_HEIGHT);

    const Camera2D camera = (Camera2D)
    {
        .zoom = 1,
        .offset = VECTOR2_ZERO,
        .target = VECTOR2_ZERO,
        .rotation = 0,
    };

    BeginTextureMode(renderTexture);
    BeginMode2D(camera);
    {
        ClearBackground(COLOR_TRANSPARENT);

        static const u8 value = 240;
        const Color gray = (Color)
        {
            .r = value,
            .g = value,
            .b = value,
            .a = 255,
        };

        const f32 xInitial = renderTexture.texture.width * 0.5;
        const f32 yInitial = 3;
        const i32 offsetInitial = 6;

        // Draw outline.
        {
            const f32 x = xInitial;
            f32 y = yInitial;
            i32 offset = offsetInitial;

            while (y < renderTexture.texture.height * 1.1)
            {
                static const i32 padding = 3;

                const Vector2 a = Vector2Create(x, y - padding);
                const Vector2 b = Vector2Create(x - offset - padding, y + offset);
                const Vector2 c = Vector2Create(x + offset + padding, y + offset);
                DrawTriangle(a, b, c, COLOR_BLACK);

                y += 4;
                offset += 2;
            }
        }

        // Draw primary region.
        {
            const f32 x = xInitial;
            f32 y = yInitial;
            i32 offset = offsetInitial;

            while (y < renderTexture.texture.height * 1.1)
            {
                const Vector2 a = Vector2Create(x, y);
                const Vector2 b = Vector2Create(x - offset, y + offset);
                const Vector2 c = Vector2Create(x + offset, y + offset);
                DrawTriangle(a, b, c, gray);

                y += 4;
                offset += 2;
            }
        }
    }
    EndMode2D();
    EndTextureMode();

    return renderTexture;
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
    self->backgroundLayer = LoadRenderTexture(CTX_VIEWPORT_WIDTH * zoom, CTX_VIEWPORT_HEIGHT * zoom);
    self->targetLayer = LoadRenderTexture(CTX_VIEWPORT_WIDTH * zoom, CTX_VIEWPORT_HEIGHT * zoom);
    self->foregroundLayer = LoadRenderTexture(CTX_VIEWPORT_WIDTH, CTX_VIEWPORT_HEIGHT);
    self->debugLayer = LoadRenderTexture(CTX_VIEWPORT_WIDTH * zoom, CTX_VIEWPORT_HEIGHT * zoom);

    self->treeTexture = GenerateTreeTexture();
}

// TODO(thismarvin): This is a proof-of-concept...
static void PopulateLevel(Scene* scene)
{
    static const usize length = 1;

    for (usize i = 0; i < length; ++i)
    {
        scene->level.segments[i].type = GetRandomValue(0, 3);
    }

    scene->level.segmentsLength = length;

    Vector2 offset = VECTOR2_ZERO;
    scene->bounds = (Rectangle)
    {
        .x = 0,
        .y = 0,
        .width = 0,
        .height = 180,
    };

    scene->player = SceneDeferAddEntity(scene, PlayerCreate(16 * 1, 16 * -4));
    scene->fog = SceneDeferAddEntity(scene, FogCreate());

    for (usize i = 0; i < scene->level.segmentsLength; ++i)
    {
        LevelSegment* segment = &scene->level.segments[i];
        LevelSegmentBuilder segmentBuilder = LevelSegmentBuilderCreate(segment->type, offset);

        for (usize i = 0; i < DequeGetSize(&segmentBuilder.entities); ++i)
        {
            const EntityBuilder builder = DEQUE_GET_UNCHECKED(&segmentBuilder.entities, EntityBuilder, i);
            SceneDeferAddEntity(scene, builder);
        }

        segment->width = segmentBuilder.width;
        offset.x += segmentBuilder.width;
        scene->bounds.width += segmentBuilder.width;

        DequeDestroy(&segmentBuilder.entities);
    }
}

static void PlantTrees(Scene* scene)
{
    static const i32 spacing = 80;

    DequeClear(&scene->treePositionsBack);
    DequeClear(&scene->treePositionsFront);

    const f32 domain = spacing + scene->bounds.width + spacing;
    const usize total = ceilf(domain / spacing);

    for (usize i = 0; i < total; ++i)
    {
        const f32 x = -spacing + spacing * i + -48 + GetRandomValue(0, 6) * 16;
        const f32 y = 8 + GetRandomValue(0, 4) * 16;
        const Vector2 position = Vector2Create(x, y);
        DEQUE_PUSH_BACK(&scene->treePositionsBack, Vector2, position);
    }

    for (usize i = 0; i < total; ++i)
    {
        const f32 x = -spacing + spacing * i + -32 + GetRandomValue(0, 4) * 16;
        const f32 y = 8 + 8 + GetRandomValue(0, 4) * 16;
        const Vector2 position = Vector2Create(x, y);
        DEQUE_PUSH_BACK(&scene->treePositionsFront, Vector2, position);
    }
}

static void SceneStart(Scene* self)
{
    memset(&self->components.tags, 0, sizeof(u64) * MAX_ENTITIES);

    DequeClear(&self->commands);

    // Initialize EntityManager.
    {
        self->m_entityManager.m_nextFreshEntityIndex = 0;
        DequeClear(&self->m_entityManager.m_recycledEntityIndices);
    }

    PopulateLevel(self);
    PlantTrees(self);
}

static void SceneReset(Scene* self)
{
    SceneStart(self);

    self->resetRequested = false;
}

void SceneInit(Scene* self)
{
    SceneSetupContent(self);
    SceneSetupInput(self);
    SceneSetupLayers(self);

    self->debugging = false;

    self->commands = DEQUE_OF(Command);
    self->m_entityManager.m_recycledEntityIndices = DEQUE_WITH_CAPACITY(usize, MAX_ENTITIES);

    self->treePositionsBack = DEQUE_OF(Vector2);
    self->treePositionsFront = DEQUE_OF(Vector2);

    SceneStart(self);
    SceneExecuteCommands(self);
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

    for (usize i = 0; i < SceneGetEntityCount(self); ++i)
    {
        SFleetingUpdate(self, i);

        SSmoothUpdate(self, i);

        PlayerInputUpdate(self, i);

        SKineticUpdate(self, i);

        SCollisionUpdate(self, i);
        SPostCollisionUpdate(self, i);

        PlayerPostCollisionUpdate(self, i);
        PlayerMortalUpdate(self, i);

        FogUpdate(self, i);
        BatteryUpdate(self, i);

        SAnimationUpdate(self, i);

        PlayerAnimationUpdate(self, i);
    }

    SceneCheckEndCondition(self);

    if (self->resetRequested)
    {
        SceneReset(self);
    }

    SceneExecuteCommands(self);
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
        const f32 min = RectangleLeft(self->bounds) + CTX_VIEWPORT_WIDTH * 0.5;
        const f32 max = RectangleRight(self->bounds) - CTX_VIEWPORT_WIDTH * 0.5;

        cameraPosition.x = MAX(min, cameraPosition.x);
        cameraPosition.x = MIN(max, cameraPosition.x);
    }

    // Camera y-axis collision.
    {
        const f32 min = RectangleTop(self->bounds) + CTX_VIEWPORT_HEIGHT * 0.5;
        const f32 max = RectangleBottom(self->bounds) - CTX_VIEWPORT_HEIGHT * 0.5;

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

    const Rectangle screenResolution = (Rectangle)
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

    const i32 width = CTX_VIEWPORT_WIDTH * zoom;
    const i32 height = CTX_VIEWPORT_HEIGHT * zoom;

    const Rectangle destination = (Rectangle)
    {
        .x = floor(screenResolution.width * 0.5),
        .y = floor(screenResolution.height * 0.5),
        .width = width,
        .height = height,
    };

    const Vector2 origin = (Vector2)
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

    // Draw debug layer.
    if (self->debugging)
    {
        Rectangle source = RectangleFromRenderTexture(self->debugLayer);
        source.height *= -1;

        DrawTexturePro(self->debugLayer.texture, source, destination, origin, 0, WHITE);
    }

    EndDrawing();
}

static void RenderRootLayer(UNUSED const RenderFnParams* params)
{
    ClearBackground(P8_BLUE);
}

static void DrawTree
(
    const RenderFnParams* params,
    const Vector2 position,
    const f32 scrollFactor
)
{
    const RenderTexture* renderTexture = &params->scene->treeTexture;

    const f32 domain = params->scene->bounds.width - CTX_VIEWPORT_WIDTH;
    const f32 progress = params->scene->actionCameraPosition.x / domain;
    const f32 partialDomain = domain * scrollFactor;
    const f32 offset = partialDomain * progress;

    const f32 x = -renderTexture->texture.width * 0.5 + position.x - offset;
    const f32 y = position.y;

    const Rectangle destination = (Rectangle)
    {
        .x = x,
        .y = y,
        .width = renderTexture->texture.width,
        .height = renderTexture->texture.height,
    };

    Rectangle source = RectangleFromRenderTexture(*renderTexture);
    source.height *= -1;

    DrawTexturePro(renderTexture->texture, source, destination, VECTOR2_ZERO, 0, COLOR_WHITE);
}

static void DrawTreeLayer
(
    const RenderFnParams* params,
    const Deque* treePositions,
    const f32 scrollFactor
)
{
    for (usize i = 0; i < DequeGetSize(treePositions); ++i)
    {
        const Vector2 position = DEQUE_GET_UNCHECKED(treePositions, Vector2, i);
        DrawTree(params, position, scrollFactor);
    }
}

static void RenderBackgroundLayer(const RenderFnParams* params)
{
    ClearBackground(COLOR_TRANSPARENT);

    DrawTreeLayer(params, &params->scene->treePositionsBack, 0.15);
    DrawTreeLayer(params, &params->scene->treePositionsFront, 0.2);
}

static void RenderTargetLayer(const RenderFnParams* params)
{
    ClearBackground(COLOR_TRANSPARENT);

    // Draw Level.
    {
        Vector2 offset = VECTOR2_ZERO;

        for (usize i = 0; i < params->scene->level.segmentsLength; ++i)
        {
            const LevelSegment* segment = &params->scene->level.segments[i];
            LevelSegmentDraw(segment, &params->scene->atlas, offset);
            offset.x += segment->width;
        }
    }

    for (usize i = 0; i < SceneGetEntityCount(params->scene); ++i)
    {
        SSpriteDraw(params->scene, i);
        SAnimationDraw(params->scene, i);
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
        FogParticleDraw(params->scene, i);
    }

    for (usize i = 0; i < SceneGetEntityCount(params->scene); ++i)
    {
        FogDraw(params->scene, i);
    }
}

static void RenderDebugLayer(const RenderFnParams* params)
{
    if (!params->scene->debugging)
    {
        return;
    }

    ClearBackground(COLOR_TRANSPARENT);

    for (usize i = 0; i < SceneGetEntityCount(params->scene); ++i)
    {
        SDebugDraw(params->scene, i);
    }
}

void SceneDraw(Scene* self)
{
    Rectangle actionCameraBounds = SceneCalculateActionCameraBounds(self, self->player);

    self->actionCameraPosition = (Vector2)
    {
        .x = actionCameraBounds.x,
        .y = actionCameraBounds.y,
    };

    const RenderFnParams actionCameraParams = (RenderFnParams)
    {
        .scene = (Scene*)self,
        .cameraBounds = actionCameraBounds,
    };
    const RenderFnParams stationaryCameraParams = (RenderFnParams)
    {
        .scene = (Scene*)self,
        .cameraBounds = (Rectangle) { 0, 0, CTX_VIEWPORT_WIDTH, CTX_VIEWPORT_HEIGHT },
    };

    SceneRenderLayer(&self->rootLayer, RenderRootLayer, &actionCameraParams);
    SceneRenderLayer(&self->backgroundLayer, RenderBackgroundLayer, &stationaryCameraParams);
    SceneRenderLayer(&self->targetLayer, RenderTargetLayer, &actionCameraParams);
    SceneRenderLayer(&self->foregroundLayer, RenderForegroundLayer, &actionCameraParams);
    SceneRenderLayer(&self->debugLayer, RenderDebugLayer, &actionCameraParams);

    SceneDrawLayers(self);
}

void SceneDeferReset(Scene* self)
{
    self->resetRequested = true;
}

void SceneDestroy(Scene* self)
{
    AtlasDestroy(&self->atlas);

    DequeDestroy(&self->commands);
    DequeDestroy(&self->m_entityManager.m_recycledEntityIndices);
    DequeDestroy(&self->treePositionsBack);
    DequeDestroy(&self->treePositionsFront);

    UnloadRenderTexture(self->treeTexture);
    UnloadRenderTexture(self->backgroundLayer);
    UnloadRenderTexture(self->targetLayer);
    UnloadRenderTexture(self->foregroundLayer);
    UnloadRenderTexture(self->debugLayer);

    InputProfileDestroy(&self->defaultActionProfile);
}
