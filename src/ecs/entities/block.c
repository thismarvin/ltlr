#include "block.h"

#include "../../scene.h"
#include "../components.h"

#include <raylib.h>
#include <stdlib.h>

static void BlockBuildHelper(Scene* scene, const BlockBuilder* builder)
{
	const Vector2 position = (Vector2) {
		.x = builder->aabb.x,
		.y = builder->aabb.y,
	};

	// clang-format off
	scene->components.tags[builder->entity] =
		TAG_NONE
		| TAG_IDENTIFIER
		| TAG_POSITION
		| TAG_DIMENSION
		| TAG_COLLIDER;
	// clang-format on

	scene->components.identifiers[builder->entity] = (CIdentifier) {
		.type = ENTITY_TYPE_BLOCK,
	};

	scene->components.positions[builder->entity] = (CPosition) {
		.value = position,
	};

	scene->components.dimensions[builder->entity] = (CDimension) {
		.width = builder->aabb.width,
		.height = builder->aabb.height,
	};

	scene->components.colliders[builder->entity] = (CCollider) {
		.resolutionSchema = builder->resolutionSchema,
		.layer = builder->layer,
		.mask = LAYER_NONE,
		.onResolution = NULL,
		.onCollision = NULL,
	};
}

void BlockBuild(Scene* scene, const void* params)
{
	BlockBuildHelper(scene, params);
}
