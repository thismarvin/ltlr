#include "components.h"
#include "context.h"
#include "entities.h"
#include "raymath.h"

usize ECreatePlayer(Scene* scene, const f32 x, const f32 y)
{
    usize entity = SceneAllocateEntity(scene);

    scene->components.tags[entity] =
        tagNone
        | tagPosition
        | tagDimension
        | tagColor
        | tagSprite
        | tagKinetic
        | tagSmooth
        | tagCollider
        | tagPlayer
        | tagMortal;

    Vector2 position = Vector2Create(x, y);

    scene->components.positions[entity] = (CPosition)
    {
        .value = position
    };

    scene->components.dimensions[entity] = (CDimension)
    {
        .width = 15,
        .height = 35
    };

    scene->components.colors[entity] = (CColor)
    {
        .value = (Color)
        {
            255, 255, 255, 255
        }
    };

    scene->components.sprites[entity] = (CSprite)
    {
        .source = (Rectangle) { 16, 0, 32, 48 },
        .offset = Vector2Create(-8, -13),
    };

    scene->components.kinetics[entity] = (CKinetic)
    {
        .velocity = VECTOR2_ZERO,
        .acceleration = Vector2Create(0, 1000)
    };

    scene->components.smooths[entity] = (CSmooth)
    {
        .previous = position
    };

    scene->components.colliders[entity] = (CCollider)
    {
        .layer = layerNone,
        .mask = layerAll,
    };

    scene->components.mortals[entity] = (CMortal)
    {
        .hp = 2
    };

    f32 jumpHeight = 16 * 3 + 6;
    f32 jumpDuration = 0.4;

    f32 jumpGravity = (2 * jumpHeight) / (jumpDuration * jumpDuration);
    f32 defaultGravity = jumpGravity * 1.5;

    f32 jumpVelocity = jumpGravity * jumpDuration;

    f32 coyoteDuration = CTX_DT * 6;
    f32 invulnerableDuration = 1.5f;

    scene->components.players[entity] = (CPlayer)
    {
        .coyoteTimer = coyoteDuration,
        .coyoteDuration = coyoteDuration,
        .moveSpeed = 200,
        .jumping = false,
        .dead = false,
        .jumpVelocity = jumpVelocity,
        .jumpGravity = jumpGravity,
        .defaultGravity = defaultGravity,
        .invulnerableTimer = invulnerableDuration,
        .invulnerableDuration = invulnerableDuration,
    };

    return entity;
}

usize ECreateBlock(Scene* scene, const f32 x, const f32 y, const f32 width, const f32 height)
{
    usize entity = SceneAllocateEntity(scene);

    scene->components.tags[entity] =
        tagNone
        | tagPosition
        | tagDimension
        | tagCollider;

    Vector2 position = Vector2Create(x, y);

    scene->components.positions[entity] = (CPosition)
    {
        .value = position
    };

    scene->components.dimensions[entity] = (CDimension)
    {
        .width = width,
        .height = height
    };

    scene->components.colliders[entity] = (CCollider)
    {
        .layer = layerAll,
        .mask = layerNone,
    };

    return entity;
}

usize ECreateWalker(Scene* scene, const f32 x, const f32 y)
{
    usize entity = SceneAllocateEntity(scene);

    scene->components.tags[entity] =
        tagNone
        | tagPosition
        | tagDimension
        | tagColor
        | tagSprite
        | tagKinetic
        | tagSmooth
        | tagCollider
        | tagWalker
        | tagDamage;

    Vector2 position = Vector2Create(x, y);

    scene->components.positions[entity] = (CPosition)
    {
        .value = position,
    };

    scene->components.dimensions[entity] = (CDimension)
    {
        .width = 16,
        .height = 16,
    };

    scene->components.colors[entity] = (CColor)
    {
        .value = (Color)
        {
            255, 255, 255, 255
        },
    };

    scene->components.sprites[entity] = (CSprite)
    {
        .source = (Rectangle) { 3 * 16, 5 * 16, 16, 16 },
        .offset = VECTOR2_ZERO,
    };

    scene->components.kinetics[entity] = (CKinetic)
    {
        .velocity = Vector2Create(50, 0),
        .acceleration = Vector2Create(0, 1000),
    };

    scene->components.smooths[entity] = (CSmooth)
    {
        .previous = position,
    };

    scene->components.colliders[entity] = (CCollider)
    {
        .layer = layerAll,
        .mask = layerAll,
    };

    scene->components.damages[entity] = (CDamage)
    {
        .value = 1,
    };

    return entity;
}

usize ECreateCloudParticle
(
    Scene* scene,
    const f32 centerX,
    const f32 centerY,
    const Vector2 direction
)
{
    usize entity = SceneAllocateEntity(scene);

    scene->components.tags[entity] =
        tagNone
        | tagPosition
        | tagDimension
        | tagColor
        | tagKinetic
        | tagSmooth
        | tagCollider
        | tagFleeting;

    f32 radius = 1;

    if (GetRandomValue(1, 100) < 25)
    {
        radius = (f32) GetRandomValue(4, 5);
    }
    else
    {
        radius = (f32)GetRandomValue(1, 3);
    }

    Vector2 position = Vector2Create(centerX - radius, centerY - radius);

    scene->components.positions[entity] = (CPosition)
    {
        .value = position,
    };

    scene->components.dimensions[entity] = (CDimension)
    {
        .width = radius * 2,
        .height = radius * 2,
    };

    scene->components.colors[entity] = (CColor)
    {
        .value = (Color)
        {
            255, 255, 255, 255
        }
    };

    f32 speed = (f32)GetRandomValue(5, 15);

    scene->components.kinetics[entity] = (CKinetic)
    {
        .velocity = Vector2Scale(direction, speed),
        .acceleration = Vector2Create(0, 15),
    };

    scene->components.smooths[entity] = (CSmooth)
    {
        .previous = position,
    };

    scene->components.colliders[entity] = (CCollider)
    {
        .layer = layerNone,
        .mask = layerAll,
    };

    f32 lifetime = MIN(1.0f, (f32)GetRandomValue(1, 100) * 0.03f);

    scene->components.fleetings[entity] = (CFleeting)
    {
        .age = 0,
        .lifetime = lifetime,
    };

    return entity;
}
