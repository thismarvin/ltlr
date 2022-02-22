#include "components.h"
#include "entities.h"

void ECreatePlayer(Scene* scene, f32 x, f32 y)
{
    usize entity = SceneAllocateEntity(scene);

    Vector2 position = Vector2Create(x, y);

    scene->components.tags[entity] = tagPosition | tagDimension | tagColor | tagSprite |
                                     tagKinetic | tagSmooth | tagCollider | tagPlayer | tagBody;

    scene->components.positions[entity].value = position;
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
        .offset = Vector2Create(-7, -13)
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
        .layer = 1,
        .mask = 1,
    };
    scene->components.bodies[entity] = (CBody)
    {
        .grounded = false
    };
    scene->components.players[entity] = (CPlayer)
    {
        .jumping = false
    };
}

void ECreateBlock(Scene* scene, f32 x, f32 y, f32 width, f32 height)
{
    usize entity = SceneAllocateEntity(scene);

    Vector2 position = Vector2Create(x, y);

    scene->components.tags[entity] = tagPosition | tagDimension | tagColor | tagCollider;

    scene->components.positions[entity].value = position;
    scene->components.dimensions[entity] = (CDimension)
    {
        .width = width,
        .height = height
    };
    scene->components.colors[entity] = (CColor)
    {
        .value = (Color)
        {
            0, 0, 0, 255
        }
    };
    scene->components.colliders[entity] = (CCollider)
    {
        .layer = 1,
        .mask = 0,
    };
    // scene->components.sprites[entity] = (CSprite) {
    //     .source = (Rectangle) { 16, 0, 32, 48 },
    //     .offset = Vector2Create(7, 13)
    // };
}
