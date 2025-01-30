#include "cloud_particle.h"

#include "../../common.h"
#include "../../context.h"
#include "../../scene.h"
#include "../components.h"

#include <assert.h>
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

static void CloudParticleOnCollision(const OnCollisionParams* params)
{
	// If the aabb is completely within another collider then remove it.
	if (params->overlap.width >= params->aabb.width
		&& params->overlap.height >= params->aabb.height)
	{
		SceneDeferDeallocateEntity(params->scene, params->entity);
	}
}

static void CloudParticleBuildHelper(Scene* scene, const CloudParticleBuilder* builder)
{
	// clang-format off
	scene->components.tags[builder->entity] =
		TAG_NONE
		| TAG_IDENTIFIER
		| TAG_POSITION
		| TAG_DIMENSION
		| TAG_KINETIC
		| TAG_SMOOTH
		| TAG_COLLIDER
		| TAG_FLEETING;
	// clang-format on

	scene->components.identifiers[builder->entity] = (CIdentifier) {
		.type = ENTITY_TYPE_CLOUD_PARTICLE,
	};

	scene->components.positions[builder->entity] = (CPosition) {
		.value = builder->position,
	};

	scene->components.dimensions[builder->entity] = (CDimension) {
		.width = builder->radius * 2,
		.height = builder->radius * 2,
	};

	scene->components.kinetics[builder->entity] = (CKinetic) {
		.velocity = builder->initialVelocity,
		.acceleration = builder->acceleration,
	};

	scene->components.smooths[builder->entity] = (CSmooth) {
		.previous = builder->position,
	};

	scene->components.colliders[builder->entity] = (CCollider) {
		.resolutionSchema = RESOLVE_NONE,
		.layer = LAYER_NONE,
		.mask = LAYER_TERRAIN,
		.onResolution = NULL,
		.onCollision = CloudParticleOnCollision,
	};

	scene->components.fleetings[builder->entity] = (CFleeting) {
		.lifetime = builder->lifetime,
		.age = 0,
	};
}

void CloudParticleBuild(Scene* scene, const void* params)
{
	CloudParticleBuildHelper(scene, params);
}

void CloudParticleDraw(const Scene* scene, const usize entity)
{
	// clang-format off
	static const u64 dependencies =
		TAG_NONE
		| TAG_POSITION
		| TAG_DIMENSION
		| TAG_FLEETING
		| TAG_SMOOTH;
	// clang-format on

	if (!SceneEntityIs(scene, entity, ENTITY_TYPE_CLOUD_PARTICLE)
		|| !SceneEntityHasDependencies(scene, entity, dependencies))
	{
		return;
	}

	const CPosition* position = &scene->components.positions[entity];
	const CFleeting* fleeting = &scene->components.fleetings[entity];
	const CDimension* dimension = &scene->components.dimensions[entity];
	const CSmooth* smooth = &scene->components.smooths[entity];

	const f32 drawSize =
		dimension->width * (fleeting->lifetime - fleeting->age) / fleeting->lifetime;

	const Vector2 interpolated = Vector2Lerp(smooth->previous, position->value, ContextGetAlpha());

	const Vector2 center = (Vector2) {
		.x = interpolated.x + (dimension->width * 0.5F),
		.y = interpolated.y + (dimension->height * 0.5F),
	};

	DrawPoly(center, 6, drawSize * 0.5F, 0, COLOR_WHITE);
}
