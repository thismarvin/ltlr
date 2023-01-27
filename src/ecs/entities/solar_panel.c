#include "solar_panel.h"

void SolarPanelBuildHelper(Scene* scene, const SolarPanelBuilder* builder)
{
	const Rectangle intramural = (Rectangle) {
		.x = 4,
		.y = 8,
		.width = 88,
		.height = 40,
	};

	// clang-format off
	scene->components.tags[builder->entity] =
		TAG_NONE
		| TAG_POSITION
		| TAG_DIMENSION
		| TAG_SPRITE
		| TAG_COLLIDER
		| TAG_SOLAR_PANEL;
	// clang-format on

	scene->components.positions[builder->entity] = (CPosition) {
		.value = Vector2Create(builder->x, builder->y),
	};

	scene->components.dimensions[builder->entity] = (CDimension) {
		.width = intramural.width,
		.height = intramural.height,
	};

	scene->components.sprites[builder->entity] = (CSprite) {
		.type = SPRITE_SOLAR_0000,
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
}

void SolarPanelBuild(Scene* scene, const void* params)
{
	SolarPanelBuildHelper(scene, params);
}
