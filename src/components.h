#pragma once

#include "common.h"

extern const u64 tagPosition;
extern const u64 tagDimension;
extern const u64 tagColor;
extern const u64 tagSprite;
extern const u64 tagKinetic;
extern const u64 tagSmooth;
extern const u64 tagPlayer;
extern const u64 tagCollider;
extern const u64 tagBody;
extern const u64 tagWalker;
extern const u64 tagMortal;
extern const u64 tagDamage;

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
    // Layer you exist on.
    u64 layer;
    // Layer you collide with.
    u64 mask;
} CCollider;

// TODO(thismarvin): Work on the name...
typedef struct
{
    // The most recent resolution against a collider.
    Vector2 resolution;
} CBody;

// TODO(thismarvin): Should this be in some sort of Singleton?
typedef struct
{
    bool jumping;
    f32 moveSpeed;
    f32 jumpVelocity;
    f32 jumpGravity;
    f32 defaultGravity;
} CPlayer;

typedef struct
{
    i16 hp;
} CMortal;

typedef struct
{
    i16 value;
} CDamage;