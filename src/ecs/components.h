#pragma once

#include "../animation.h"
#include "../common.h"
#include "../sprites_generated.h"

#define TAG_NONE ((u64)0)
#define TAG_IDENTIFIER ((u64)1 << 0)
#define TAG_POSITION ((u64)1 << 1)
#define TAG_DIMENSION ((u64)1 << 2)
#define TAG_COLOR ((u64)1 << 3)
#define TAG_SPRITE ((u64)1 << 4)
#define TAG_ANIMATION ((u64)1 << 5)
#define TAG_KINETIC ((u64)1 << 6)
#define TAG_SMOOTH ((u64)1 << 7)
#define TAG_COLLIDER ((u64)1 << 8)
#define TAG_MORTAL ((u64)1 << 9)
#define TAG_DAMAGE ((u64)1 << 10)
#define TAG_FLEETING ((u64)1 << 11)
#define TAG_PLAYER ((u64)1 << 12)

#define RESOLVE_NONE ((u8)0)
#define RESOLVE_UP ((u8)1 << 0)
#define RESOLVE_RIGHT ((u8)1 << 1)
#define RESOLVE_DOWN ((u8)1 << 2)
#define RESOLVE_LEFT ((u8)1 << 3)
#define RESOLVE_ALL (RESOLVE_UP | RESOLVE_RIGHT | RESOLVE_DOWN | RESOLVE_LEFT)

#define LAYER_NONE ((u64)0)
#define LAYER_TERRAIN ((u64)1 << 0)
#define LAYER_LETHAL ((u64)1 << 1)
#define LAYER_INTERACTABLE ((u64)1 << 2)
#define LAYER_INVISIBLE ((u64)1 << 3)

typedef struct Scene Scene;

typedef enum
{
	ENTITY_TYPE_NONE,
	ENTITY_TYPE_BATTERY,
	ENTITY_TYPE_BLOCK,
	ENTITY_TYPE_CLOUD_PARTICLE,
	ENTITY_TYPE_FOG,
	ENTITY_TYPE_FOG_PARTICLE,
	ENTITY_TYPE_LAKITU,
	ENTITY_TYPE_PLAYER,
	ENTITY_TYPE_SOLAR_PANEL,
	ENTITY_TYPE_SPIKE,
	ENTITY_TYPE_WALKER,
} EntityType;

typedef enum
{
	PLAYER_SPRINT_STATE_NONE,
	PLAYER_SPRINT_STATE_ACCELERATING,
	PLAYER_SPRINT_STATE_TERMINAL,
	PLAYER_SPRINT_STATE_DECELERATING,
} PlayerSprintState;

typedef enum
{
	PLAYER_ANIMATION_STATE_STILL,
	PLAYER_ANIMATION_STATE_RUNNING,
	PLAYER_ANIMATION_STATE_JUMPING,
	PLAYER_ANIMATION_STATE_SPINNING,
	PLAYER_ANIMATION_STATE_DYING,
} PlayerAnimationState;

typedef struct
{
	Scene* scene;
	usize entity;
	Rectangle aabb;
	usize otherEntity;
	Rectangle otherAabb;
	Rectangle overlap;
} OnCollisionParams;

typedef void (*OnCollision)(const OnCollisionParams*);

typedef struct
{
	Rectangle aabb;
} OnResolutionResult;

typedef struct
{
	Scene* scene;
	usize entity;
	Rectangle aabb;
	usize otherEntity;
	Rectangle otherAabb;
	Rectangle overlap;
	Vector2 resolution;
} OnResolutionParams;

typedef OnResolutionResult (*OnResolution)(const OnResolutionParams*);

typedef struct
{
	u8 type;
} CIdentifier;

typedef struct
{
	Vector2 value;
} CPosition;

typedef struct
{
	f32 width;
	f32 height;
} CDimension;

typedef struct
{
	Color value;
} CColor;

typedef struct
{
	Rectangle intramural;
	Reflection reflection;
	Sprite type;
} CSprite;

typedef struct
{
	f32 frameTimer;
	f32 frameDuration;
	Rectangle intramural;
	Reflection reflection;
	Animation type;
	u16 frame;
	u16 length;
} CAnimation;

typedef struct
{
	Vector2 velocity;
	Vector2 acceleration;
} CKinetic;

typedef struct
{
	Vector2 previous;
} CSmooth;

typedef struct
{
	// The directions other entities will resolve against.
	u8 resolutionSchema;
	// The layer you exist on.
	u64 layer;
	// The layers you collide with.
	u64 mask;
	// The first pass of the collision system uses this callback as a strategy to resolve the
	// current entity's aabb with other colliders.
	OnResolution onResolution;
	// The second pass of the collision system uses this callback to compare the resolved aabb of
	// the current entity against other colliders.
	OnCollision onCollision;
} CCollider;

typedef struct
{
	i16 hp;
} CMortal;

typedef struct
{
	i16 value;
} CDamage;

typedef struct
{
	f32 lifetime;
	f32 age;
} CFleeting;

// TODO(thismarvin): Should this be in some sort of Singleton?
typedef struct
{
	bool groundedLastFrame;
	bool grounded;
	f32 coyoteTimer;
	f32 coyoteDuration;
	bool jumping;
	bool dead;
	Vector2 gravityForce;
	f32 invulnerableTimer;
	f32 invulnerableDuration;
	Direction initialDirection;
	Direction sprintDirection;
	PlayerSprintState sprintState;
	f32 sprintTimer;
	f32 sprintDuration;
	Vector2 sprintForce;
	PlayerAnimationState animationState;
} CPlayer;
