#include "spike.h"

static void SpikeBuildHelper(Scene* scene, const SpikeBuilder* builder)
{
	Vector2 position;
	Rectangle intramural;
	Sprite spriteType;

	switch (builder->rotation)
	{
		default:
		case SPIKE_ROTATE_0: {
			position = Vector2Create(builder->x + 2, builder->y + 13);
			intramural = (Rectangle) { 2, 13, 12, 3 };
			spriteType = SPRITE_SPIKE_0000;
			break;
		}
		case SPIKE_ROTATE_90: {
			position = Vector2Create(builder->x, builder->y + 2);
			intramural = (Rectangle) { 0, 2, 3, 12 };
			spriteType = SPRITE_SPIKE_0001;
			break;
		}
		case SPIKE_ROTATE_180: {
			position = Vector2Create(builder->x + 2, builder->y);
			intramural = (Rectangle) { 2, 0, 12, 3 };
			spriteType = SPRITE_SPIKE_0002;
			break;
		}
		case SPIKE_ROTATE_270: {
			position = Vector2Create(builder->x + 13, builder->y + 2);
			intramural = (Rectangle) { 13, 2, 3, 12 };
			spriteType = SPRITE_SPIKE_0003;
			break;
		}
	}

	// clang-format off
	scene->components.tags[builder->entity] =
		TAG_NONE
		| TAG_IDENTIFIER
		| TAG_POSITION
		| TAG_DIMENSION
		| TAG_COLLIDER
		| TAG_DAMAGE
		| TAG_SPRITE;
	// clang-format on

	scene->components.identifiers[builder->entity] = (CIdentifier) {
		.type = ENTITY_TYPE_SPIKE,
	};

	scene->components.positions[builder->entity] = (CPosition) {
		.value = position,
	};

	scene->components.sprites[builder->entity] = (CSprite) {
		.intramural = intramural,
		.reflection = REFLECTION_NONE,
		.type = spriteType,
	};

	scene->components.dimensions[builder->entity] = (CDimension) {
		.width = intramural.width,
		.height = intramural.height,
	};

	scene->components.colliders[builder->entity] = (CCollider) {
		.resolutionSchema = RESOLVE_NONE,
		.layer = LAYER_LETHAL,
		.mask = LAYER_NONE,
		.onResolution = NULL,
		.onCollision = NULL,
	};

	scene->components.damages[builder->entity] = (CDamage) {
		.value = 1,
	};
}

void SpikeBuild(Scene* scene, const void* params)
{
	SpikeBuildHelper(scene, params);
}
