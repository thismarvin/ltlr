#pragma once

#include "../common.h"

#define TAG_NONE ((u64)0)
#define TAG_POSITION ((u64)1 << 0)
#define TAG_DIMENSION ((u64)1 << 1)
#define TAG_COLOR ((u64)1 << 2)
#define TAG_SPRITE ((u64)1 << 3)
#define TAG_KINETIC ((u64)1 << 4)
#define TAG_SMOOTH ((u64)1 << 5)
#define TAG_PLAYER ((u64)1 << 6)
#define TAG_COLLIDER ((u64)1 << 7)
#define TAG_WALKER ((u64)1 << 8)
#define TAG_MORTAL ((u64)1 << 9)
#define TAG_DAMAGE ((u64)1 << 10)
#define TAG_FLEETING ((u64)1 << 11)

#define LAYER_NONE ((u64)0)
#define LAYER_UP ((u64)1 << 0)
#define LAYER_RIGHT ((u64)1 << 1)
#define LAYER_DOWN ((u64)1 << 2)
#define LAYER_LEFT ((u64)1 << 3)
#define LAYER_ALL (LAYER_UP | LAYER_RIGHT | LAYER_DOWN | LAYER_LEFT)

// TODO(thismarvin): Naming components is hard...

typedef struct Scene Scene;

typedef struct
{
    Scene* scene;
    usize entity;
    usize otherEntity;
} OnDamageParams;

typedef void (*OnDamage)(const OnDamageParams*);

typedef struct
{
    bool xAxisResolved;
    bool yAxisResolved;
} OnCollisionResult;

#define ON_COLLISION_RESULT_NONE (OnCollisionResult) { false, false }

typedef struct
{
    Scene* scene;
    usize entity;
    Rectangle aabb;
    usize otherEntity;
    Rectangle otherAabb;
    Rectangle overlap;
    Vector2 resolution;
} OnCollisionParams;

typedef OnCollisionResult (*OnCollision)(const OnCollisionParams*);

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
    Rectangle source;
    Vector2 offset;
} CSprite;

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
    // Layers you exist on.
    u64 layer;
    // Layers you collide with.
    u64 mask;
    OnCollision onCollision;
} CCollider;

// TODO(thismarvin): Should this be in some sort of Singleton?
typedef struct
{
    bool groundedLastFrame;
    bool grounded;
    f32 coyoteTimer;
    f32 coyoteDuration;
    bool jumping;
    bool dead;
    f32 moveSpeed;
    f32 jumpVelocity;
    f32 jumpGravity;
    f32 defaultGravity;
    f32 invulnerableTimer;
    f32 invulnerableDuration;
} CPlayer;

typedef struct
{
    i16 hp;
    OnDamage onDamage;
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

typedef struct
{
    u64 tag;
    union
    {
        CPosition position;
        CDimension dimension;
        CColor color;
        CSprite sprite;
        CKinetic kinetic;
        CSmooth smooth;
        CCollider collider;
        CPlayer player;
        CMortal mortal;
        CDamage damage;
        CFleeting fleeting;
    };
} Component;

Component ComponentCreateCPosition(CPosition value);
Component ComponentCreateCDimension(CDimension value);
Component ComponentCreateCColor(CColor value);
Component ComponentCreateCSprite(CSprite value);
Component ComponentCreateCKinetic(CKinetic value);
Component ComponentCreateCSmooth(CSmooth value);
Component ComponentCreateCCollider(CCollider value);
Component ComponentCreateCPlayer(CPlayer value);
Component ComponentCreateCMortal(CMortal value);
Component ComponentCreateCDamage(CDamage value);
Component ComponentCreateCFleeting(CFleeting value);
