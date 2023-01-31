#include "battery.h"

#include <math.h>

static void BatteryBuildHelper(Scene* scene, const BatteryBuilder* builder)
{
	const Vector2 position = (Vector2) {
		.x = builder->x,
		.y = builder->y,
	};
	const Rectangle intramural = (Rectangle) {
		.x = 1,
		.y = 0,
		.width = 14,
		.height = 32,
	};

	// clang-format off
	scene->components.tags[builder->entity] =
		TAG_NONE
		| TAG_IDENTIFIER
		| TAG_POSITION
		| TAG_DIMENSION
		| TAG_SPRITE
		| TAG_COLLIDER
		| TAG_SMOOTH
		| TAG_KINETIC;
	// clang-format on

	scene->components.identifiers[builder->entity] = (CIdentifier) {
		.type = ENTITY_TYPE_BATTERY,
	};

	scene->components.positions[builder->entity] = (CPosition) {
		.value = position,
	};

	scene->components.dimensions[builder->entity] = (CDimension) {
		.width = intramural.width,
		.height = intramural.height,
	};

	scene->components.sprites[builder->entity] = (CSprite) {
		.type = SPRITE_BATTERY,
		.intramural = intramural,
		.reflection = REFLECTION_NONE,
	};

	scene->components.colliders[builder->entity] = (CCollider) {
		.resolutionSchema = RESOLVE_NONE,
		.layer = LAYER_INTERACTABLE,
		.mask = LAYER_NONE,
		.onResolution = OnResolutionNoop,
		.onCollision = OnCollisionNoop,
	};

	scene->components.smooths[builder->entity] = (CSmooth) {
		.previous = position,
	};

	scene->components.kinetics[builder->entity] = (CKinetic) {
		.velocity = VECTOR2_ZERO,
		.acceleration = VECTOR2_ZERO,
	};
}

void BatteryBuild(Scene* scene, const void* params)
{
	BatteryBuildHelper(scene, params);
}

void BatteryUpdate(Scene* scene, const usize entity)
{
	static const u64 dependencies = TAG_KINETIC;

	if (!SceneEntityIs(scene, entity, ENTITY_TYPE_BATTERY)
		|| !SceneEntityHasDependencies(scene, entity, dependencies))
	{
		return;
	}

	CKinetic* kinetic = &scene->components.kinetics[entity];

	kinetic->velocity.y = sinf(ContextGetTotalTime() * 3.0f) * 10.0f;
}
