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
    bool grounded;
} CBody;

typedef struct
{
    bool jumping;
} CPlayer;
