#include "walker.h"

#include <assert.h>

static OnResolutionResult WalkerOnResolution(const OnResolutionParams* params)
{
	static const u64 dependencies = TAG_KINETIC;
	assert(SceneEntityHasDependencies(params->scene, params->entity, dependencies));

	CKinetic* kinetic = &params->scene->components.kinetics[params->entity];

	// Resolve collision.
	const Rectangle resolvedAabb =
		ApplyResolutionPerfectly(params->aabb, params->otherAabb, params->resolution);

	// Walk side to side.
	{
		if (params->resolution.x != 0)
		{
			kinetic->velocity.x *= -1;
		}

		if (params->resolution.y != 0)
		{
			kinetic->velocity.y = 0;
		}
	}

	return (OnResolutionResult) {
		.aabb = resolvedAabb,
	};
}

void WalkerBuildHelper(Scene* scene, const WalkerBuilder* builder)
{
	const Vector2 position = Vector2Create(builder->x, builder->y);
	const Rectangle intramural = (Rectangle) {
		.x = 14,
		.y = 0,
		.width = 20,
		.height = 16,
	};

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
		| TAG_DAMAGE;
	// clang-format on

	scene->components.identifiers[builder->entity] = (CIdentifier) {
		.type = ENTITY_TYPE_WALKER,
	};

	scene->components.positions[builder->entity] = (CPosition) {
		.value = position,
	};

	scene->components.dimensions[builder->entity] = (CDimension) {
		.width = intramural.width,
		.height = intramural.height,
	};

	// TODO(thismarvin): Add CAnimationFromWalkerIdle() somewhere.
	scene->components.animations[builder->entity] = (CAnimation) {
		.frameTimer = 0,
		.frameDuration = ANIMATION_WALKER_IDLE_FRAME_DURATION,
		.intramural = intramural,
		.reflection = REFLECTION_NONE,
		.frame = 0,
		.length = ANIMATION_WALKER_IDLE_LENGTH,
		.type = ANIMATION_WALKER_IDLE,
	};

	scene->components.kinetics[builder->entity] = (CKinetic) {
		.velocity = Vector2Create(50, 0),
		.acceleration = Vector2Create(0, 1000),
	};

	scene->components.smooths[builder->entity] = (CSmooth) {
		.previous = position,
	};

	scene->components.colliders[builder->entity] = (CCollider) {
		.resolutionSchema = RESOLVE_ALL,
		.layer = LAYER_LETHAL,
		.mask = LAYER_TERRAIN | LAYER_INVISIBLE | LAYER_LETHAL,
		.onResolution = WalkerOnResolution,
		.onCollision = OnCollisionNoop,
	};

	scene->components.damages[builder->entity] = (CDamage) {
		.value = 1,
	};
}

void WalkerBuild(Scene* scene, const void* params)
{
	WalkerBuildHelper(scene, params);
}
