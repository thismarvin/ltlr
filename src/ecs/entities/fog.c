#include "fog.h"

#include "../../palette/p8.h"
#include "fog_particle.h"

#include <assert.h>
#include <raymath.h>

#define FOG_HEIGHT (CTX_VIEWPORT_HEIGHT * 2)
#define FOG_INITIAL_POSITION \
	(Vector2) \
	{ \
		.x = -CTX_VIEWPORT_WIDTH * 0.5, .y = -(FOG_HEIGHT - CTX_VIEWPORT_HEIGHT) * 0.5f, \
	}

#define FOG_LUMP_TOTAL (8)

#define FOG_SPEED (50)
#define FOG_DECELERATION_DELTA (128.0)
// a = (vf^2 - vo^2) / (2 * (xf - xo))
#define FOG_DECELERATION ((0.0 - FOG_SPEED * FOG_SPEED) / (2.0 * FOG_DECELERATION_DELTA))
// t = (vf - vo) / a
#define FOG_DECELERATION_DURATION ((0.0 - FOG_SPEED) / FOG_DECELERATION)

static const f32 baseRadius = (f32)FOG_HEIGHT / FOG_LUMP_TOTAL * 0.75f;
static const f32 lumpSpacing = (f32)FOG_HEIGHT / FOG_LUMP_TOTAL;
static f32 lumpRadii[FOG_LUMP_TOTAL];
static f32 lumpTargetRadii[FOG_LUMP_TOTAL];
static const f32 breathingPhaseDuration = 4.0f;
static f32 breathingPhaseTimer = 0;
static u8 breathingPhase = 0;

static const f32 movingParticleSpawnDuration = 0.025f;
static f32 movingParticleSpawnTimer = movingParticleSpawnDuration;

static bool decelerationTimerEnabled = false;
static f32 decelerationTimer = 0.0;

static void FogReset(void)
{
	for (usize i = 0; i < FOG_LUMP_TOTAL; ++i)
	{
		lumpRadii[i] = baseRadius;
		lumpTargetRadii[i] = baseRadius;
	}

	breathingPhaseTimer = 0;
	breathingPhase = 0;

	movingParticleSpawnTimer = movingParticleSpawnDuration;

	decelerationTimerEnabled = false;
	decelerationTimer = 0.0;
}

void FogBuildHelper(Scene* scene, const FogBuilder* builder)
{
	FogReset();

	// clang-format off
	scene->components.tags[builder->entity] =
		TAG_NONE
		| TAG_IDENTIFIER
		| TAG_POSITION
		| TAG_KINETIC
		| TAG_SMOOTH;
	// clang-format on

	scene->components.identifiers[builder->entity] = (CIdentifier) {
		.type = ENTITY_TYPE_FOG,
	};

	scene->components.positions[builder->entity] = (CPosition) {
		.value = FOG_INITIAL_POSITION,
	};

	scene->components.kinetics[builder->entity] = (CKinetic) {
		.velocity = VECTOR2_ZERO,
		.acceleration = VECTOR2_ZERO,
	};

	scene->components.smooths[builder->entity] = (CSmooth) {
		.previous = FOG_INITIAL_POSITION,
	};
}

void FogBuild(Scene* scene, const void* params)
{
	FogBuildHelper(scene, params);
}

static void SpawnMovingParticles(Scene* scene, const CPosition* position, const CKinetic* kinetic)
{
	static const i32 minSize = 3;
	static const i32 maxSize = 5;

	static const i32 minXSpeed = 30;
	static const i32 maxXSpeed = 50;
	static const i32 minYSpeed = -3;
	static const i32 maxYSpeed = 3;

	static const i32 minLifetime = 5;
	static const i32 maxLifetime = 8;

	const Vector2 spawnPosition = (Vector2) {
		.x = position->value.x + (baseRadius * 0.5f),
		.y = position->value.y + FOG_HEIGHT * 0.25f + GetRandomValue(0, FOG_HEIGHT * 0.5f),
	};

	const Vector2 velocity = (Vector2) {
		.x = kinetic->velocity.x + GetRandomValue(minXSpeed, maxXSpeed),
		.y = GetRandomValue(minYSpeed, maxYSpeed),
	};

	const f32 radius = GetRandomValue(minSize, maxSize);
	const f32 lifetime = 0.1f * GetRandomValue(minLifetime, maxLifetime);

	FogParticleBuilder* builder = malloc(sizeof(FogParticleBuilder));
	builder->entity = SceneAllocateEntity(scene);
	builder->position = spawnPosition;
	builder->velocity = velocity;
	builder->radius = radius;
	builder->lifetime = lifetime;

	SceneDefer(scene, FogParticleBuild, builder);
}

static void ShiftBreathingPhase()
{
	switch (breathingPhase)
	{
		case 0: {
			for (usize i = 0; i < FOG_LUMP_TOTAL; ++i)
			{
				lumpRadii[i] = lumpTargetRadii[i];

				if (i % 2 == 0)
				{
					lumpTargetRadii[i] = baseRadius;
				}
				else
				{
					lumpTargetRadii[i] = baseRadius * 0.75f;
				}
			}

			break;
		}

		case 1: {
			for (usize i = 0; i < FOG_LUMP_TOTAL; ++i)
			{
				lumpRadii[i] = lumpTargetRadii[i];
				lumpTargetRadii[i] = baseRadius;
			}

			break;
		}

		case 2: {
			for (usize i = 0; i < FOG_LUMP_TOTAL; ++i)
			{
				lumpRadii[i] = lumpTargetRadii[i];

				if (i % 2 == 0)
				{
					lumpTargetRadii[i] = baseRadius * 0.75f;
				}
				else
				{
					lumpTargetRadii[i] = baseRadius;
				}
			}

			break;
		}

		case 3: {
			for (usize i = 0; i < FOG_LUMP_TOTAL; ++i)
			{
				lumpRadii[i] = lumpTargetRadii[i];
				lumpTargetRadii[i] = baseRadius;
			}

			break;
		}
	}

	breathingPhase = (breathingPhase + 1) % 4;
}

void FogUpdate(Scene* scene, const usize entity)
{
	static const u64 dependencies = TAG_POSITION | TAG_KINETIC;

	if (!SceneEntityIs(scene, entity, ENTITY_TYPE_FOG)
		|| !SceneEntityHasDependencies(scene, entity, dependencies))
	{
		return;
	}

	assert(SceneEntityHasDependencies(scene, scene->player, TAG_POSITION));

	const CPosition* playerPosition = &scene->components.positions[scene->player];
	CPosition* position = &scene->components.positions[entity];
	CKinetic* kinetic = &scene->components.kinetics[entity];

	const bool hasNotMoved = kinetic->velocity.x == 0;

	if (hasNotMoved && playerPosition->value.x < CTX_VIEWPORT_WIDTH)
	{
		return;
	}

	kinetic->velocity.y = cosf(ContextGetTotalTime() * 0.5f) * 8;

	// Make sure the fog does not overlap the level's last segment.
	{
		const f32 lastSegmentWidth = scene->level.segments[scene->level.segmentsLength - 1].width;
		const f32 xMax = scene->bounds.width - lastSegmentWidth;

		if (!decelerationTimerEnabled)
		{
			kinetic->velocity.x = FOG_SPEED;

			// Decelerate as the fog reaches its limit.
			if (position->value.x + baseRadius >= xMax - FOG_DECELERATION_DELTA)
			{
				kinetic->acceleration.x = FOG_DECELERATION;
				decelerationTimerEnabled = true;
				decelerationTimer = 0;
			}
		}
		else
		{
			if (decelerationTimer < FOG_DECELERATION_DURATION)
			{
				decelerationTimer += CTX_DT;
			}
			else
			{
				kinetic->acceleration.x = 0;
				kinetic->velocity.x = 0;
			}
		}
	}

	// Moving Particle spawn logic.
	{
		movingParticleSpawnTimer += CTX_DT;

		if (movingParticleSpawnTimer >= movingParticleSpawnDuration)
		{
			if (GetRandomValue(0, 9) != 0)
			{
				SpawnMovingParticles(scene, position, kinetic);
			}

			movingParticleSpawnTimer = 0;
		}
	}

	// Smooth phase transitioning logic for breathing.
	{
		breathingPhaseTimer += CTX_DT;

		if (breathingPhaseTimer >= breathingPhaseDuration)
		{
			ShiftBreathingPhase();
			breathingPhaseTimer = 0;
		}
	}
}

void FogDraw(const Scene* scene, const usize entity)
{
	static const u64 dependencies = TAG_POSITION | TAG_SMOOTH;

	if (!SceneEntityIs(scene, entity, ENTITY_TYPE_FOG)
		|| !SceneEntityHasDependencies(scene, entity, dependencies))
	{
		return;
	}

	const CPosition* position = &scene->components.positions[entity];
	const CSmooth* smooth = &scene->components.smooths[entity];

	const Vector2 interpolated = Vector2Lerp(smooth->previous, position->value, ContextGetAlpha());

	const f32 step = breathingPhaseTimer / breathingPhaseDuration;

	static const f32 multiplier = 10;
	const f32 time = ContextGetTotalTime() * 2;

	for (usize i = 0; i < FOG_LUMP_TOTAL; ++i)
	{
		const f32 radius = Lerp(lumpRadii[i], lumpTargetRadii[i], step);
		const f32 offset = cosf((f32)i / FOG_LUMP_TOTAL * 2 * PI + time) * multiplier;
		const Vector2 center =
			Vector2Create(interpolated.x + offset, interpolated.y + (lumpSpacing * i));
		DrawCircleV(center, radius * 1.1f, COLOR_WHITE);
	}

	for (usize i = 0; i < FOG_LUMP_TOTAL; ++i)
	{
		const f32 radius = Lerp(lumpRadii[i], lumpTargetRadii[i], step);
		const f32 offset = cosf((f32)i / FOG_LUMP_TOTAL * 2 * PI + time) * multiplier;
		const Vector2 center =
			Vector2Create(interpolated.x + offset, interpolated.y + (lumpSpacing * i));
		DrawCircleV(center, radius, COLOR_BLACK);
	}

	{
		const f32 x = interpolated.x - CTX_VIEWPORT_WIDTH * 2;
		const f32 y = interpolated.y;
		const f32 width = CTX_VIEWPORT_WIDTH * 2;
		const f32 height = FOG_HEIGHT;
		DrawRectangle(x, y, width, height, COLOR_BLACK);
	}
}

void FogDebugDraw(const Scene* scene, const usize entity)
{
	static const u64 dependencies = TAG_POSITION;

	if (!SceneEntityIs(scene, entity, ENTITY_TYPE_FOG)
		|| !SceneEntityHasDependencies(scene, entity, dependencies))
	{
		return;
	}

	const CPosition* position = &scene->components.positions[entity];

	static const i32 padding = 16;

	// Draw the pseudo-bounds of the fog.
	{
		const Rectangle aabb = (Rectangle) {
			.x = position->value.x - CTX_VIEWPORT_WIDTH * 2,
			.y = -padding,
			.width = CTX_VIEWPORT_WIDTH * 2,
			.height = padding + CTX_VIEWPORT_HEIGHT + padding,
		};

		const Color color = P8_PINK;
		const Color premultiplied = ColorMultiply(color, 0.4);

		DrawRectangleRec(aabb, premultiplied);
		DrawRectangleLinesEx(aabb, 4, color);
	}

	// Draw the line of lethalness.
	{
		const Rectangle aabb = (Rectangle) {
			.x = position->value.x - FOG_LETHAL_DISTANCE,
			.y = -padding,
			.width = 1,
			.height = padding + CTX_VIEWPORT_HEIGHT + padding,
		};

		DrawRectangleRec(aabb, P8_RED);
	}
}
