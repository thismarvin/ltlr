#pragma once

#include "common.h"

extern const u64 tagNone;
extern const u64 tagPosition;
extern const u64 tagDimension;
extern const u64 tagColor;
extern const u64 tagSprite;
extern const u64 tagKinetic;
extern const u64 tagSmooth;
extern const u64 tagPlayer;
extern const u64 tagCollider;
extern const u64 tagWalker;
extern const u64 tagMortal;
extern const u64 tagDamage;
extern const u64 tagFleeting;

extern const u64 layerNone;
extern const u64 layerUp;
extern const u64 layerRight;
extern const u64 layerDown;
extern const u64 layerLeft;
extern const u64 layerAll;

// TODO(thismarvin): Naming components is hard...

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
} CCollider;

// TODO(thismarvin): Should this be in some sort of Singleton?
typedef struct
{
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
} CMortal;

typedef struct
{
    i16 value;
} CDamage;

typedef struct
{
    f32 age;
    f32 lifetime;
} CFleeting;
