#include "./ecs/entities.h"
#include "./ecs/systems.h"
#include "./palette/p8.h"
#include "context.h"
#include "scene.h"
#include "scene_generated.h"
#include <assert.h>
#include <raymath.h>
#include <stdio.h>
#include <string.h>

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

        SceneSubmitCommand(self, CommandCreateSetComponent(entity, *component));
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

static void SceneSetupActionProfile(Scene* self)
{
    self->defaultActionProfile = InputProfileCreate(3);

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
}

static void SceneSetupMenuProfile(Scene* self)
{
    self->defaultMenuProfile = InputProfileCreate(1);

    // Keyboard.
    {
        {
            KeyboardBinding binding = KeyboardBindingCreate("select", 2);

            KeyboardBindingAddKey(&binding, KEY_SPACE);
            KeyboardBindingAddKey(&binding, KEY_ENTER);

            InputProfileAddKeyboardBinding(&self->defaultMenuProfile, binding);
        }
    }

    // Gamepad.
    {
        // Buttons.
        {
            {
                GamepadBinding binding = GamepadBindingCreate("select", 2);

                GamepadBindingAddButton(&binding, GAMEPAD_BUTTON_MIDDLE_LEFT);
                GamepadBindingAddButton(&binding, GAMEPAD_BUTTON_MIDDLE_RIGHT);

                InputProfileAddGamepadBinding(&self->defaultMenuProfile, binding);
            }
        }
    }
}

static void SceneSetupInput(Scene* self)
{
    self->input = InputHandlerCreate(0);

    SceneSetupMenuProfile(self);
    SceneSetupActionProfile(self);

    InputHandlerSetProfile(&self->input, &self->defaultMenuProfile);
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
    // TODO(thismarvin): Expose a "Render Resolution" option?

    // Use the monitor's resolution as the default render resolution.
    Rectangle renderResolution = GetMonitorResolution();

    f32 zoom = CalculateZoom(CTX_VIEWPORT, renderResolution);

    // Ensure that the render resolution uses integer scaling.
    zoom = floor(zoom);

    self->rootLayer = LoadRenderTexture(1, 1);
    self->backgroundLayer = LoadRenderTexture(CTX_VIEWPORT_WIDTH * zoom, CTX_VIEWPORT_HEIGHT * zoom);
    self->targetLayer = LoadRenderTexture(CTX_VIEWPORT_WIDTH * zoom, CTX_VIEWPORT_HEIGHT * zoom);
    self->foregroundLayer = LoadRenderTexture(CTX_VIEWPORT_WIDTH, CTX_VIEWPORT_HEIGHT);
    self->interfaceLayer = LoadRenderTexture(CTX_VIEWPORT_WIDTH * zoom, CTX_VIEWPORT_HEIGHT * zoom);
    // TODO(thismarvin): If this is just for fades then this resolution is overkill!
    self->transitionLayer = LoadRenderTexture(CTX_VIEWPORT_WIDTH * zoom, CTX_VIEWPORT_HEIGHT * zoom);
    self->debugLayer = LoadRenderTexture(CTX_VIEWPORT_WIDTH * zoom, CTX_VIEWPORT_HEIGHT * zoom);

    self->treeTexture = GenerateTreeTexture();
}

static void ArrayFillWithRange(i32* array, const i32 start, const i32 end)
{
    const usize domain = end - start;

    for (usize i = 0; i < domain; ++i)
    {
        array[i] = start + i;
    }
}

static void ArrayShuffle(i32* array, i32* candidates, usize arrayLength)
{
    for (usize i = 0; i < arrayLength; ++i)
    {
        const usize end = arrayLength - 1 - i;

        const usize index = GetRandomValue(0, end);

        array[i] = candidates[index];

        memcpy(candidates + index, candidates + index + 1, sizeof(i32) * (end - index));
    }
}

static void ScenePopulateLevel(Scene* self)
{
    static const u16 totalStarters = TOTAL_STARTER_SEGMENTS;
    static const u16 totalFillers = TOTAL_FILLER_SEGMENTS;
    static const u16 totalBatteries = TOTAL_BATTERY_SEGMENTS;
    static const u16 totalSolars = TOTAL_SOLAR_SEGMENTS;

    static const u16 starterBegin = totalBatteries + totalFillers + totalSolars;
    static const u16 fillerBegin = totalBatteries;
    static const u16 batteryBegin = 0;
    static const u16 solarBegin = totalBatteries + totalFillers;

    i32 fillerCandidates[totalFillers];
    {
        i32 candidates[totalFillers];
        ArrayFillWithRange(candidates, 0, totalFillers);

        ArrayShuffle(fillerCandidates, candidates, totalFillers);
    }

    i32 batteryCandidates[totalBatteries];
    {
        i32 candidates[totalBatteries];
        ArrayFillWithRange(candidates, 0, totalBatteries);

        ArrayShuffle(batteryCandidates, candidates, totalBatteries);
    }

    const u16 starter = GetRandomValue(0, totalStarters - 1);
    self->level.segments[0].type = starterBegin + starter;

    for (usize i = 0; i < 3; ++i)
    {
        const u16 filler = fillerCandidates[i];
        const u16 battery = batteryCandidates[i];
        self->level.segments[(i * 2) + 1].type = fillerBegin + filler;
        self->level.segments[(i * 2) + 2].type = batteryBegin + battery;
    }

    const u16 solar = GetRandomValue(0, totalSolars - 1);
    self->level.segments[7].type = solarBegin + solar;

    self->level.segmentsLength = MAX_LEVEL_SEGMENTS;

    Vector2 offset = VECTOR2_ZERO;
    self->bounds = (Rectangle)
    {
        .x = 0,
        .y = 0,
        .width = 0,
        .height = 180,
    };

    for (usize i = 0; i < self->level.segmentsLength; ++i)
    {
        LevelSegment* segment = &self->level.segments[i];
        LevelSegmentBuilder segmentBuilder = LevelSegmentBuilderCreate(segment->type, offset);

        for (usize i = 0; i < DequeGetSize(&segmentBuilder.entities); ++i)
        {
            const EntityBuilder builder = DEQUE_GET_UNCHECKED(&segmentBuilder.entities, EntityBuilder, i);
            SceneDeferAddEntity(self, builder);
        }

        segment->width = segmentBuilder.width;
        offset.x += segmentBuilder.width;
        self->bounds.width += segmentBuilder.width;

        DequeDestroy(&segmentBuilder.entities);
    }

    // Extend the end of the level so the player doesn't visibly fall out-of-bounds.
    {
        const Rectangle aabb = (Rectangle)
        {
            .x = self->bounds.width,
            .y = 16 * 10,
            .width = 16 * 5,
            .height = 16 * (2 + 4),
        };
        SceneDeferAddEntity(self, BlockCreate(aabb, RESOLVE_ALL, LAYER_TERRAIN));
    }

    self->player = SceneDeferAddEntity(self, PlayerCreate(16 * 1, 16 * -4));
    self->fog = SceneDeferAddEntity(self, FogCreate());
    self->lakitu = SceneDeferAddEntity(self, LakituCreate());
}

static void ScenePlantTrees(Scene* self)
{
    static const i32 spacing = 80;

    DequeClear(&self->treePositionsBack);
    DequeClear(&self->treePositionsFront);

    const f32 domain = spacing + self->bounds.width + spacing;
    const usize total = ceilf(domain / spacing);

    for (usize i = 0; i < total; ++i)
    {
        const f32 x = -spacing + spacing * i + -48 + GetRandomValue(0, 6) * 16;
        const f32 y = 8 + GetRandomValue(0, 4) * 16;
        const Vector2 position = Vector2Create(x, y);
        DEQUE_PUSH_BACK(&self->treePositionsBack, Vector2, position);
    }

    for (usize i = 0; i < total; ++i)
    {
        const f32 x = -spacing + spacing * i + -32 + GetRandomValue(0, 4) * 16;
        const f32 y = 8 + 8 + GetRandomValue(0, 4) * 16;
        const Vector2 position = Vector2Create(x, y);
        DEQUE_PUSH_BACK(&self->treePositionsFront, Vector2, position);
    }
}

static void SceneBeginFadeIn(Scene* self)
{
    self->director = DIRECTOR_STATE_ENTRANCE;

    FaderReset(&self->fader);
    self->fader.type = FADE_IN;
    self->fader.easer.duration = CTX_DT * 20;
}

static void SceneBeginFadeOut(Scene* self)
{
    self->director = DIRECTOR_STATE_EXIT;

    FaderReset(&self->fader);
    self->fader.type = FADE_OUT;
    self->fader.easer.duration = CTX_DT * 40;
}

static void SceneBuildStage(Scene* self)
{
    SceneBeginFadeIn(self);

    memset(&self->components.tags, 0, sizeof(u64) * MAX_ENTITIES);

    DequeClear(&self->commands);

    self->m_entityManager.m_nextFreshEntityIndex = 0;
    DequeClear(&self->m_entityManager.m_recycledEntityIndices);

    ScenePopulateLevel(self);
    ScenePlantTrees(self);

    self->resetRequested = false;
    self->advanceStageRequested = false;

    SceneExecuteCommands(self);
}

static void SceneReset(Scene* self)
{
    self->stage = 0;

    self->score = 0;
    memset(&self->scoreString, '0', sizeof(char) * MAX_SCORE_DIGITS);

    self->scoreBufferTimerDuration = CTX_DT * 2;
    self->scoreBufferTimer = 0;
    self->scoreBuffer = 0;

    SceneBuildStage(self);
}

static void SceneAdvanceStage(Scene* self)
{
    self->stage += 1;
    self->stage = MIN(self->stage, 64);

    const CMortal playersMortal = self->components.mortals[self->player];

    SceneBuildStage(self);

    // TODO(thismarvin): Is there a better way to preserve the player's health?
    self->components.mortals[self->player] = playersMortal;
}

void SceneInit(Scene* self)
{
    SceneSetupContent(self);
    SceneSetupInput(self);
    SceneSetupLayers(self);

    self->state = SCENE_STATE_MENU;

    self->debugging = false;

    self->commands = DEQUE_OF(Command);
    self->m_entityManager = (EntityManager)
    {
        .m_nextFreshEntityIndex = 0,
        .m_recycledEntityIndices = DEQUE_WITH_CAPACITY(usize, MAX_ENTITIES),
    };

    self->treePositionsBack = DEQUE_OF(Vector2);
    self->treePositionsFront = DEQUE_OF(Vector2);

    self->director = DIRECTOR_STATE_ENTRANCE;
    self->fader = FaderDefault();
    self->fader.easer.ease = EaseInOutQuad;

    SceneReset(self);
}

void SceneIncrementScore(Scene* self, const u32 value)
{
    self->scoreBuffer += value;
    self->scoreBuffer = MIN(self->scoreBuffer, MAX_SCORE);
}

void SceneDeferReset(Scene* self)
{
    self->resetRequested = true;
}

void SceneDeferAdvanceStage(Scene* self)
{
    self->advanceStageRequested = true;
}

static void SceneUpdateScore(Scene* self)
{
    if (self->scoreBuffer == 0)
    {
        return;
    }

    self->scoreBufferTimer += CTX_DT;

    if (self->scoreBufferTimer >= self->scoreBufferTimerDuration)
    {
        const i32 take = ceilf(self->scoreBuffer * 0.1);

        self->score += take;
        self->score = MIN(self->score, MAX_SCORE);

        self->scoreBuffer -= take;
        self->scoreBuffer = MAX(self->scoreBuffer, 0);

        // TODO(thismarvin): Is this expensive? Should we evaluate this lazily?
        snprintf((char*)self->scoreString, MAX_SCORE_DIGITS, "%06d", self->score);

        self->scoreBufferTimer = 0;
    }
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

static void SceneMenuUpdate(Scene* self)
{
    if (InputHandlerPressed(&self->input, "select"))
    {
        InputHandlerConsume(&self->input, "select");

        // TODO(thismarvin): Defer this somehow...
        self->state = SCENE_STATE_ACTION;
        InputHandlerSetProfile(&self->input, &self->defaultActionProfile);

        // TODO(thismarvin): There should be a bespoke transition into the Action state.
        self->fader.easer.duration = CTX_DT * 40;
    }

    // TODO(thismarvin): The following is very hacky! Also, maybe put this in LakituUpdate?
    {
        CPosition* position = &self->components.positions[self->lakitu];

        SSmoothUpdate(self, self->lakitu);
        SKineticUpdate(self, self->lakitu);

        if (position->value.x > self->bounds.width - CTX_VIEWPORT_WIDTH * 0.5)
        {
            position->value.x = CTX_VIEWPORT_WIDTH * 0.5;
            self->components.smooths[self->lakitu].previous = position->value;
        }
    }
}

static void SceneUpdateDirectorAction(Scene* self)
{
    switch (self->director)
    {
        case DIRECTOR_STATE_ENTRANCE:
        {
            FaderUpdate(&self->fader);

            if (FaderIsDone(&self->fader))
            {
                self->director = DIRECTOR_STATE_SUPERVISE;
            }

            break;
        }

        case DIRECTOR_STATE_SUPERVISE:
        {
            if (self->resetRequested || self->advanceStageRequested)
            {
                SceneBeginFadeOut(self);
            }

            break;
        }

        case DIRECTOR_STATE_EXIT:
        {
            FaderUpdate(&self->fader);

            if (FaderIsDone(&self->fader))
            {
                if (self->advanceStageRequested)
                {
                    SceneAdvanceStage(self);
                }
                else if (self->resetRequested)
                {
                    SceneReset(self);
                }
            }

            break;
        }
    }
}

static void SceneUpdateDirector(Scene* self)
{
    switch (self->state)
    {
        case SCENE_STATE_MENU:
        {
            break;
        }

        case SCENE_STATE_ACTION:
        {
            SceneUpdateDirectorAction(self);
            break;
        }
    }
}

static void SceneActionUpdate(Scene* self)
{
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

    SceneUpdateScore(self);
    SceneCheckEndCondition(self);
}

void SceneUpdate(Scene* self)
{
    InputHandlerUpdate(&self->input);

    if (IsKeyPressed(KEY_EQUAL))
    {
        self->debugging = !self->debugging;
    }

    switch (self->state)
    {
        case SCENE_STATE_MENU:
        {
            SceneMenuUpdate(self);
            break;
        }

        case SCENE_STATE_ACTION:
        {
            SceneActionUpdate(self);
            break;
        }
    }

    SceneUpdateDirector(self);

    SceneExecuteCommands(self);
}

// Return a Rectangle that is within the scene's bounds and centered on a given entity.
static Rectangle SceneCalculateActionCameraBounds(const Scene* self, const usize targetEntity)
{
    if ((self->components.tags[targetEntity] & (TAG_POSITION)) != (TAG_POSITION))
    {
        return CTX_VIEWPORT;
    }

    Vector2 cameraPosition = self->components.positions[targetEntity].value;

    if ((self->components.tags[targetEntity] & (TAG_SMOOTH)) == (TAG_SMOOTH))
    {
        const CSmooth* smooth = &self->components.smooths[targetEntity];

        cameraPosition = Vector2Lerp(smooth->previous, cameraPosition, ContextGetAlpha());
    }

    if ((self->components.tags[targetEntity] & (TAG_DIMENSION)) == (TAG_DIMENSION))
    {
        const CDimension* dimension = &self->components.dimensions[targetEntity];

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

static void SceneDrawScore(const Scene* self, const Vector2 position)
{
    for (usize i = 0; i < MAX_SCORE_DIGITS - 1; ++i)
    {
        const i32 index = (MAX_SCORE_DIGITS - 2) - i;

        const Sprite digit = SPRITE_NUMBERS_0000 + (self->scoreString[index] - '0');

        const AtlasDrawParams params = (AtlasDrawParams)
        {
            .sprite = digit,
            .position = (Vector2) { position.x + index * 14, position.y },
            .intramural = (Rectangle) { 0, 0, 0, 0 },
            .reflection = REFLECTION_NONE,
            .tint = COLOR_WHITE,
        };
        AtlasDraw(&self->atlas, &params);
    }
}

static void SceneDrawHealthBar(const Scene* self, const Vector2 position)
{
    static const usize totalHearts = PLAYER_MAX_HIT_POINTS;

    const Rectangle intramural = (Rectangle)
    {
        .x = 8,
        .y = 7,
        .width = 19,
        .height = 17,
    };

    // Draw heart capsules.
    for (usize i = 0; i < totalHearts; ++i)
    {
        const Vector2 myPosition = (Vector2)
        {
            .x = position.x + i * 15,
            .y = position.y,
        };

        const AtlasDrawParams params = (AtlasDrawParams)
        {
            .sprite = SPRITE_HEART_0000,
            .position = myPosition,
            .intramural = intramural,
            .reflection = REFLECTION_NONE,
            .tint = COLOR_WHITE,
        };
        AtlasDraw(&self->atlas, &params);
    }

    // Draw heart containers.
    {
        const usize hp = self->components.mortals[self->player].hp;
        const usize hearts = totalHearts - hp;

        for (usize i = hearts; i < totalHearts; ++i)
        {
            const Vector2 myPosition = (Vector2)
            {
                .x = position.x + i * 15,
                .y = position.y
            };

            const AtlasDrawParams params = (AtlasDrawParams)
            {
                .sprite = SPRITE_HEART_0001,
                .position = myPosition,
                .intramural = intramural,
                .reflection = REFLECTION_NONE,
                .tint = COLOR_WHITE,
            };
            AtlasDraw(&self->atlas, &params);
        }
    }
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
    const Scene* scene = (Scene*)params->scene;

    const RenderTexture* renderTexture = &scene->treeTexture;

    const f32 domain = scene->bounds.width - CTX_VIEWPORT_WIDTH;
    const f32 progress = scene->actionCameraPosition.x / domain;
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

    Rectangle source = RectangleFromRenderTexture(renderTexture);
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
    const Scene* scene = (Scene*)params->scene;

    ClearBackground(COLOR_TRANSPARENT);

    DrawTreeLayer(params, &scene->treePositionsBack, 0.15);
    DrawTreeLayer(params, &scene->treePositionsFront, 0.2);
}

static void RenderTargetLayer(const RenderFnParams* params)
{
    const Scene* scene = (Scene*)params->scene;

    ClearBackground(COLOR_TRANSPARENT);

    // Draw Level.
    {
        Vector2 offset = VECTOR2_ZERO;

        for (usize i = 0; i < scene->level.segmentsLength; ++i)
        {
            const LevelSegment* segment = &scene->level.segments[i];
            LevelSegmentDraw(segment, &scene->atlas, offset);
            offset.x += segment->width;
        }
    }

    for (usize i = 0; i < SceneGetEntityCount(scene); ++i)
    {
        SSpriteDraw(scene, i);
        SAnimationDraw(scene, i);
    }
}

static void RenderMenuInterface(const RenderFnParams* params)
{
    const Scene* scene = (Scene*)params->scene;

    const Color transparentBlack = (Color)
    {
        .r = 0,
        .g = 0,
        .b = 0,
        .a = 100
    };

    ClearBackground(transparentBlack);

    // Draw logo.
    {
        const AtlasDrawParams drawParams = (AtlasDrawParams)
        {
            .sprite = SPRITE_LOGO,
            .position = (Vector2) { 32, (180.0 - 112) / 2 },
            .intramural = (Rectangle) { 0, 0, 0, 0 },
            .reflection = REFLECTION_NONE,
            .tint = COLOR_WHITE,
        };
        AtlasDraw(&scene->atlas, &drawParams);
    }
}

static void RenderActionInterface(const RenderFnParams* params)
{
    static const usize healthBarWidth = 15 * PLAYER_MAX_HIT_POINTS + 4;
    static const usize padding = 4;

    const Scene* scene = (Scene*)params->scene;

    ClearBackground(COLOR_TRANSPARENT);

    SceneDrawScore(scene, Vector2Create(padding, padding));
    SceneDrawHealthBar(scene, Vector2Create(CTX_VIEWPORT_WIDTH - padding - healthBarWidth, padding));
}

static void RenderInterfaceLayer(const RenderFnParams* params)
{
    const Scene* scene = (Scene*)params->scene;

    switch (scene->state)
    {
        case SCENE_STATE_MENU:
        {
            RenderMenuInterface(params);
            break;
        }

        case SCENE_STATE_ACTION:
        {
            RenderActionInterface(params);
            break;
        }
    }
}

static void RenderTransitionLayer(const RenderFnParams* params)
{
    const Scene* scene = (Scene*)params->scene;

    ClearBackground(COLOR_TRANSPARENT);

    if (scene->director == DIRECTOR_STATE_SUPERVISE)
    {
        return;
    }

    FaderDraw(&scene->fader);
}

static void RenderForegroundLayer(const RenderFnParams* params)
{
    const Scene* scene = (Scene*)params->scene;

    ClearBackground(COLOR_TRANSPARENT);

    for (usize i = 0; i < SceneGetEntityCount(scene); ++i)
    {
        CloudParticleDraw(scene, i);
    }

    for (usize i = 0; i < SceneGetEntityCount(scene); ++i)
    {
        FogParticleDraw(scene, i);
    }

    for (usize i = 0; i < SceneGetEntityCount(scene); ++i)
    {
        FogDraw(scene, i);
    }
}

static void RenderDebugLayer(const RenderFnParams* params)
{
    const Scene* scene = (Scene*)params->scene;

    ClearBackground(COLOR_TRANSPARENT);

    if (!scene->debugging)
    {
        return;
    }

    for (usize i = 0; i < SceneGetEntityCount(scene); ++i)
    {
        SDebugColliderDraw(scene, i);
    }
}

static void SceneMenuDraw(Scene* self)
{
    Rectangle actionCameraBounds = SceneCalculateActionCameraBounds(self, self->lakitu);

    self->actionCameraPosition = (Vector2)
    {
        .x = actionCameraBounds.x,
        .y = actionCameraBounds.y,
    };

    const RenderFnParams actionCameraParams = (RenderFnParams)
    {
        .scene = self,
        .cameraBounds = actionCameraBounds,
    };
    const RenderFnParams stationaryCameraParams = (RenderFnParams)
    {
        .scene = self,
        .cameraBounds = CTX_VIEWPORT,
    };

    RenderLayer(&self->rootLayer, RenderRootLayer, &stationaryCameraParams);
    RenderLayer(&self->backgroundLayer, RenderBackgroundLayer, &stationaryCameraParams);
    RenderLayer(&self->targetLayer, RenderTargetLayer, &actionCameraParams);
    RenderLayer(&self->interfaceLayer, RenderInterfaceLayer, &stationaryCameraParams);

    const RenderTexture renderTextures[4] =
    {
        self->rootLayer,
        self->backgroundLayer,
        self->targetLayer,
        self->interfaceLayer,
    };
    DrawLayers(renderTextures, 4);
}

static void SceneActionDraw(Scene* self)
{
    Rectangle actionCameraBounds = SceneCalculateActionCameraBounds(self, self->player);

    self->actionCameraPosition = (Vector2)
    {
        .x = actionCameraBounds.x,
        .y = actionCameraBounds.y,
    };

    const RenderFnParams actionCameraParams = (RenderFnParams)
    {
        .scene = self,
        .cameraBounds = actionCameraBounds,
    };
    const RenderFnParams stationaryCameraParams = (RenderFnParams)
    {
        .scene = self,
        .cameraBounds = CTX_VIEWPORT,
    };

    RenderLayer(&self->rootLayer, RenderRootLayer, &stationaryCameraParams);
    RenderLayer(&self->backgroundLayer, RenderBackgroundLayer, &stationaryCameraParams);
    RenderLayer(&self->targetLayer, RenderTargetLayer, &actionCameraParams);
    RenderLayer(&self->foregroundLayer, RenderForegroundLayer, &actionCameraParams);
    RenderLayer(&self->interfaceLayer, RenderInterfaceLayer, &stationaryCameraParams);
    RenderLayer(&self->transitionLayer, RenderTransitionLayer, &stationaryCameraParams);
    RenderLayer(&self->debugLayer, RenderDebugLayer, &actionCameraParams);

    const RenderTexture renderTextures[7] =
    {
        self->rootLayer,
        self->backgroundLayer,
        self->targetLayer,
        self->foregroundLayer,
        self->interfaceLayer,
        self->transitionLayer,
        self->debugLayer,
    };
    DrawLayers(renderTextures, 7);
}

void SceneDraw(Scene* self)
{
    switch (self->state)
    {
        case SCENE_STATE_MENU:
        {
            SceneMenuDraw(self);
            break;
        }

        case SCENE_STATE_ACTION:
        {
            SceneActionDraw(self);
            break;
        }
    }
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
    UnloadRenderTexture(self->interfaceLayer);
    UnloadRenderTexture(self->transitionLayer);
    UnloadRenderTexture(self->debugLayer);

    InputProfileDestroy(&self->defaultMenuProfile);
    InputProfileDestroy(&self->defaultActionProfile);
}
