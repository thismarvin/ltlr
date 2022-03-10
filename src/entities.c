#include "components.h"
#include "entities.h"

usize ECreatePlayer(Scene* scene, f32 x, f32 y)
{
    usize entity = SceneAllocateEntity(scene);

    Vector2 position = Vector2Create(x, y);

    scene->components.tags[entity] = tagPosition | tagDimension | tagColor | tagSprite |
                                     tagKinetic | tagSmooth | tagCollider | tagPlayer | tagMortal;

    scene->components.positions[entity].value = position;
    scene->components.mortals[entity].hp = 10;
    scene->components.dimensions[entity] = (CDimension)
    {
        .width = 16,
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
        .offset = Vector2Create(-7, -13),
        .enabled = true,
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

    {
        f32 jumpHeight = 16 * 3 + 6;
        f32 jumpDuration = 0.4;

        f32 jumpGravity = (2 * jumpHeight) / (jumpDuration * jumpDuration);
        f32 defaultGravity = jumpGravity * 1.5;

        f32 jumpVelocity = jumpGravity * jumpDuration;

        scene->components.players[entity] = (CPlayer)
        {
            .moveSpeed = 200,
            .jumping = false,
            .dead = false,
            .jumpVelocity = jumpVelocity,
            .jumpGravity = jumpGravity,
            .defaultGravity = defaultGravity,
            .invulnerableTimer = 3,
            .invulnerableDuration = 3,
        };
    }

    return entity;
}

usize ECreateBlock(Scene* scene, f32 x, f32 y, f32 width, f32 height)
{
    usize entity = SceneAllocateEntity(scene);

    Vector2 position = Vector2Create(x, y);

    scene->components.tags[entity] = tagPosition | tagDimension | tagCollider;

    scene->components.positions[entity].value = position;
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

usize ECreateWalker(Scene* scene, f32 x, f32 y)
{
    usize entity = SceneAllocateEntity(scene);

    Vector2 position = Vector2Create(x, y);

    scene->components.tags[entity] = tagPosition | tagDimension | tagColor | tagSprite |
                                     tagKinetic | tagSmooth | tagCollider | tagWalker | tagDamage;

    scene->components.damages[entity].value = 1;
    scene->components.positions[entity].value = position;
    scene->components.dimensions[entity] = (CDimension)
    {
        .width = 16,
        .height = 16
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
        .source = (Rectangle) { 3 * 16, 5 * 16, 16, 16 },
        .offset = VECTOR2_ZERO,
        .enabled = true,
    };
    scene->components.kinetics[entity] = (CKinetic)
    {
        .velocity = Vector2Create(50, 0),
        .acceleration = Vector2Create(0, 1000)
    };
    scene->components.smooths[entity] = (CSmooth)
    {
        .previous = position
    };
    scene->components.colliders[entity] = (CCollider)
    {
        .layer = layerAll,
        .mask = layerAll,
    };

    return entity;
}
