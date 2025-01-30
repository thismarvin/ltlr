#include "player.h"

#include "../../animation.h"
#include "../../common.h"
#include "../../context.h"
#include "../../palette/p8.h"
#include "../../rng.h"
#include "../../scene.h"
#include "../../sprites_generated.h"
#include "../../utils/arena_allocator.h"
#include "../components.h"
#include "cloud_particle.h"
#include "common.h"

#include <assert.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define PLAYER_SPRITE_INTRAMURAL ((Rectangle) { 24, 29, 15, 35 })
#define COYOTE_DURATION (CTX_DT * 6)
#define INVULNERABLE_DURATION (1.5F)
#define TRAIL_DURATION (CTX_DT * 2)
#define STOMP_STUCK_DURATION (CTX_DT * 6)

typedef struct
{
	usize entity;
	f32 x;
	f32 y;
	Sprite sprite;
	Reflection reflection;
} ShadowBuilder;

static const f32 moveSpeed = 200;
static const f32 jumpHeight = (16 * 3) + 6;
static const f32 jumpDuration = 0.4;
static const f32 jumpGravity = (2 * jumpHeight) / (jumpDuration * jumpDuration);
static const f32 defaultGravity = jumpGravity * 1.5;
static const f32 jumpVelocity = jumpGravity * jumpDuration;
static const f32 terminalVelocity = 500;
static const f32 timeToSprint = CTX_DT * 6;
static const f32 timeToStop = CTX_DT * 6;

static void ShadowBuildHelper(Scene* scene, const ShadowBuilder* builder)
{
	// clang-format off
	scene->components.tags[builder->entity] =
		TAG_NONE
		| TAG_IDENTIFIER
		| TAG_POSITION
		| TAG_COLOR
		| TAG_SPRITE
		| TAG_FLEETING;
	// clang-format on

	scene->components.identifiers[builder->entity] = (CIdentifier) {
		.type = ENTITY_TYPE_PLAYER_SHADOW,
	};

	scene->components.positions[builder->entity] = (CPosition) {
		.value =
			(Vector2) {
				.x = builder->x,
				.y = builder->y,
			},
	};

	scene->components.colors[builder->entity] = (CColor) {
		.value = COLOR_WHITE,
	};

	scene->components.sprites[builder->entity] = (CSprite) {
		.type = builder->sprite,
		.intramural = PLAYER_SPRITE_INTRAMURAL,
		.reflection = builder->reflection,
	};

	scene->components.fleetings[builder->entity] = (CFleeting) {
		.lifetime = CTX_DT * 24,
		.age = 0,
	};
}

void ShadowBuild(Scene* scene, const void* params)
{
	ShadowBuildHelper(scene, params);
}

static void PlayerStandstill(Player* player, CKinetic* kinetic)
{
	player->sprintTimer = 0;
	player->sprintState = PLAYER_SPRINT_STATE_NONE;
	player->sprintForce.x = 0;
	player->sprintDirection = DIR_NONE;
	kinetic->velocity.x = 0;
}

static bool PlayerIsVulnerable(const Player* player)
{
	return player->invulnerableTimer >= INVULNERABLE_DURATION;
}

static Direction PlayerFacing(const Player* player)
{
	if (player->sprintDirection != DIR_NONE)
	{
		return player->sprintDirection;
	}

	return player->initialDirection;
}

static bool PlayerStompInProgress(const Player* player)
{
	return player->stompState == PLAYER_STOMP_STATE_STOMPING
		   || player->stompState == PLAYER_STOMP_STATE_STUCK_IN_GROUND;
}

static void SpawnCloudParticle(
	Scene* scene,
	const Vector2 position,
	const f32 radius,
	const Vector2 velocity,
	const Vector2 acceleration,
	const f32 lifetime
)
{
	CloudParticleBuilder* builder =
		ArenaAllocatorTake(&scene->arenaAllocator, sizeof(CloudParticleBuilder));
	builder->entity = SceneAllocateEntity(scene);
	builder->position = position;
	builder->radius = radius;
	builder->initialVelocity = velocity;
	builder->acceleration = acceleration;
	builder->lifetime = lifetime;
	SceneDefer(scene, CloudParticleBuild, builder);
}

static void SpawnImpactParticles(Scene* scene, const usize entity, const f32 y)
{
	assert(SceneEntityHasDependencies(scene, entity, TAG_POSITION | TAG_DIMENSION | TAG_KINETIC));

	const CPosition* position = &scene->components.positions[entity];
	const CDimension* dimension = &scene->components.dimensions[entity];
	const CKinetic* kinetic = &scene->components.kinetics[entity];

	static const f32 gravity = 9.8F;
	const usize spawnCount = RngNextRange(&scene->rng, 10, 20 + 1);
	const f32 spread = dimension->width * 0.25;
	const Vector2 leftAnchor = (Vector2) {
		.x = position->value.x,
		.y = y,
	};
	const Vector2 rightAnchor = (Vector2) {
		.x = position->value.x + dimension->width,
		.y = y,
	};

	// Lateral pockets.
	{
		static const f32 theta = 25;
		const f32 angleIncrement = (DEG2RAD * theta) / spawnCount;

		for (usize i = 0; i < spawnCount; ++i)
		{
			const f32 radius = RngNextRange(&scene->rng, 1, 4 + 1);
			const f32 offset = RngNextRange(&scene->rng, 0, spread + 1);
			const f32 speed = RngNextRange(&scene->rng, 10, 30 + 1);
			const f32 lifetime = 1 + (0.5 * RngNextRange(&scene->rng, 0, 4 + 1));

			// Left pocket.
			{
				const Vector2 cloudPosition = (Vector2) {
					.x = leftAnchor.x - offset - (radius * 2),
					.y = leftAnchor.y - (radius * 2),
				};
				const f32 rotation = (DEG2RAD * 180) + (angleIncrement * i);
				const Vector2 direction = (Vector2) {
					.x = cosf(rotation),
					.y = sinf(rotation),
				};
				const Vector2 vo = Vector2Scale(direction, speed);
				const Vector2 ao = (Vector2) {
					// a = (vf - vo) / t
					.x = (0 - vo.x) / lifetime,
					.y = gravity,
				};

				SpawnCloudParticle(scene, cloudPosition, radius, vo, ao, lifetime);
			}

			// Right pocket.
			{
				const Vector2 cloudPosition = (Vector2) {
					.x = rightAnchor.x + offset,
					.y = rightAnchor.y - (radius * 2),
				};
				const f32 rotation = 0 - (angleIncrement * i);
				const Vector2 direction = (Vector2) {
					.x = cosf(rotation),
					.y = sinf(rotation),
				};
				const Vector2 vo = Vector2Scale(direction, speed);
				const Vector2 ao = (Vector2) {
					// a = (vf - vo) / t
					.x = (0 - vo.x) / lifetime,
					.y = gravity,
				};

				SpawnCloudParticle(scene, cloudPosition, radius, vo, ao, lifetime);
			}
		}
	}

	if (kinetic->velocity.x == 0)
	{
		return;
	}

	// Spawn extra cloud particles in the direction opposite of velocity.
	{
		static const f32 reflection = 30;
		static const f32 theta = 20;
		const f32 anchorOffset = dimension->width * 0.25;
		const usize total = spawnCount * 0.5;
		const f32 angleIncrement = (DEG2RAD * theta) / total;

		for (usize i = 0; i < total; ++i)
		{
			const f32 radius = RngNextRange(&scene->rng, 1, 3 + 1);
			const f32 offset = RngNextRange(&scene->rng, 0, spread + 1);
			const f32 speed = RngNextRange(&scene->rng, 20, 35 + 1);
			const f32 lifetime = 0.5 + (0.5 * RngNextRange(&scene->rng, 0, 4 + 1));

			if (kinetic->velocity.x > 0)
			{
				const Vector2 cloudPosition = (Vector2) {
					.x = leftAnchor.x + anchorOffset - offset - (radius * 2),
					.y = leftAnchor.y - (radius * 2),
				};
				const f32 rotation =
					(DEG2RAD * (180 + reflection - (theta * 0.5))) + (angleIncrement * i);
				const Vector2 direction = (Vector2) {
					.x = cosf(rotation),
					.y = sinf(rotation),
				};
				const Vector2 vo = Vector2Scale(direction, speed);
				const Vector2 ao = (Vector2) {
					// a = (vf - vo) / t
					.x = (0 - vo.x) / lifetime,
					.y = gravity,
				};

				SpawnCloudParticle(scene, cloudPosition, radius, vo, ao, lifetime);
			}
			else
			{
				const Vector2 cloudPosition = (Vector2) {
					.x = rightAnchor.x - anchorOffset + offset,
					.y = rightAnchor.y - (radius * 2),
				};
				const f32 rotation =
					(DEG2RAD * (0 - reflection + (theta * 0.5))) - (angleIncrement * i);
				const Vector2 direction = (Vector2) {
					.x = cosf(rotation),
					.y = sinf(rotation),
				};
				const Vector2 vo = Vector2Scale(direction, speed);
				const Vector2 ao = (Vector2) {
					// a = (vf - vo) / t
					.x = (0 - vo.x) / lifetime,
					.y = gravity,
				};

				SpawnCloudParticle(scene, cloudPosition, radius, vo, ao, lifetime);
			}
		}
	}
}

static void SpawnJumpParticles(Scene* scene, const usize entity)
{
	assert(SceneEntityHasDependencies(scene, entity, TAG_POSITION | TAG_DIMENSION | TAG_KINETIC));

	const CPosition* position = &scene->components.positions[entity];
	const CDimension* dimension = &scene->components.dimensions[entity];
	const CKinetic* kinetic = &scene->components.kinetics[entity];

	static const f32 gravity = 9.8F;
	const usize spawnCount = RngNextRange(&scene->rng, 10, 30 + 1);
	const Vector2 anchor = (Vector2) {
		.x = position->value.x + (dimension->width * 0.5),
		.y = position->value.y + dimension->height,
	};

	// Lateral pockets.
	{
		static const f32 theta = 30;
		const f32 angleIncrement = (DEG2RAD * theta) / spawnCount;

		for (usize i = 0; i < spawnCount; ++i)
		{
			const f32 radius = RngNextRange(&scene->rng, 1, 3 + 1);
			const f32 speed = RngNextRange(&scene->rng, 10, 15 + 1);
			const f32 lifetime = 0.5 + (0.5 * RngNextRange(&scene->rng, 0, 3 + 1));

			// Left pocket.
			{
				const Vector2 cloudPosition = (Vector2) {
					.x = anchor.x - (radius * 2),
					.y = anchor.y - (radius * 2),
				};
				const f32 rotation = (DEG2RAD * 180) + (angleIncrement * i);
				const Vector2 direction = (Vector2) {
					.x = cosf(rotation),
					.y = sinf(rotation),
				};
				const Vector2 vo = Vector2Scale(direction, speed);
				const Vector2 ao = (Vector2) {
					// a = (vf - vo) / t
					.x = (0 - vo.x) / lifetime,
					.y = gravity,
				};

				SpawnCloudParticle(scene, cloudPosition, radius, vo, ao, lifetime);
			}

			// Right pocket.
			{
				const Vector2 cloudPosition = (Vector2) {
					.x = anchor.x,
					.y = anchor.y - (radius * 2),
				};
				const f32 rotation = 0 - (angleIncrement * i);
				const Vector2 direction = (Vector2) {
					.x = cosf(rotation),
					.y = sinf(rotation),
				};
				const Vector2 vo = Vector2Scale(direction, speed);
				const Vector2 ao = (Vector2) {
					// a = (vf - vo) / t
					.x = (0 - vo.x) / lifetime,
					.y = gravity,
				};

				SpawnCloudParticle(scene, cloudPosition, radius, vo, ao, lifetime);
			}
		}
	}

	// Middle pocket.
	{
		static const f32 reflection = 20;
		static const f32 theta = 20;
		static const f32 lateralMultiplier = 1.75;

		const i8 sign = SIGN(kinetic->velocity.x);
		f32 angle = -90;

		if (sign < 0)
		{
			angle = -reflection;
		}
		else if (sign > 0)
		{
			angle = -180 + reflection;
		}

		angle -= theta * 0.5;
		angle *= DEG2RAD;

		const usize total = spawnCount * 0.5;
		const f32 angleIncrement = (DEG2RAD * theta) / total;

		for (usize i = 0; i < total; ++i)
		{
			const f32 radius = RngNextRange(&scene->rng, 2, 3 + 1);
			const f32 lifetime = 0.5 + (0.5 * RngNextRange(&scene->rng, 0, 4 + 1));

			const Vector2 cloudPosition = (Vector2) {
				.x = anchor.x - radius,
				.y = anchor.y - (radius * 2),
			};
			const f32 rotation = angle + (angleIncrement * i);
			const Vector2 directionTmp = (Vector2) {
				.x = cosf(rotation),
				.y = sinf(rotation),
			};

			f32 speed = RngNextRange(&scene->rng, 10, 15 + 1);

			if (kinetic->velocity.x != 0)
			{
				speed *= lateralMultiplier;
			}

			const Vector2 vo = Vector2Scale(directionTmp, speed);
			const Vector2 ao = (Vector2) {
				// a = (vf - vo) / t
				.x = (0 - vo.x) / lifetime,
				.y = gravity,
			};

			SpawnCloudParticle(scene, cloudPosition, radius, vo, ao, lifetime);
		}
	}
}

static void PlayerOnDamage(Scene* scene, const usize entity, const usize otherEntity)
{
	assert(SceneEntityHasDependencies(scene, entity, TAG_PLAYER | TAG_MORTAL));
	assert(SceneEntityHasDependencies(scene, otherEntity, TAG_DAMAGE));

	const u8 handle = scene->components.players[entity].handle;
	Player* player = &scene->players[handle];
	CMortal* mortal = &scene->components.mortals[entity];

	const CDamage* otherDamage = &scene->components.damages[otherEntity];

	if (!PlayerIsVulnerable(player))
	{
		return;
	}

	mortal->hp -= otherDamage->value;
	player->invulnerableTimer = 0;
}

static OnResolutionResult PlayerOnResolution(const OnResolutionParams* params)
{
	assert(SceneEntityHasDependencies(params->scene, params->entity, TAG_PLAYER | TAG_KINETIC));

	const u8 handle = params->scene->components.players[params->entity].handle;
	Player* player = &params->scene->players[handle];
	CKinetic* kinetic = &params->scene->components.kinetics[params->entity];

	// Collision specific logic that will not resolve the player.
	{
		if (SceneEntityIs(params->scene, params->otherEntity, ENTITY_TYPE_WALKER))
		{
			return (OnResolutionResult) {
				.aabb = params->aabb,
			};
		}
	}

	// Collision leeway.
	{
		// Check if the player hit its head on the bottom of a collider.
		if (params->resolution.y > 0 && fabsf(params->overlap.width) <= 4)
		{
			Rectangle resolvedAabb = params->aabb;

			if (params->aabb.x < params->otherAabb.x)
			{
				resolvedAabb.x = RectangleLeft(params->otherAabb) - params->aabb.width;
			}
			else
			{
				resolvedAabb.x = RectangleRight(params->otherAabb);
			}

			return (OnResolutionResult) {
				.aabb = resolvedAabb,
			};
		}
	}

	// Make sure that landing on top of a one-way platform does not noticeably teleport the player.
	{
		if (kinetic->velocity.y > 0 && params->resolution.y < 0)
		{
			const f32 difference =
				fabsf(RectangleTop(params->otherAabb) - RectangleBottom(params->aabb));

			if (difference > 3.0F)
			{
				return (OnResolutionResult) {
					.aabb = params->aabb,
				};
			}
		}
	}

	// Resolve collision.
	const Rectangle resolvedAabb =
		ApplyResolutionPerfectly(params->aabb, params->otherAabb, params->resolution);

	// Resolution specific player logic.
	{
		if ((params->resolution.x < 0 && kinetic->velocity.x > 0)
			|| (params->resolution.x > 0 && kinetic->velocity.x < 0))
		{
			PlayerStandstill(player, kinetic);
		}

		if (params->resolution.y < 0)
		{
			player->grounded = true;

			if (kinetic->velocity.y > terminalVelocity * 0.75)
			{
				SpawnImpactParticles(
					params->scene,
					params->entity,
					RectangleTop(params->otherAabb)
				);
			}
		}

		if ((params->resolution.y < 0 && kinetic->velocity.y > 0)
			|| (params->resolution.y > 0 && kinetic->velocity.y < 0))
		{
			kinetic->velocity.y = 0;
		}
	}

	return (OnResolutionResult) {
		.aabb = resolvedAabb,
	};
}

static Rectangle PlayerGetFeetCollider(const Rectangle aabb)
{
	static const f32 height = 4;

	return (Rectangle) {
		.x = aabb.x,
		.y = RectangleBottom(aabb) - height,
		.width = aabb.width,
		.height = height,
	};
}

static void PlayerOnCollision(const OnCollisionParams* params)
{
	assert(SceneEntityHasDependencies(
		params->scene,
		params->entity,
		TAG_PLAYER | TAG_KINETIC | TAG_MORTAL
	));

	assert(SceneEntityHasDependencies(params->scene, params->otherEntity, TAG_IDENTIFIER));

	const u8 handle = params->scene->components.players[params->entity].handle;
	Player* player = &params->scene->players[handle];
	const CKinetic* kinetic = &params->scene->components.kinetics[params->entity];
	CMortal* mortal = &params->scene->components.mortals[params->entity];

	const CIdentifier* otherIdentifier =
		&params->scene->components.identifiers[params->otherEntity];

	if (SceneEntityHasDependencies(params->scene, params->otherEntity, TAG_DAMAGE))
	{
		switch (otherIdentifier->type)
		{
			case ENTITY_TYPE_SPIKE: {
				assert(SceneEntityHasDependencies(params->scene, params->otherEntity, TAG_SPRITE));

				const bool theSpikeIsOnTheGround =
					params->scene->components.sprites[params->otherEntity].type
					== SPRITE_SPIKE_0000;

				if (theSpikeIsOnTheGround)
				{
					const bool thePlayerIsFallingOrGrounded = kinetic->velocity.y >= 0;

					if (thePlayerIsFallingOrGrounded)
					{
						const Rectangle collider = PlayerGetFeetCollider(params->aabb);

						if (CheckCollisionRecs(collider, params->otherAabb))
						{
							PlayerOnDamage(params->scene, params->entity, params->otherEntity);
						}
					}

					break;
				}

				PlayerOnDamage(params->scene, params->entity, params->otherEntity);

				break;
			}
			case ENTITY_TYPE_WALKER: {
				if (player->stompState == PLAYER_STOMP_STATE_STOMPING)
				{
					SceneIncrementScore(params->scene, 50);
					SceneDeferDeallocateEntity(params->scene, params->otherEntity);

					break;
				}

				PlayerOnDamage(params->scene, params->entity, params->otherEntity);

				break;
			}
			default: {
				PlayerOnDamage(params->scene, params->entity, params->otherEntity);
			}
		}
	}

	switch (otherIdentifier->type)
	{
		case ENTITY_TYPE_BATTERY: {
			// TODO(thismarvin): Add a static PlayerIncrementHealth method?
			mortal->hp += 1;
			mortal->hp = MIN(mortal->hp, PLAYER_MAX_HIT_POINTS);

			SceneIncrementScore(params->scene, 100);

			SceneCollectBattery(params->scene);
			SceneDeferDeallocateEntity(params->scene, params->otherEntity);

			break;
		}
		case ENTITY_TYPE_SOLAR_PANEL: {
			assert(SceneEntityHasDependencies(params->scene, params->otherEntity, TAG_SPRITE));

			CSprite* otherSprite = &params->scene->components.sprites[params->otherEntity];

			const bool solarPanelIsOff = otherSprite->type == SPRITE_SOLAR_0000;
			const bool batteryAvailable = params->scene->totalBatteries != 0;

			if (solarPanelIsOff && batteryAvailable)
			{
				otherSprite->type = SPRITE_SOLAR_0001;

				SceneConsumeBattery(params->scene);

				// TODO(austin0209): Spawn fireworks.
			}

			break;
		}
		default: {
			break;
		}
	}
}

static void PlayerBuildHelper(Scene* scene, const PlayerBuilder* builder)
{
	const Vector2 position = Vector2Create(builder->x, builder->y);
	const Rectangle intramural = PLAYER_SPRITE_INTRAMURAL;

	// clang-format off
	scene->components.tags[builder->entity] =
		TAG_NONE
		| TAG_IDENTIFIER
		| TAG_POSITION
		| TAG_DIMENSION
		| TAG_ANIMATION
		| TAG_KINETIC
		| TAG_SMOOTH
		| TAG_COLLIDER
		| TAG_PLAYER
		| TAG_MORTAL;
	// clang-format on

	scene->components.identifiers[builder->entity] = (CIdentifier) {
		.type = ENTITY_TYPE_PLAYER,
	};

	scene->components.positions[builder->entity] = (CPosition) {
		.value = position,
	};

	scene->components.dimensions[builder->entity] = (CDimension) {
		.width = intramural.width,
		.height = intramural.height,
	};

	scene->components.animations[builder->entity] = (CAnimation) {
		.frameTimer = 0,
		.frameDuration = CTX_DT,
		.intramural = intramural,
		.reflection = REFLECTION_NONE,
		.type = ANIMATION_PLAYER_SPIN,
		.frame = 0,
		.length = ANIMATION_PLAYER_SPIN_LENGTH,
	};

	scene->components.kinetics[builder->entity] = (CKinetic) {
		.velocity = VECTOR2_ZERO,
		.acceleration = VECTOR2_ZERO,
	};

	scene->components.smooths[builder->entity] = (CSmooth) {
		.previous = position,
	};

	scene->components.colliders[builder->entity] = (CCollider) {
		.resolutionSchema = RESOLVE_NONE,
		.layer = LAYER_NONE,
		.mask = LAYER_TERRAIN | LAYER_LETHAL | LAYER_INTERACTABLE,
		.onResolution = PlayerOnResolution,
		.onCollision = PlayerOnCollision,
	};

	scene->components.mortals[builder->entity] = (CMortal) {
		.hp = 2,
	};

	scene->components.players[builder->entity] = (CPlayer) {
		.handle = builder->handle,
	};

	scene->players[builder->handle] = (Player) {
		.gravityForce = VECTOR2_ZERO,
		.groundedLastFrame = false,
		.grounded = false,
		.jumping = false,
		.coyoteTimer = 0,
		.coyoteTimeActive = false,
		.dead = false,
		.invulnerableTimer = INVULNERABLE_DURATION,
		.sprintTimer = 0,
		.sprintDuration = 0,
		.initialDirection = DIR_NONE,
		.sprintDirection = DIR_NONE,
		.sprintState = PLAYER_SPRINT_STATE_NONE,
		.sprintForce = VECTOR2_ZERO,
		.animationState = PLAYER_ANIMATION_STATE_STILL,
		.stompTimer = 0,
		.stompState = PLAYER_STOMP_STATE_STOMPING,
		.stompForce = VECTOR2_ZERO,
		.trailTimer = 0,
		.velocityLastFrame = 0,
	};
}

void PlayerBuild(Scene* scene, const void* params)
{
	PlayerBuildHelper(scene, params);
}

static void PlayerDecelerate(Player* player, const CKinetic* kinetic)
{
	if (kinetic->velocity.x == 0)
	{
		player->sprintTimer = 0;
		player->sprintState = PLAYER_SPRINT_STATE_NONE;
		player->sprintForce.x = 0;
		player->sprintDirection = DIR_NONE;

		return;
	}

	const f32 delta = fabsf(kinetic->velocity.x);

	static const f32 vf = 0;
	const f32 vo = kinetic->velocity.x;
	const f32 t = delta * timeToStop / moveSpeed;

	const f32 adjustedTime = ceilf(t / CTX_DT) * CTX_DT;

	player->sprintTimer = 0;
	player->sprintDuration = adjustedTime;
	player->sprintState = PLAYER_SPRINT_STATE_DECELERATING;
	// vf = vo + a * t
	// a = (vf - vo) / t
	player->sprintForce.x = (vf - vo) / adjustedTime;
	player->sprintDirection = DIR_NONE;
}

static void PlayerAccelerate(Player* player, const CKinetic* kinetic, const Direction direction)
{
	if ((direction == DIR_LEFT && kinetic->velocity.x <= -moveSpeed)
		|| (direction == DIR_RIGHT && kinetic->velocity.x >= moveSpeed))
	{
		player->sprintTimer = 0;
		player->sprintState = PLAYER_SPRINT_STATE_TERMINAL;
		player->sprintForce.x = 0;
		player->sprintDirection = direction;

		return;
	}

	f32 delta = moveSpeed;

	if ((direction == DIR_LEFT && kinetic->velocity.x < 0)
		|| (direction == DIR_RIGHT && kinetic->velocity.x > 0))
	{
		delta -= fabsf(kinetic->velocity.x);
	}
	else if ((direction == DIR_LEFT && kinetic->velocity.x > 0)
			 || (direction == DIR_RIGHT && kinetic->velocity.x < 0))
	{
		delta += fabsf(kinetic->velocity.x);
	}

	const i8 sign = direction == DIR_LEFT ? -1 : 1;

	const f32 vf = moveSpeed * sign;
	const f32 vo = kinetic->velocity.x;
	const f32 t = delta * timeToSprint / moveSpeed;

	const f32 adjustedTime = ceilf(t / CTX_DT) * CTX_DT;

	player->sprintTimer = 0;
	player->sprintDuration = adjustedTime;
	player->sprintState = PLAYER_SPRINT_STATE_ACCELERATING;
	// vf = vo + a * t
	// a = (vf - vo) / t
	player->sprintForce.x = (vf - vo) / adjustedTime;
	player->sprintDirection = direction;
}

static void PlayerLateralMovementLogic(Scene* scene, const usize entity)
{
	assert(SceneEntityHasDependencies(scene, entity, TAG_PLAYER | TAG_KINETIC));

	const u8 handle = scene->components.players[entity].handle;
	Player* player = &scene->players[handle];
	CKinetic* kinetic = &scene->components.kinetics[entity];

	Direction strafe = DIR_NONE;

	// Handle input.
	{
		if (!ScenePressing(scene, handle, INPUT_BINDING_RIGHT)
			&& ScenePressing(scene, handle, INPUT_BINDING_LEFT))
		{
			player->initialDirection = DIR_LEFT;
			strafe = DIR_LEFT;
		}
		else if (!ScenePressing(scene, handle, INPUT_BINDING_LEFT)
				 && ScenePressing(scene, handle, INPUT_BINDING_RIGHT))
		{
			player->initialDirection = DIR_RIGHT;
			strafe = DIR_RIGHT;
		}
		else if (player->initialDirection == DIR_RIGHT
				 && ScenePressing(scene, handle, INPUT_BINDING_LEFT))
		{
			strafe = DIR_LEFT;
		}
		else if (player->initialDirection == DIR_LEFT
				 && ScenePressing(scene, handle, INPUT_BINDING_RIGHT))
		{
			strafe = DIR_RIGHT;
		}
	}

	// Disable lateral movement while the player is stomping.
	if (PlayerStompInProgress(player))
	{
		strafe = DIR_NONE;
	}

	// Handle sprint state.
	switch (player->sprintState)
	{
		case PLAYER_SPRINT_STATE_NONE: {
			if (strafe == DIR_NONE)
			{
				player->sprintForce.x = 0;

				break;
			}

			PlayerAccelerate(player, kinetic, strafe);

			break;
		}
		case PLAYER_SPRINT_STATE_ACCELERATING: {
			if (strafe == DIR_NONE)
			{
				PlayerDecelerate(player, kinetic);

				break;
			}

			if ((player->sprintDirection == DIR_LEFT && strafe == DIR_RIGHT)
				|| (player->sprintDirection == DIR_RIGHT && strafe == DIR_LEFT))
			{
				PlayerAccelerate(player, kinetic, strafe);

				break;
			}

			player->sprintTimer += CTX_DT;

			if (player->sprintTimer >= player->sprintDuration)
			{
				player->sprintTimer = 0;
				player->sprintState = PLAYER_SPRINT_STATE_TERMINAL;
				player->sprintForce.x = 0;
			}

			break;
		}
		case PLAYER_SPRINT_STATE_TERMINAL: {
			if ((player->sprintDirection == DIR_LEFT && strafe == DIR_RIGHT)
				|| (player->sprintDirection == DIR_RIGHT && strafe == DIR_LEFT))
			{
				PlayerAccelerate(player, kinetic, strafe);

				break;
			}

			if (strafe == DIR_NONE)
			{
				PlayerDecelerate(player, kinetic);
			}

			break;
		}
		case PLAYER_SPRINT_STATE_DECELERATING: {
			if (strafe != DIR_NONE)
			{
				PlayerAccelerate(player, kinetic, strafe);

				break;
			}

			player->sprintTimer += CTX_DT;

			if (player->sprintTimer >= player->sprintDuration)
			{
				PlayerStandstill(player, kinetic);
			}

			break;
		}
	}
}

static void PlayerJumpLogic(Scene* scene, const usize entity)
{
	assert(SceneEntityHasDependencies(scene, entity, TAG_PLAYER | TAG_KINETIC));

	const u8 handle = scene->components.players[entity].handle;
	Player* player = &scene->players[handle];
	CKinetic* kinetic = &scene->components.kinetics[entity];

	if (PlayerStompInProgress(player))
	{
		return;
	}

	if ((player->grounded || player->coyoteTimeActive) && !player->jumping
		&& ScenePressed(scene, handle, INPUT_BINDING_JUMP))
	{
		SceneConsume(scene, handle, INPUT_BINDING_JUMP);

		player->grounded = false;
		player->jumping = true;

		kinetic->velocity.y = -jumpVelocity;

		if (!player->coyoteTimeActive)
		{
			SpawnJumpParticles(scene, entity);
		}

		player->coyoteTimeActive = false;
	}

	// Variable Jump Height.
	if (SceneReleased(scene, handle, INPUT_BINDING_JUMP) && kinetic->velocity.y < 0)
	{
		SceneConsume(scene, handle, INPUT_BINDING_JUMP);

		player->jumping = false;
		kinetic->velocity.y = MAX(kinetic->velocity.y, -jumpVelocity * 0.5);
	}
}

static void PlayerStompLogic(Scene* scene, const usize entity)
{
	assert(SceneEntityHasDependencies(scene, entity, TAG_PLAYER | TAG_KINETIC));

	const u8 handle = scene->components.players[entity].handle;
	Player* player = &scene->players[handle];
	CKinetic* kinetic = &scene->components.kinetics[entity];

	static f32 stompAcceleration = 2048;

	switch (player->stompState)
	{
		case PLAYER_STOMP_STATE_NONE: {
			if (!player->grounded && ScenePressed(scene, handle, INPUT_BINDING_STOMP))
			{
				player->stompState = PLAYER_STOMP_STATE_STOMPING;
				player->stompForce.y = stompAcceleration;
			}

			break;
		}
		case PLAYER_STOMP_STATE_STOMPING: {
			if (player->groundedLastFrame)
			{
				player->stompState = PLAYER_STOMP_STATE_STUCK_IN_GROUND;
				player->stompForce.y = 0;

				break;
			}

			player->velocityLastFrame = kinetic->velocity.y;

			break;
		}
		case PLAYER_STOMP_STATE_STUCK_IN_GROUND: {
			player->stompTimer += CTX_DT;

			if (player->stompTimer >= STOMP_STUCK_DURATION)
			{
				player->stompState = PLAYER_STOMP_STATE_SPRINGING;
				player->stompTimer = 0;

				f32 maxVelocity = -jumpVelocity;

				if (ScenePressing(scene, handle, INPUT_BINDING_STOMP))
				{
					maxVelocity *= 1.5;
				}

				player->grounded = false;
				player->jumping = true;
				kinetic->velocity.y = -player->velocityLastFrame * 0.5;
				kinetic->velocity.y = MAX(kinetic->velocity.y, maxVelocity);
			}

			break;
		}
		case PLAYER_STOMP_STATE_SPRINGING: {
			if (!player->groundedLastFrame && ScenePressed(scene, handle, INPUT_BINDING_STOMP))
			{
				player->stompState = PLAYER_STOMP_STATE_STOMPING;
				player->stompForce.y = stompAcceleration;
			}

			if (player->groundedLastFrame)
			{
				player->stompState = PLAYER_STOMP_STATE_NONE;
			}

			break;
		}
	}
}

void PlayerInputUpdate(Scene* scene, const usize entity)
{
	// TODO(thismarvin): Should grounded logic really be in Input?

	static const u64 dependencies = TAG_PLAYER | TAG_KINETIC;

	if (!SceneEntityIs(scene, entity, ENTITY_TYPE_PLAYER)
		|| !SceneEntityHasDependencies(scene, entity, dependencies))
	{
		return;
	}

	if (scene->advanceStageRequested)
	{
		return;
	}

	const u8 handle = scene->components.players[entity].handle;
	Player* player = &scene->players[handle];
	CKinetic* kinetic = &scene->components.kinetics[entity];

	if (player->dead)
	{
		return;
	}

	// Maintenance.
	{
		player->groundedLastFrame = player->grounded;

		if (player->grounded)
		{
			kinetic->velocity.y = 0;
			player->jumping = false;
		}

		player->gravityForce.y = defaultGravity;

		if (player->jumping && kinetic->velocity.y < jumpVelocity)
		{
			player->gravityForce.y = jumpGravity;
		}

		if (player->coyoteTimer >= COYOTE_DURATION)
		{
			player->coyoteTimeActive = false;
		}

		if (player->coyoteTimeActive)
		{
			player->coyoteTimer += CTX_DT;
		}
	}

	PlayerLateralMovementLogic(scene, entity);
	PlayerJumpLogic(scene, entity);
	PlayerStompLogic(scene, entity);

	// Assume that the player is not grounded; prove that they are later.
	player->grounded = false;

	// Calculate Net Force.
	{
		kinetic->acceleration = (Vector2) {
			.x = player->gravityForce.x + player->sprintForce.x + player->stompForce.x,
			.y = player->gravityForce.y + player->sprintForce.y + player->stompForce.y,
		};
	}
}

void PlayerPostCollisionUpdate(Scene* scene, const usize entity)
{
	static const u64 dependencies = TAG_PLAYER | TAG_POSITION | TAG_KINETIC;

	if (!SceneEntityIs(scene, entity, ENTITY_TYPE_PLAYER)
		|| !SceneEntityHasDependencies(scene, entity, dependencies))
	{
		return;
	}

	const u8 handle = scene->components.players[entity].handle;
	Player* player = &scene->players[handle];
	CPosition* position = &scene->components.positions[entity];
	CKinetic* kinetic = &scene->components.kinetics[entity];

	// General purpose player specific collision logic.
	{
		// Do not let the player walk-off the leftmost side of the stage.
		if (position->value.x < scene->bounds.x)
		{
			position->value.x = scene->bounds.x;
			PlayerStandstill(player, kinetic);
		}

		// Force the player right if they finish the stage.
		if (position->value.x > RectangleRight(scene->bounds))
		{
			kinetic->velocity.x = moveSpeed;
		}

		// Once the player is fully offscreen, start to transition to the next stage.
		if (position->value.x > RectangleRight(scene->bounds) + 16)
		{
			SceneDeferAdvanceStage(scene);
		}

		// Freeze the player in-place as the stage transitions to the next.
		if (scene->advanceStageRequested)
		{
			kinetic->acceleration = VECTOR2_ZERO;
			kinetic->velocity = VECTOR2_ZERO;
		}
	}

	// Enable "Coyote Time" if the player walked off an edge.
	if (player->groundedLastFrame && !player->grounded && !player->jumping)
	{
		player->coyoteTimeActive = true;
		player->coyoteTimer = 0;
	}
}

static void PlayerFlashingLogic(Scene* scene, const usize entity)
{
	const u8 handle = scene->components.players[entity].handle;
	Player* player = &scene->players[handle];

	player->invulnerableTimer += CTX_DT;

	static const u32 totalFlashes = 5;
	const f32 timeSlice = INVULNERABLE_DURATION / (totalFlashes * 2.0F);

	if (!player->dead && !PlayerIsVulnerable(player))
	{
		const u32 passedSlices = (u32)(player->invulnerableTimer / timeSlice);

		if (passedSlices % 2 == 0)
		{
			SceneDeferDisableTag(scene, entity, TAG_ANIMATION);
		}
		else
		{
			SceneDeferEnableTag(scene, entity, TAG_ANIMATION);
		}
	}
	else
	{
		// This is a pre-caution to make sure the last state isn't off.
		SceneDeferEnableTag(scene, entity, TAG_ANIMATION);
	}
}

void PlayerMortalUpdate(Scene* scene, const usize entity)
{
	static const u64 dependencies = TAG_PLAYER | TAG_MORTAL | TAG_POSITION | TAG_KINETIC;

	if (!SceneEntityIs(scene, entity, ENTITY_TYPE_PLAYER)
		|| !SceneEntityHasDependencies(scene, entity, dependencies))
	{
		return;
	}

	const u8 handle = scene->components.players[entity].handle;
	Player* player = &scene->players[handle];
	const CMortal* mortal = &scene->components.mortals[entity];
	const CPosition* position = &scene->components.positions[entity];
	CKinetic* kinetic = &scene->components.kinetics[entity];

	// TODO(thismarvin): This might need to be more involved in the future...
	if (position->value.y > CTX_VIEWPORT_HEIGHT * 2)
	{
		SceneDeferReset(scene);

		return;
	}

	if (player->dead)
	{
		return;
	}

	// Deal with the player falling out-of-bounds.
	if (position->value.y > scene->bounds.height)
	{
		player->dead = true;

		SceneDeferDisableTag(scene, entity, TAG_COLLIDER);

		kinetic->velocity.x = 0;
		kinetic->acceleration.x = 0;

		return;
	}

	// Deal with the player literally dying.
	if (mortal->hp <= 0)
	{
		player->dead = true;
		player->stompState = PLAYER_STOMP_STATE_NONE;

		SceneDeferEnableTag(scene, entity, TAG_ANIMATION);
		SceneDeferDisableTag(scene, entity, TAG_COLLIDER);

		if (kinetic->velocity.y >= 0)
		{
			kinetic->velocity.y = -jumpVelocity * 0.85;
		}

		kinetic->acceleration.x = 0;
		kinetic->acceleration.y = defaultGravity;

		return;
	}

	PlayerFlashingLogic(scene, entity);
}

static bool IsFrameJustStarting(const CAnimation* animation)
{
	return animation->frameTimer == 0;
}

static void EnableAnimation(Scene* scene, usize entity, Player* player, Animation animation)
{
	CAnimation contents;

	switch (animation)
	{
		case ANIMATION_PLAYER_STILL: {
			contents = (CAnimation) {
				.frameTimer = 0,
				.frameDuration = ANIMATION_PLAYER_STILL_FRAME_DURATION,
				.intramural = PLAYER_SPRITE_INTRAMURAL,
				.reflection = REFLECTION_NONE,
				.frame = 0,
				.length = ANIMATION_PLAYER_STILL_LENGTH,
				.type = ANIMATION_PLAYER_STILL,
			};

			player->animationState = PLAYER_ANIMATION_STATE_STILL;

			break;
		}
		case ANIMATION_PLAYER_RUN: {
			contents = (CAnimation) {
				.frameTimer = 0,
				.frameDuration = ANIMATION_PLAYER_RUN_FRAME_DURATION,
				.intramural = PLAYER_SPRITE_INTRAMURAL,
				.reflection = REFLECTION_NONE,
				.frame = 0,
				.length = ANIMATION_PLAYER_RUN_LENGTH,
				.type = ANIMATION_PLAYER_RUN,
			};

			player->animationState = PLAYER_ANIMATION_STATE_RUNNING;

			break;
		}
		case ANIMATION_PLAYER_JUMP: {
			contents = (CAnimation) {
				.frameTimer = 0,
				.frameDuration = ANIMATION_PLAYER_JUMP_FRAME_DURATION,
				.intramural = PLAYER_SPRITE_INTRAMURAL,
				.reflection = REFLECTION_NONE,
				.frame = 0,
				.length = ANIMATION_PLAYER_JUMP_LENGTH,
				.type = ANIMATION_PLAYER_JUMP,
			};

			player->animationState = PLAYER_ANIMATION_STATE_JUMPING;

			break;
		}
		case ANIMATION_PLAYER_SPIN: {
			contents = (CAnimation) {
				.frameTimer = 0,
				.frameDuration = ANIMATION_PLAYER_SPIN_FRAME_DURATION,
				.intramural = PLAYER_SPRITE_INTRAMURAL,
				.reflection = REFLECTION_NONE,
				.frame = 0,
				.length = ANIMATION_PLAYER_SPIN_LENGTH,
				.type = ANIMATION_PLAYER_SPIN,
			};

			player->animationState = PLAYER_ANIMATION_STATE_SPINNING;

			break;
		}
		default: {
			fprintf(stderr, "Unsupported Animation type.\n");
			exit(EXIT_FAILURE);

			break;
		}
	}

	scene->components.animations[entity] = contents;
}

void PlayerAnimationUpdate(Scene* scene, const usize entity)
{
	static const u64 dependencies = TAG_PLAYER;

	if (!SceneEntityIs(scene, entity, ENTITY_TYPE_PLAYER)
		|| !SceneEntityHasDependencies(scene, entity, dependencies))
	{
		return;
	}

	const u8 handle = scene->components.players[entity].handle;
	Player* player = &scene->players[handle];
	CAnimation* animation = &scene->components.animations[entity];

	// Deal with death.
	{
		if (player->dead && player->animationState != PLAYER_ANIMATION_STATE_DYING)
		{
			EnableAnimation(scene, entity, player, ANIMATION_PLAYER_SPIN);

			// This is a little hacky, but I don't want to add a bespoke death animation!
			animation->frameDuration = ANIMATION_PLAYER_SPIN_FRAME_DURATION * 0.5;
			player->animationState = PLAYER_ANIMATION_STATE_DYING;
		}
	}

	switch (player->animationState)
	{
		case PLAYER_ANIMATION_STATE_STILL: {
			if (player->jumping)
			{
				EnableAnimation(scene, entity, player, ANIMATION_PLAYER_JUMP);

				break;
			}

			if (player->sprintState != PLAYER_SPRINT_STATE_NONE)
			{
				EnableAnimation(scene, entity, player, ANIMATION_PLAYER_RUN);

				break;
			}

			// TODO(thismarvin): Implement an idle animation?

			break;
		}
		case PLAYER_ANIMATION_STATE_RUNNING: {
			if (player->jumping)
			{
				EnableAnimation(scene, entity, player, ANIMATION_PLAYER_JUMP);

				break;
			}

			if (player->sprintState == PLAYER_SPRINT_STATE_NONE)
			{
				if (IsFrameJustStarting(animation))
				{
					EnableAnimation(scene, entity, player, ANIMATION_PLAYER_STILL);
				}

				break;
			}

			break;
		}
		case PLAYER_ANIMATION_STATE_JUMPING: {
			if (player->grounded)
			{
				if (player->sprintDirection != DIR_NONE)
				{
					if (IsFrameJustStarting(animation))
					{
						EnableAnimation(scene, entity, player, ANIMATION_PLAYER_RUN);
					}

					break;
				}

				EnableAnimation(scene, entity, player, ANIMATION_PLAYER_STILL);

				break;
			}

			if (player->stompState == PLAYER_STOMP_STATE_STOMPING)
			{
				EnableAnimation(scene, entity, player, ANIMATION_PLAYER_SPIN);
				animation->frameDuration = CTX_DT * 8;
				player->animationState = PLAYER_ANIMATION_STATE_STOMPING;
			}

			break;
		}
		case PLAYER_ANIMATION_STATE_STOMPING: {
			// Speed up the animation as time goes by;
			animation->frameDuration -= CTX_DT;
			animation->frameDuration = MAX(animation->frameDuration, 0);

			if (player->stompState == PLAYER_STOMP_STATE_STUCK_IN_GROUND)
			{
				EnableAnimation(scene, entity, player, ANIMATION_PLAYER_STILL);
				player->animationState = PLAYER_ANIMATION_STATE_RECOVERING;
			}

			break;
		}
		case PLAYER_ANIMATION_STATE_RECOVERING: {
			if (player->stompState == PLAYER_STOMP_STATE_SPRINGING)
			{
				EnableAnimation(scene, entity, player, ANIMATION_PLAYER_STILL);
			}

			break;
		}
		default: {
			break;
		}
	}

	// Animation reflection logic.
	{
		const Direction facing = PlayerFacing(player);
		const Reflection reflection =
			facing == DIR_LEFT ? REFLECTION_REVERSE_X_AXIS : REFLECTION_NONE;

		animation->reflection = reflection;
	}
}

void PlayerTrailUpdate(Scene* scene, const usize entity)
{
	static const u64 dependencies = TAG_PLAYER | TAG_SMOOTH;

	if (!SceneEntityIs(scene, entity, ENTITY_TYPE_PLAYER)
		|| !SceneEntityHasDependencies(scene, entity, dependencies))
	{
		return;
	}

	const u8 handle = scene->components.players[entity].handle;
	Player* player = &scene->players[handle];
	const CSmooth* smooth = &scene->components.smooths[entity];

	player->trailTimer += CTX_DT;

	if (player->trailTimer >= TRAIL_DURATION)
	{
		// TODO(thismarvin): Should the trail care about the player flashing?
		if (player->stompState == PLAYER_STOMP_STATE_STOMPING
			&& SceneEntityHasDependencies(scene, entity, TAG_ANIMATION))
		{
			const CAnimation* animation = &scene->components.animations[entity];

			ShadowBuilder* builder =
				ArenaAllocatorTake(&scene->arenaAllocator, sizeof(ShadowBuilder));
			builder->entity = SceneAllocateEntity(scene);
			builder->x = smooth->previous.x;
			builder->y = smooth->previous.y;
			builder->sprite = ANIMATIONS[animation->type][animation->frame];
			builder->reflection = animation->reflection;
			SceneDefer(scene, ShadowBuild, builder);
		}

		player->trailTimer = 0;
	}
}

void PlayerShadowUpdate(Scene* scene, const usize entity)
{
	static const u64 dependencies = TAG_FLEETING | TAG_COLOR;

	if (!SceneEntityIs(scene, entity, ENTITY_TYPE_PLAYER_SHADOW)
		|| !SceneEntityHasDependencies(scene, entity, dependencies))
	{
		return;
	}

	const CFleeting* fleeting = &scene->components.fleetings[entity];
	CColor* color = &scene->components.colors[entity];

	const f32 value = 1.0 - (fleeting->age / fleeting->lifetime);

	const Color tint = (Color) {
		.r = 255 * value,
		.g = 255 * value,
		.b = 255 * value,
		.a = 255,
	};

	const u8 r = tint.r * value;
	const u8 b = tint.g * value;
	const u8 g = tint.b * value;
	const u8 a = tint.a * value;

	color->value = (Color) { r, g, b, a };
}

void PlayerDebugDraw(const Scene* scene, const usize entity)
{
	static const u64 dependencies = TAG_PLAYER | TAG_POSITION | TAG_DIMENSION;

	if (!SceneEntityIs(scene, entity, ENTITY_TYPE_PLAYER)
		|| !SceneEntityHasDependencies(scene, entity, dependencies))
	{
		return;
	}

	const CPosition* position = &scene->components.positions[entity];
	const CDimension* dimension = &scene->components.dimensions[entity];

	const Rectangle aabb = (Rectangle) {
		.x = position->value.x,
		.y = position->value.y,
		.width = dimension->width,
		.height = dimension->height,
	};

	// Draw general-purpose aabb.
	{
		const Color color = P8_RED;
		const Color premultiplied = ColorMultiply(color, 0.4);

		DrawRectangleRec(aabb, premultiplied);
		DrawRectangleLinesEx(aabb, 2, color);
	}

	// Draw smaller feet aabb.
	{
		const Rectangle collider = PlayerGetFeetCollider(aabb);

		const Color color = P8_PINK;
		const Color premultiplied = ColorMultiply(color, 0.8);

		DrawRectangleRec(collider, premultiplied);
		DrawRectangleLinesEx(collider, 1, color);
	}
}
