#pragma once

#define CREATE_SOLID_BLOCK(mX, mY, mWidth, mHeight) \
	do \
	{ \
		const Rectangle aabb = (Rectangle) { \
			.x = (mX) + offset.x, \
			.y = (mY) + offset.y, \
			.width = (mWidth), \
			.height = (mHeight), \
		}; \
		BlockBuilder* builder = ArenaAllocatorTake(&scene->arenaAllocator, sizeof(BlockBuilder)); \
		builder->entity = SceneAllocateEntity(scene); \
		builder->aabb = aabb; \
		builder->resolutionSchema = RESOLVE_ALL; \
		builder->layer = LAYER_TERRAIN; \
		SceneDefer(scene, BlockBuild, builder); \
	} while (0)

#define CREATE_ONE_WAY_BLOCK(mX, mY, mWidth, mHeight) \
	do \
	{ \
		const Rectangle aabb = (Rectangle) { \
			.x = (mX) + offset.x, \
			.y = (mY) + offset.y, \
			.width = (mWidth), \
			.height = (mHeight), \
		}; \
		BlockBuilder* builder = ArenaAllocatorTake(&scene->arenaAllocator, sizeof(BlockBuilder)); \
		builder->entity = SceneAllocateEntity(scene); \
		builder->aabb = aabb; \
		builder->resolutionSchema = RESOLVE_UP; \
		builder->layer = LAYER_TERRAIN; \
		SceneDefer(scene, BlockBuild, builder); \
	} while (0)

#define CREATE_INVISIBLE_BLOCK(mX, mY, mWidth, mHeight) \
	do \
	{ \
		const Rectangle aabb = (Rectangle) { \
			.x = (mX) + offset.x, \
			.y = (mY) + offset.y, \
			.width = (mWidth), \
			.height = (mHeight), \
		}; \
		BlockBuilder* builder = ArenaAllocatorTake(&scene->arenaAllocator, sizeof(BlockBuilder)); \
		builder->entity = SceneAllocateEntity(scene); \
		builder->aabb = aabb; \
		builder->resolutionSchema = RESOLVE_ALL; \
		builder->layer = LAYER_INVISIBLE; \
		SceneDefer(scene, BlockBuild, builder); \
	} while (0)

#define CREATE_SPIKE(mX, mY, mRotation) \
	do \
	{ \
		const f32 x = (mX) + offset.x; \
		const f32 y = (mY) + offset.y; \
		SpikeBuilder* builder = ArenaAllocatorTake(&scene->arenaAllocator, sizeof(SpikeBuilder)); \
		builder->entity = SceneAllocateEntity(scene); \
		builder->x = x; \
		builder->y = y; \
		builder->rotation = (mRotation); \
		SceneDefer(scene, SpikeBuild, builder); \
	} while (0)

#define CREATE_WALKER(mX, mY) \
	do \
	{ \
		const f32 x = (mX) + offset.x; \
		const f32 y = (mY) + offset.y; \
		WalkerBuilder* builder = \
			ArenaAllocatorTake(&scene->arenaAllocator, sizeof(WalkerBuilder)); \
		builder->entity = SceneAllocateEntity(scene); \
		builder->x = x; \
		builder->y = y; \
		SceneDefer(scene, WalkerBuild, builder); \
	} while (0)

#define CREATE_BATTERY(mX, mY) \
	do \
	{ \
		const f32 x = (mX) + offset.x + 1; \
		const f32 y = (mY) + offset.y; \
		BatteryBuilder* builder = \
			ArenaAllocatorTake(&scene->arenaAllocator, sizeof(BatteryBuilder)); \
		builder->entity = SceneAllocateEntity(scene); \
		builder->x = x; \
		builder->y = y; \
		SceneDefer(scene, BatteryBuild, builder); \
	} while (0)

#define CREATE_SOLAR_PANEL(mX, mY) \
	do \
	{ \
		const f32 x = (mX) + offset.x - 36; \
		const f32 y = (mY) + offset.y - 24; \
		SolarPanelBuilder* builder = \
			ArenaAllocatorTake(&scene->arenaAllocator, sizeof(SolarPanelBuilder)); \
		builder->entity = SceneAllocateEntity(scene); \
		builder->x = x; \
		builder->y = y; \
		SceneDefer(scene, SolarPanelBuild, builder); \
	} while (0)

#define DRAW_SPRITE(mSprite, mX, mY) \
	do \
	{ \
		const AtlasDrawParams params = (AtlasDrawParams) { \
			.sprite = (mSprite), \
			.position = Vector2Create((mX) + offset.x, (mY) + offset.y), \
			.scale = Vector2Create(1, 1), \
			.intramural = (Rectangle) { 0, 0, 0, 0 }, \
			.reflection = REFLECTION_NONE, \
			.tint = COLOR_WHITE, \
		}; \
		AtlasDraw(atlas, &params); \
	} while (0)

#define DRAW_COVER_UP(mX, mY, mColor) \
	do \
	{ \
		const AtlasDrawParams params = (AtlasDrawParams) { \
			.sprite = SPRITE_PIXEL, \
			.position = Vector2Create((mX)-0.5F + offset.x, (mY)-0.5F + offset.y), \
			.scale = Vector2Create(7, 7), \
			.intramural = (Rectangle) { 0, 0, 0, 0 }, \
			.reflection = REFLECTION_NONE, \
			.tint = (mColor), \
		}; \
		AtlasDraw(atlas, &params); \
	} while (0)
