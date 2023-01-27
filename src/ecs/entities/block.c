#include "block.h"

void BlockBuildHelper(Scene* scene, const BlockBuilder* builder)
{
	const Vector2 position = (Vector2) {
		.x = builder->aabb.x,
		.y = builder->aabb.y,
	};

	// clang-format off
	scene->components.tags[builder->entity] =
		TAG_NONE
		| TAG_POSITION
		| TAG_DIMENSION
		| TAG_COLLIDER;
	// clang-format on

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
		.onResolution = OnResolutionNoop,
		.onCollision = OnCollisionNoop,
	};
}

void BlockBuild(Scene* scene, const void* params)
{
	BlockBuildHelper(scene, params);
}
