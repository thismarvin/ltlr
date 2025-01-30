#include "scene.h"

#include "./collections/deque.h"
#include "./ecs/components.h"
#include "./ecs/entities/battery.h"
#include "./ecs/entities/block.h"
#include "./ecs/entities/cloud_particle.h"
#include "./ecs/entities/fog.h"
#include "./ecs/entities/fog_particle.h"
#include "./ecs/entities/lakitu.h"
#include "./ecs/entities/player.h"
#include "./ecs/systems.h"
#include "./palette/p8.h"
#include "./utils/arena_allocator.h"
#include "atlas.h"
#include "bit_mask.h"
#include "common.h"
#include "context.h"
#include "easing.h"
#include "fader.h"
#include "game.h"
#include "input.h"
#include "level.h"
#include "replay.h"
#include "rng.h"
#include "scene_generated.h"
#include "shaders.h"
#include "sprites_generated.h"

#include <assert.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#if defined(NDEBUG)
	#include <time.h>
#endif

#if defined(PLATFORM_WEB)
	#include <emscripten/emscripten.h>
#endif

// Record the last 30 minutes of input!
#define RECORDING_SIZE ((usize)1 * 60 * 60 * 30)

#define RUN_SYSTEM(mSystemFn, mScene, mEntities) \
	do \
	{ \
		for (usize i = 0; i < (mEntities); ++i) \
		{ \
			mSystemFn(mScene, i); \
		} \
	} while (false)

typedef struct
{
	OnDefer fn;
	void* params;
} SceneDeferParams;

typedef struct
{
	usize entity;
} DeallocateEntityParams;

typedef struct
{
	usize entity;
	u64 tag;
} EnableComponentParams;

typedef struct
{
	usize entity;
	u64 tag;
} DisableComponentParams;

typedef struct
{
	usize entity;
	u64 tag;
} SetTagParams;

usize SceneAllocateEntity(Scene* self)
{
	EntityManager* entityManager = &self->m_entityManager;

	if (DequeGetSize(&entityManager->m_recycledEntityIndices) != 0)
	{
		return DEQUE_POP_FRONT(&entityManager->m_recycledEntityIndices, usize);
	}

	// No used indices, use next available fresh one.
	const usize next = MIN(entityManager->m_nextFreshEntityIndex, MAX_ENTITIES - 1);

	entityManager->m_nextFreshEntityIndex = entityManager->m_nextFreshEntityIndex + 1;
	entityManager->m_nextFreshEntityIndex =
		MIN(entityManager->m_nextFreshEntityIndex, MAX_ENTITIES);

	if (entityManager->m_nextFreshEntityIndex == MAX_ENTITIES)
	{
		TraceLog(LOG_WARNING, "Maximum amount of entities reached.");
	}

	return next;
}

static void SceneDeallocateEntity(Scene* self, const void* params)
{
	const DeallocateEntityParams* data = params;

	self->components.tags[data->entity] = TAG_NONE;
	DequePushFront(&self->m_entityManager.m_recycledEntityIndices, &data->entity);
}

static void SceneEnableComponent(Scene* self, const void* params)
{
	const EnableComponentParams* data = params;

	self->components.tags[data->entity] |= data->tag;
}

static void SceneDisableComponent(Scene* self, const void* params)
{
	const DisableComponentParams* data = params;

	self->components.tags[data->entity] &= ~data->tag;
}

static void SceneSetTag(Scene* self, const void* params)
{
	const SetTagParams* data = params;

	self->components.tags[data->entity] = data->tag;
}

usize SceneGetTotalAllocatedEntities(const Scene* self)
{
	return self->m_entityManager.m_nextFreshEntityIndex;
}

bool SceneEntityHasDependencies(const Scene* self, const usize entity, const u64 dependencies)
{
	return (self->components.tags[entity] & dependencies) == dependencies;
}

bool SceneEntityIs(const Scene* self, const usize entity, const EntityType type)
{
	return (self->components.tags[entity] & TAG_IDENTIFIER) == TAG_IDENTIFIER
		   && self->components.identifiers[entity].type == type;
}

void SceneDefer(Scene* self, const OnDefer fn, const void* params)
{
	const SceneDeferParams args = (SceneDeferParams) {
		.fn = fn,
		.params = (void*)params,
	};

	DEQUE_PUSH_FRONT(&self->deferred, SceneDeferParams, args);
}

void SceneDeferDeallocateEntity(Scene* self, const usize entity)
{
	DeallocateEntityParams* params =
		ArenaAllocatorTake(&self->arenaAllocator, sizeof(DeallocateEntityParams));
	params->entity = entity;
	SceneDefer(self, SceneDeallocateEntity, params);
}

void SceneDeferEnableTag(Scene* self, const usize entity, const u64 tag)
{
	EnableComponentParams* params =
		ArenaAllocatorTake(&self->arenaAllocator, sizeof(EnableComponentParams));
	params->entity = entity;
	params->tag = tag;
	SceneDefer(self, SceneEnableComponent, params);
}

void SceneDeferDisableTag(Scene* self, const usize entity, const u64 tag)
{
	DisableComponentParams* params =
		ArenaAllocatorTake(&self->arenaAllocator, sizeof(DisableComponentParams));
	params->entity = entity;
	params->tag = tag;
	SceneDefer(self, SceneDisableComponent, params);
}

void SceneDeferSetTag(Scene* self, const usize entity, const u64 tag)
{
	SetTagParams* params = ArenaAllocatorTake(&self->arenaAllocator, sizeof(SetTagParams));
	params->entity = entity;
	params->tag = tag;
	SceneDefer(self, SceneSetTag, params);
}

static void SceneFlush(Scene* self)
{
	for (usize i = 0; i < DequeGetSize(&self->deferred); ++i)
	{
		SceneDeferParams* params = &DEQUE_GET_UNCHECKED(&self->deferred, SceneDeferParams, i);

		params->fn(self, params->params);
	}

	ArenaAllocatorFlush(&self->arenaAllocator);
	DequeClear(&self->deferred);
}

static void SceneSetupDropShadow(Scene* self)
{
	self->dropShadow = LoadShaderFromMemory(0, shaderDropShadowSource);

	static const f32 directionValue[2] = { 1.0, -1.0 };
	static const f32 sizeValue = 2.0;
	static const f32 textureWidthValue = 320.0;
	static const f32 textureHeightValue = 180.0;
	static const f32 colorValue[4] = { 0.0, 0.0, 0.0, 0.5 };

	const i32 directionLocation = GetShaderLocation(self->dropShadow, "Direction");
	const i32 sizeLocation = GetShaderLocation(self->dropShadow, "Size");
	const i32 textureWidthLocation = GetShaderLocation(self->dropShadow, "TextureWidth");
	const i32 textureHeightLocation = GetShaderLocation(self->dropShadow, "TextureHeight");
	const i32 colorLocation = GetShaderLocation(self->dropShadow, "Color");

	SetShaderValue(self->dropShadow, directionLocation, &directionValue, SHADER_UNIFORM_VEC2);
	SetShaderValue(self->dropShadow, sizeLocation, &sizeValue, SHADER_UNIFORM_FLOAT);
	SetShaderValue(
		self->dropShadow,
		textureWidthLocation,
		&textureWidthValue,
		SHADER_UNIFORM_FLOAT
	);
	SetShaderValue(
		self->dropShadow,
		textureHeightLocation,
		&textureHeightValue,
		SHADER_UNIFORM_FLOAT
	);
	SetShaderValue(self->dropShadow, colorLocation, &colorValue, SHADER_UNIFORM_VEC4);
}

static void SceneSetupContent(Scene* self)
{
	self->atlas = AtlasCreate(DATADIR "content/atlas.png");

	SceneSetupDropShadow(self);
}

// clang-format off

static InputProfile CreateDefaultInputProfile(void)
{
	InputProfile profile = InputProfileCreate(4);

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
			KeyboardBinding binding = KeyboardBindingCreate("stomp", 2);

			KeyboardBindingAddKey(&binding, KEY_X);
			KeyboardBindingAddKey(&binding, KEY_J);

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
				GamepadBinding binding = GamepadBindingCreate("stomp", 2);

				GamepadBindingAddButton(&binding, GAMEPAD_BUTTON_RIGHT_FACE_LEFT);
				GamepadBindingAddButton(&binding, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);

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
			static const f32 threshold = 0.25F;
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

	return profile;
}

// clang-format on

static void SceneSetupInput(Scene* self)
{
	for (usize i = 0; i < MAX_PLAYERS; ++i)
	{
		self->inputs[i] = InputHandlerCreate(i);

		self->inputProfiles[i] = CreateDefaultInputProfile();

		InputHandlerSetProfile(&self->inputs[i], &self->inputProfiles[i]);
	}
}

static RenderTexture GenerateTreeTexture(void)
{
	const RenderTexture renderTexture = LoadRenderTexture(CTX_VIEWPORT_WIDTH, CTX_VIEWPORT_HEIGHT);

	const Camera2D camera = (Camera2D) {
		.zoom = 1,
		.offset = VECTOR2_ZERO,
		.target = VECTOR2_ZERO,
		.rotation = 0,
	};

	BeginTextureMode(renderTexture);
	BeginMode2D(camera);
	{
		ClearBackground(COLOR_TRANSPARENT);

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
				DrawTriangle(a, b, c, COLOR_WHITE);

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
	Rectangle renderResolution = GetMonitorRectangle();

	f32 zoom = CalculateZoom(CTX_VIEWPORT, renderResolution);

	// Ensure that the render resolution uses integer scaling.
	zoom = floor(zoom);

	const usize scaledWidth = CTX_VIEWPORT_WIDTH * zoom;
	const usize scaledHeight = CTX_VIEWPORT_HEIGHT * zoom;

	self->rootLayer = LoadRenderTexture(1, 1);
	self->backgroundLayer = LoadRenderTexture(scaledWidth, scaledHeight);
	self->foregroundLayer = LoadRenderTexture(CTX_VIEWPORT_WIDTH, CTX_VIEWPORT_HEIGHT);
	self->interfaceLayer = LoadRenderTexture(scaledWidth, scaledHeight);
	self->debugLayer = LoadRenderTexture(scaledWidth, scaledHeight);

	// TODO(thismarvin): If this is just for fades thrn this resolution is overkill!
	self->transitionLayer = LoadRenderTexture(scaledWidth, scaledHeight);

	// It's very important that these two render textures have the same dimensions!
	self->targetLayer = LoadRenderTexture(scaledWidth, scaledHeight);
	self->targetLayerBuffer = LoadRenderTexture(scaledWidth, scaledHeight);

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

static void ArrayShuffle(Rng* rng, i32* array, const usize arrayLength, i32* candidates)
{
	for (usize i = 0; i < arrayLength; ++i)
	{
		const usize end = arrayLength - 1 - i;

		const usize index = RngNextRange(rng, 0, end + 1);

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

		ArrayShuffle(&self->rng, fillerCandidates, totalFillers, candidates);
	}

	i32 batteryCandidates[totalBatteries];
	{
		i32 candidates[totalBatteries];
		ArrayFillWithRange(candidates, 0, totalBatteries);

		ArrayShuffle(&self->rng, batteryCandidates, totalBatteries, candidates);
	}

	const u16 starter = RngNextRange(&self->rng, 0, totalStarters);
	self->level.segments[0].type = starterBegin + starter;

	for (usize i = 0; i < 3; ++i)
	{
		const u16 filler = fillerCandidates[i];
		const u16 battery = batteryCandidates[i];
		self->level.segments[(i * 2) + 1].type = fillerBegin + filler;
		self->level.segments[(i * 2) + 2].type = batteryBegin + battery;
	}

	const u16 solar = RngNextRange(&self->rng, 0, totalSolars);
	self->level.segments[7].type = solarBegin + solar;

	self->level.segmentsLength = MAX_LEVEL_SEGMENTS;

	Vector2 offset = VECTOR2_ZERO;
	self->bounds = (Rectangle) {
		.x = 0,
		.y = 0,
		.width = 0,
		.height = 180,
	};

	for (usize i = 0; i < self->level.segmentsLength; ++i)
	{
		LevelSegment* segment = &self->level.segments[i];
		LevelSegmentBuilder segmentBuilder = LevelSegmentBuilderCreate(self, segment->type, offset);

		segment->width = segmentBuilder.width;
		offset.x += segmentBuilder.width;
		self->bounds.width += segmentBuilder.width;
	}

	// Extend the end of the level so the player doesn't visibly fall out-of-bounds.
	{
		const Rectangle aabb = (Rectangle) {
			.x = self->bounds.width,
			.y = 16 * 10,
			.width = 16 * 5,
			.height = 16 * (2 + 4),
		};

		BlockBuilder* builder = ArenaAllocatorTake(&self->arenaAllocator, sizeof(BlockBuilder));
		builder->entity = SceneAllocateEntity(self);
		builder->aabb = aabb;
		builder->resolutionSchema = RESOLVE_ALL;
		builder->layer = LAYER_TERRAIN;
		SceneDefer(self, BlockBuild, builder);
	}

	{
		self->player = SceneAllocateEntity(self);
		PlayerBuilder* builder = ArenaAllocatorTake(&self->arenaAllocator, sizeof(PlayerBuilder));
		builder->entity = self->player;
		builder->handle = 0;
		builder->x = 16 * 5;
		builder->y = 16 * -4;
		SceneDefer(self, PlayerBuild, builder);
	}

	{
		self->fog = SceneAllocateEntity(self);
		FogBuilder* builder = ArenaAllocatorTake(&self->arenaAllocator, sizeof(FogBuilder));
		builder->entity = self->fog;
		SceneDefer(self, FogBuild, builder);
	}

	{
		self->lakitu = SceneAllocateEntity(self);
		LakituBuilder* builder = ArenaAllocatorTake(&self->arenaAllocator, sizeof(LakituBuilder));
		builder->entity = self->lakitu;
		SceneDefer(self, LakituBuild, builder);
	}
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
		const f32 x =
			-spacing + (i32)(spacing * i) + -48 + (RngNextRange(&self->rng, 0, 6 + 1) * 16);
		const f32 y = 8 + (RngNextRange(&self->rng, 0, 4 + 1) * 16);
		const Vector2 position = Vector2Create(x, y);
		DEQUE_PUSH_BACK(&self->treePositionsBack, Vector2, position);
	}

	for (usize i = 0; i < total; ++i)
	{
		const f32 x =
			-spacing + (i32)(spacing * i) + -32 + (RngNextRange(&self->rng, 0, 4 + 1) * 16);
		const f32 y = 8 + 8 + (RngNextRange(&self->rng, 0, 4 + 1) * 16);
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

static void SceneResetEcs(Scene* self)
{
	// We don't have to zero out each component array; just resetting the tags is sufficient.
	memset(&self->components.tags, 0, sizeof(u64) * MAX_ENTITIES);

	// Clear any deferred commands.
	{
		ArenaAllocatorFlush(&self->arenaAllocator);
		DequeClear(&self->deferred);
	}

	self->m_entityManager.m_nextFreshEntityIndex = 0;
	DequeClear(&self->m_entityManager.m_recycledEntityIndices);
}

static void SceneBuildStage(Scene* self)
{
	SceneBeginFadeIn(self);

	SceneResetEcs(self);

	ScenePopulateLevel(self);
	ScenePlantTrees(self);

	self->resetRequested = false;
	self->advanceStageRequested = false;

	SceneFlush(self);
}

static void SceneReset(Scene* self)
{
	self->stage = 0;

	self->score = 0;
	memset(self->scoreString, '0', sizeof(char) * MAX_SCORE_DIGITS);

	self->scoreBufferTimerDuration = CTX_DT * 2;
	self->scoreBufferTimer = 0;
	self->scoreBuffer = 0;

	self->totalBatteries = 0;

	SceneBuildStage(self);
}

static void SceneAdvanceStage(Scene* self)
{
	self->stage += 1;
	self->stage = MIN(self->stage, 64);

	self->totalBatteries = 0;

	const CMortal playersMortal = self->components.mortals[self->player];

	SceneBuildStage(self);

	// TODO(thismarvin): Is there a better way to preserve the player's health?
	self->components.mortals[self->player] = playersMortal;
}

void SceneInit(Scene* self)
{
#if !defined(BENCHMARKING)
	SceneSetupContent(self);
	SceneSetupLayers(self);
#endif

	SceneSetupInput(self);

	self->frame = 0;
	self->elapsedTime = 0;

#if defined(NDEBUG)
	self->seed = time(NULL);
#else
	self->seed = MAGIC_NUMBER;
#endif

	TraceLog(LOG_INFO, "SEED: %lu", self->seed);

	self->rng = RngCreate(self->seed);

	self->deferred = DEQUE_OF(SceneDeferParams);

	// Setup input recording.
	{
		for (usize i = 0; i < MAX_PLAYERS; ++i)
		{
			self->inputStreams[i] = InputStreamCreate(TOTAL_INPUT_BINDINGS, RECORDING_SIZE);
		}
	}

	self->state = SCENE_STATE_MENU;

	self->debugging = false;

	self->m_entityManager = (EntityManager) {
		.m_nextFreshEntityIndex = 0,
		.m_recycledEntityIndices = DEQUE_WITH_CAPACITY(usize, MAX_ENTITIES),
	};

	self->treePositionsBack = DEQUE_OF(Vector2);
	self->treePositionsFront = DEQUE_OF(Vector2);

	self->director = DIRECTOR_STATE_ENTRANCE;
	self->fader = FaderDefault();
	self->fader.easer.ease = EaseInOutQuad;

	self->arenaAllocator = ArenaAllocatorCreate((usize)(1024 * 8));

	SceneReset(self);
}

f64 SceneGetElapsedTime(const Scene* self)
{
	return self->elapsedTime;
}

static usize BufferFromInputBinding(const InputBinding binding)
{
	switch (binding)
	{
		case INPUT_BINDING_JUMP: {
			return 8;
		}
		default: {
			return 1;
		}
	}
}

bool ScenePressing(const Scene* self, const u8 player, const InputBinding binding)
{
	return InputStreamPressing(&self->inputStreams[player], binding, self->frame);
}

bool ScenePressed(const Scene* self, const u8 player, const InputBinding binding)
{
	const usize buffer = BufferFromInputBinding(binding);

	return InputStreamPressed(&self->inputStreams[player], binding, buffer, self->frame);
}

bool SceneReleased(const Scene* self, const u8 player, const InputBinding binding)
{
	const usize buffer = BufferFromInputBinding(binding);

	return InputStreamReleased(&self->inputStreams[player], binding, buffer, self->frame);
}

void SceneConsume(Scene* self, const u8 player, const InputBinding binding)
{
	InputStreamConsume(&self->inputStreams[player], binding, self->frame);
}

void SceneIncrementScore(Scene* self, const u32 value)
{
	self->scoreBuffer += value;
	self->scoreBuffer = MIN(self->scoreBuffer, MAX_SCORE);
}

void SceneCollectBattery(Scene* self)
{
	self->totalBatteries += 1;
	self->totalBatteries = MIN(self->totalBatteries, 3);
}

void SceneConsumeBattery(Scene* self)
{
	if (self->totalBatteries == 0)
	{
		return;
	}

	self->totalBatteries -= 1;
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
	// TODO(thismarvin): Move this to fog.c?

	assert(SceneEntityHasDependencies(self, self->player, TAG_POSITION));
	assert(SceneEntityHasDependencies(self, self->fog, TAG_POSITION));

	const CPosition* fogPosition = &self->components.positions[self->fog];
	const CPosition* playerPosition = &self->components.positions[self->player];

	const f32 distance = fogPosition->value.x - playerPosition->value.x;

	if (distance > FOG_LETHAL_DISTANCE)
	{
		// TODO(thismarvin): Isn't the transition redundant here?
		SceneDeferReset(self);
	}
}

static void SceneMenuUpdate(Scene* self)
{
	// Only give player-one control in the menu state.
	if (ScenePressed(self, 0, INPUT_BINDING_JUMP))
	{
		SceneConsume(self, 0, INPUT_BINDING_JUMP);

		// TODO(thismarvin): Defer this somehow...
		self->state = SCENE_STATE_ACTION;

		// TODO(thismarvin): There should be a bespoke transition into the Action state.
		SceneBeginFadeIn(self);
		self->fader.easer.duration = CTX_DT * 40;
	}

	// TODO(thismarvin): The following is very hacky! Also, maybe put this in LakituUpdate?
	{
		const CPosition* position = &self->components.positions[self->lakitu];

		SSmoothUpdate(self, self->lakitu);
		SKineticUpdate(self, self->lakitu);

		if (position->value.x > self->bounds.width - CTX_VIEWPORT_WIDTH * 0.75)
		{
			self->resetRequested = true;
		}
	}
}

static void SceneUpdateDirectorMenu(Scene* self)
{
	switch (self->director)
	{
		case DIRECTOR_STATE_ENTRANCE: {
			FaderUpdate(&self->fader);

			if (FaderIsDone(&self->fader))
			{
				self->director = DIRECTOR_STATE_SUPERVISE;
			}

			break;
		}
		case DIRECTOR_STATE_SUPERVISE: {
			if (self->resetRequested)
			{
				SceneBeginFadeOut(self);
			}

			break;
		}
		case DIRECTOR_STATE_EXIT: {
			FaderUpdate(&self->fader);

			if (FaderIsDone(&self->fader))
			{
				CPosition* position = &self->components.positions[self->lakitu];

				position->value.x = CTX_VIEWPORT_WIDTH * 0.5;
				self->components.smooths[self->lakitu].previous = position->value;

				self->resetRequested = false;

				SceneBeginFadeIn(self);
			}

			break;
		}
	}
}

static void SceneUpdateDirectorAction(Scene* self)
{
	switch (self->director)
	{
		case DIRECTOR_STATE_ENTRANCE: {
			FaderUpdate(&self->fader);

			if (FaderIsDone(&self->fader))
			{
				self->director = DIRECTOR_STATE_SUPERVISE;
			}

			break;
		}
		case DIRECTOR_STATE_SUPERVISE: {
			if (self->resetRequested || self->advanceStageRequested)
			{
				SceneBeginFadeOut(self);
			}

			break;
		}
		case DIRECTOR_STATE_EXIT: {
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
		case SCENE_STATE_MENU: {
			SceneUpdateDirectorMenu(self);
			break;
		}
		case SCENE_STATE_ACTION: {
			SceneUpdateDirectorAction(self);
			break;
		}
	}
}

static void SceneActionUpdate(Scene* self)
{
	// TODO(thismarvin): Should entity allocation be in-place?

	// Systems can allocate entities in-place; make sure we only iterate through the entities that
	// existed at the beginning of the frame.
	const usize entities = SceneGetTotalAllocatedEntities(self);

	RUN_SYSTEM(SFleetingUpdate, self, entities);
	RUN_SYSTEM(SSmoothUpdate, self, entities);
	RUN_SYSTEM(SAnimationUpdate, self, entities);
	RUN_SYSTEM(BatteryUpdate, self, entities);
	RUN_SYSTEM(PlayerInputUpdate, self, entities);
	/**/ RUN_SYSTEM(PlayerShadowUpdate, self, entities);
	/**/ RUN_SYSTEM(SKineticUpdate, self, entities);
	/****/ RUN_SYSTEM(SCollisionUpdate, self, entities);
	/******/ RUN_SYSTEM(SPostCollisionUpdate, self, entities);
	/********/ RUN_SYSTEM(PlayerPostCollisionUpdate, self, entities);
	/**********/ RUN_SYSTEM(PlayerMortalUpdate, self, entities);
	/**********/ RUN_SYSTEM(PlayerTrailUpdate, self, entities);
	/**********/ RUN_SYSTEM(PlayerAnimationUpdate, self, entities);
	/************/ RUN_SYSTEM(FogUpdate, self, entities);

	SceneUpdateScore(self);
	SceneCheckEndCondition(self);
}

static void SceneUpdateInput(Scene* self)
{
	// Update input handlers.
	for (usize i = 0; i < MAX_PLAYERS; ++i)
	{
		InputHandlerUpdate(&self->inputs[i]);
	}

	// Update input streams.
	for (usize i = 0; i < MAX_PLAYERS; ++i)
	{
		const bool replayInProgress = self->inputStreams[i].length > self->frame;

		if (replayInProgress)
		{
			continue;
		}

		bool payload[TOTAL_INPUT_BINDINGS] = { false };

		if (InputHandlerPressing(&self->inputs[i], "left"))
		{
			payload[0] = true;
		}
		if (InputHandlerPressing(&self->inputs[i], "right"))
		{
			payload[1] = true;
		}
		if (InputHandlerPressing(&self->inputs[i], "jump"))
		{
			payload[2] = true;
		}
		if (InputHandlerPressing(&self->inputs[i], "stomp"))
		{
			payload[3] = true;
		}

		InputStreamPush(&self->inputStreams[i], payload);
	}
}

void SceneUpdate(Scene* self)
{
	SceneUpdateInput(self);

#if defined(PLATFORM_DESKTOP)
	if (IsKeyPressed(KEY_F11))
	{
		ToggleFullscreenShim();
	}
#endif

	if (IsKeyPressed(KEY_EQUAL))
	{
		self->debugging = !self->debugging;
	}

	if (IsKeyPressed(KEY_MINUS))
	{
		ReplayResult result = ReplayTryFromInputStream(self->seed, &self->inputStreams[0]);

		if (result.type == REPLAY_RESULT_TYPE_OK)
		{
			Replay* replay = &result.contents.ok;
			ReplayBytes bytes = ReplayBytesFromReplay(replay);

			SaveFileData("recording.ltlrr", bytes.data, bytes.size);

			ReplayBytesDestroy(&bytes);
			ReplayDestroy(replay);

#if defined(PLATFORM_WEB)
			EM_ASM((saveFileFromMEMFSToDisk("recording.ltlrr", "recording.ltlrr");));
#endif
		}
		else
		{
			TraceLog(LOG_ERROR, StringFromReplayError(result.contents.err));
		}
	}

	switch (self->state)
	{
		case SCENE_STATE_MENU: {
			SceneMenuUpdate(self);
			break;
		}
		case SCENE_STATE_ACTION: {
			SceneActionUpdate(self);
			break;
		}
	}

	SceneUpdateDirector(self);

	SceneFlush(self);

	// TODO(thismarvin): Should this be at the end? Don't we usually have it first?!
	self->frame += 1;
	self->elapsedTime += CTX_DT;
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

		const Vector2 offset = (Vector2) {
			.x = dimension->width * 0.5,
			.y = dimension->height * 0.5,
		};

		cameraPosition = Vector2Add(cameraPosition, offset);
	}

	// Camera x-axis collision.
	{
		const f32 min = RectangleLeft(self->bounds) + (CTX_VIEWPORT_WIDTH * 0.5);
		const f32 max = RectangleRight(self->bounds) - (CTX_VIEWPORT_WIDTH * 0.5);

		cameraPosition.x = MAX(min, cameraPosition.x);
		cameraPosition.x = MIN(max, cameraPosition.x);
	}

	// Camera y-axis collision.
	{
		const f32 min = RectangleTop(self->bounds) + (CTX_VIEWPORT_HEIGHT * 0.5);
		const f32 max = RectangleBottom(self->bounds) - (CTX_VIEWPORT_HEIGHT * 0.5);

		cameraPosition.y = MAX(min, cameraPosition.y);
		cameraPosition.y = MIN(max, cameraPosition.y);
	}

	return (Rectangle) {
		.x = cameraPosition.x - (CTX_VIEWPORT_WIDTH * 0.5),
		.y = cameraPosition.y - (CTX_VIEWPORT_HEIGHT * 0.5),
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

		const AtlasDrawParams params = (AtlasDrawParams) {
			.sprite = digit,
			.position = (Vector2) { position.x + (index * 14), position.y },
			.scale = Vector2Create(1, 1),
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

	const Rectangle intramural = (Rectangle) {
		.x = 8,
		.y = 7,
		.width = 19,
		.height = 17,
	};

	// Draw heart capsules.
	for (usize i = 0; i < totalHearts; ++i)
	{
		const Vector2 myPosition = (Vector2) {
			.x = position.x + (i * 15),
			.y = position.y,
		};

		const AtlasDrawParams params = (AtlasDrawParams) {
			.sprite = SPRITE_HEART_0000,
			.position = myPosition,
			.scale = Vector2Create(1, 1),
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
			const Vector2 myPosition = (Vector2) { .x = position.x + (i * 15), .y = position.y };

			const AtlasDrawParams params = (AtlasDrawParams) {
				.sprite = SPRITE_HEART_0001,
				.position = myPosition,
				.scale = Vector2Create(1, 1),
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

static void DrawTree(const RenderFnParams* params, const Vector2 position, const f32 scrollFactor)
{
	const Scene* scene = (Scene*)params->scene;

	const RenderTexture* renderTexture = &scene->treeTexture;

	const f32 domain = scene->bounds.width - CTX_VIEWPORT_WIDTH;
	const f32 progress = scene->actionCameraPosition.x / domain;
	const f32 partialDomain = domain * scrollFactor;
	const f32 offset = partialDomain * progress;

	const f32 x = (-renderTexture->texture.width * 0.5) + position.x - offset;
	const f32 y = position.y;

	const Rectangle destination = (Rectangle) {
		.x = x,
		.y = y,
		.width = renderTexture->texture.width,
		.height = renderTexture->texture.height,
	};

	Rectangle source = RectangleFromRenderTexture(renderTexture);
	source.height *= -1;

	const Color tint = (Color) {
		.r = 182,
		.g = 215,
		.b = 236,
		.a = 255,
	};

	if (CheckCollisionRecs(params->cameraBounds, destination))
	{
		DrawTexturePro(renderTexture->texture, source, destination, VECTOR2_ZERO, 0, tint);
	}
}

static void DrawTreeLayer(
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

			const Rectangle segmentBounds = (Rectangle) {
				.x = offset.x,
				.y = 0,
				.width = segment->width,
				.height = CTX_VIEWPORT_HEIGHT,
			};

			if (CheckCollisionRecs(params->cameraBounds, segmentBounds))
			{
				LevelSegmentDraw(segment, &scene->atlas, offset);
			}

			offset.x += segment->width;
		}
	}

	const usize entities = SceneGetTotalAllocatedEntities(scene);

	// TODO(thismarvin): There needs to be a better way to sort draw calls...

	RUN_SYSTEM(SSpriteDraw, scene, entities);
	RUN_SYSTEM(SAnimationDraw, scene, entities);
}

static void RenderTargetLayerShader(const RenderFnParams* params)
{
	const Scene* scene = (Scene*)params->scene;

	const RenderTexture2D* renderTexture = &scene->targetLayer;

	const i32 width = renderTexture->texture.width;
	const i32 height = renderTexture->texture.height;

	const Rectangle destination = (Rectangle) {
		.x = floor(width * 0.5),
		.y = floor(height * 0.5),
		.width = width,
		.height = height,
	};

	const Vector2 origin = (Vector2) {
		.x = floor(width * 0.5),
		.y = floor(height * 0.5),
	};

	BeginShaderMode(scene->dropShadow);
	BeginDrawing();
	{
		ClearBackground(COLOR_TRANSPARENT);

		Rectangle source = RectangleFromRenderTexture(renderTexture);
		source.height *= -1;

		DrawTexturePro(renderTexture->texture, source, destination, origin, 0, COLOR_WHITE);
	}
	EndDrawing();
	EndShaderMode();
}

static void RenderMenuInterface(const RenderFnParams* params)
{
	const Scene* scene = (Scene*)params->scene;

	const Color transparentBlack = (Color) { 0, 0, 0, 100 };

	ClearBackground(transparentBlack);

	// Draw logo.
	{
		const AtlasDrawParams drawParams = (AtlasDrawParams) {
			.sprite = SPRITE_LOGO,
			.position = (Vector2) { 32, (180.0 - 112) / 2 },
			.scale = Vector2Create(1, 1),
			.intramural = (Rectangle) { 0, 0, 0, 0 },
			.reflection = REFLECTION_NONE,
			.tint = COLOR_WHITE,
		};
		AtlasDraw(&scene->atlas, &drawParams);
	}
}

static void RenderActionInterface(const RenderFnParams* params)
{
	static const usize healthBarWidth = (15 * PLAYER_MAX_HIT_POINTS) + 4;
	static const usize padding = 4;

	const Scene* scene = (Scene*)params->scene;

	ClearBackground(COLOR_TRANSPARENT);

	SceneDrawScore(scene, Vector2Create(padding, padding));
	SceneDrawHealthBar(
		scene,
		Vector2Create(CTX_VIEWPORT_WIDTH - padding - healthBarWidth, padding)
	);
}

static void RenderInterfaceLayer(const RenderFnParams* params)
{
	const Scene* scene = (Scene*)params->scene;

	switch (scene->state)
	{
		case SCENE_STATE_MENU: {
			RenderMenuInterface(params);
			break;
		}
		case SCENE_STATE_ACTION: {
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

	const usize entities = SceneGetTotalAllocatedEntities(scene);

	RUN_SYSTEM(CloudParticleDraw, scene, entities);
	RUN_SYSTEM(FogParticleDraw, scene, entities);
	RUN_SYSTEM(FogDraw, scene, entities);
}

static void RenderDebugLayer(const RenderFnParams* params)
{
	const Scene* scene = (Scene*)params->scene;

	ClearBackground(COLOR_TRANSPARENT);

	if (!scene->debugging)
	{
		return;
	}

	const usize entities = SceneGetTotalAllocatedEntities(scene);

	RUN_SYSTEM(SDebugColliderDraw, scene, entities);
	RUN_SYSTEM(FogDebugDraw, scene, entities);
	RUN_SYSTEM(PlayerDebugDraw, scene, entities);
}

static void SceneMenuDraw(Scene* self)
{
	Rectangle actionCameraBounds = SceneCalculateActionCameraBounds(self, self->lakitu);

	self->actionCameraPosition = (Vector2) {
		.x = actionCameraBounds.x,
		.y = actionCameraBounds.y,
	};

	const RenderFnParams actionCameraParams = (RenderFnParams) {
		.scene = self,
		.cameraBounds = actionCameraBounds,
	};
	const RenderFnParams stationaryCameraParams = (RenderFnParams) {
		.scene = self,
		.cameraBounds = CTX_VIEWPORT,
	};

	RenderLayer(&self->rootLayer, RenderRootLayer, &stationaryCameraParams);
	RenderLayer(&self->backgroundLayer, RenderBackgroundLayer, &stationaryCameraParams);
	RenderLayer(&self->targetLayer, RenderTargetLayer, &actionCameraParams);
	RenderLayer(&self->targetLayerBuffer, RenderTargetLayerShader, &stationaryCameraParams);
	RenderLayer(&self->interfaceLayer, RenderInterfaceLayer, &stationaryCameraParams);
	RenderLayer(&self->transitionLayer, RenderTransitionLayer, &stationaryCameraParams);

	// clang-format off
	const RenderTexture renderTextures[5] = {
		self->rootLayer,
		self->backgroundLayer,
		self->targetLayerBuffer,
		self->interfaceLayer,
		self->transitionLayer,
	};
	// clang-format on
	DrawLayers(renderTextures, 5);
}

static void SceneActionDraw(Scene* self)
{
	Rectangle actionCameraBounds = SceneCalculateActionCameraBounds(self, self->player);

	self->actionCameraPosition = (Vector2) {
		.x = actionCameraBounds.x,
		.y = actionCameraBounds.y,
	};

	const RenderFnParams actionCameraParams = (RenderFnParams) {
		.scene = self,
		.cameraBounds = actionCameraBounds,
	};
	const RenderFnParams stationaryCameraParams = (RenderFnParams) {
		.scene = self,
		.cameraBounds = CTX_VIEWPORT,
	};

	RenderLayer(&self->rootLayer, RenderRootLayer, &stationaryCameraParams);
	RenderLayer(&self->backgroundLayer, RenderBackgroundLayer, &stationaryCameraParams);
	RenderLayer(&self->targetLayer, RenderTargetLayer, &actionCameraParams);
	RenderLayer(&self->targetLayerBuffer, RenderTargetLayerShader, &stationaryCameraParams);
	RenderLayer(&self->foregroundLayer, RenderForegroundLayer, &actionCameraParams);
	RenderLayer(&self->interfaceLayer, RenderInterfaceLayer, &stationaryCameraParams);
	RenderLayer(&self->transitionLayer, RenderTransitionLayer, &stationaryCameraParams);
	RenderLayer(&self->debugLayer, RenderDebugLayer, &actionCameraParams);

	// clang-format off
	const RenderTexture renderTextures[7] = {
		self->rootLayer,
		self->backgroundLayer,
		self->targetLayerBuffer,
		self->foregroundLayer,
		self->interfaceLayer,
		self->transitionLayer,
		self->debugLayer,
	};
	// clang-format on
	DrawLayers(renderTextures, 7);
}

void SceneDraw(Scene* self)
{
	BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);

	switch (self->state)
	{
		case SCENE_STATE_MENU: {
			SceneMenuDraw(self);
			break;
		}
		case SCENE_STATE_ACTION: {
			SceneActionDraw(self);
			break;
		}
	}

	EndBlendMode();
}

void SceneDestroy(Scene* self)
{
	AtlasDestroy(&self->atlas);

	DequeDestroy(&self->deferred);
	DequeDestroy(&self->m_entityManager.m_recycledEntityIndices);
	DequeDestroy(&self->treePositionsBack);
	DequeDestroy(&self->treePositionsFront);

	ArenaAllocatorDestroy(&self->arenaAllocator);

	UnloadRenderTexture(self->treeTexture);
	UnloadRenderTexture(self->backgroundLayer);
	UnloadRenderTexture(self->targetLayer);
	UnloadRenderTexture(self->targetLayerBuffer);
	UnloadRenderTexture(self->foregroundLayer);
	UnloadRenderTexture(self->interfaceLayer);
	UnloadRenderTexture(self->transitionLayer);
	UnloadRenderTexture(self->debugLayer);

	for (usize i = 0; i < MAX_PLAYERS; ++i)
	{
		InputProfileDestroy(&self->inputProfiles[i]);
	}

#if !defined(NDEBUG)
	// Dump player-one's input.
	{
		ReplayResult result = ReplayTryFromInputStream(self->seed, &self->inputStreams[0]);

		if (result.type == REPLAY_RESULT_TYPE_OK)
		{
			Replay* replay = &result.contents.ok;
			ReplayBytes bytes = ReplayBytesFromReplay(replay);

			SaveFileData("debug_recording.ltlrr", bytes.data, bytes.size);

			ReplayBytesDestroy(&bytes);
			ReplayDestroy(replay);
		}
	}
#endif

	for (usize i = 0; i < MAX_PLAYERS; ++i)
	{
		InputStreamDestroy(&self->inputStreams[i]);
	}

	UnloadShader(self->dropShadow);
}
