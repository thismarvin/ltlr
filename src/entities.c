#include "components.h"
#include "context.h"
#include "entities.h"
#include "raymath.h"

#define ADD_COMPONENT(mType, mValue) DEQUE_PUSH_FRONT(&components, Component, ComponentCreate##mType(mValue))

Deque ECreatePlayer(const f32 x, const f32 y)
{
    Deque components = DEQUE_OF(Component);

    Vector2 position = Vector2Create(x, y);

    ADD_COMPONENT(CPosition, ((CPosition)
    {
        .value = position,
    }));

    ADD_COMPONENT(CDimension, ((CDimension)
    {
        .width = 15,
        .height = 35,
    }));

    ADD_COMPONENT(CColor, ((CColor)
    {
        .value = COLOR_WHITE,
    }));

    ADD_COMPONENT(CSprite, ((CSprite)
    {
        .source = (Rectangle) { 16, 0, 32, 48 },
        .offset = Vector2Create(-8, -13),
    }));

    ADD_COMPONENT(CKinetic, ((CKinetic)
    {
        .velocity = VECTOR2_ZERO,
        .acceleration = Vector2Create(0, 1000),
    }));

    ADD_COMPONENT(CSmooth, ((CSmooth)
    {
        .previous = position,
    }));

    ADD_COMPONENT(CCollider, ((CCollider)
    {
        .layer = layerNone,
        .mask = layerAll,
    }));

    ADD_COMPONENT(CMortal, ((CMortal)
    {
        .hp = 2,
    }));

    f32 jumpHeight = 16 * 3 + 6;
    f32 jumpDuration = 0.4;

    f32 jumpGravity = (2 * jumpHeight) / (jumpDuration * jumpDuration);
    f32 defaultGravity = jumpGravity * 1.5;

    f32 jumpVelocity = jumpGravity * jumpDuration;

    f32 coyoteDuration = CTX_DT * 6;
    f32 invulnerableDuration = 1.5f;

    ADD_COMPONENT(CPlayer, ((CPlayer)
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
    }));

    return components;
}

Deque ECreateBlock(const f32 x, const f32 y, const f32 width, const f32 height)
{
    Deque components = DEQUE_OF(Component);

    Vector2 position = Vector2Create(x, y);

    ADD_COMPONENT(CPosition, ((CPosition)
    {
        .value = position,
    }));

    ADD_COMPONENT(CDimension, ((CDimension)
    {
        .width = width,
        .height = height,
    }));

    ADD_COMPONENT(CCollider, ((CCollider)
    {
        .layer = layerAll,
        .mask = layerNone,
    }));

    return components;
}

Deque ECreateWalker(const f32 x, const f32 y)
{
    Deque components = DEQUE_OF(Component);

    Vector2 position = Vector2Create(x, y);

    ADD_COMPONENT(CWalker, ((CWalker)
    {
        .unused = 0,
    }));

    ADD_COMPONENT(CPosition, ((CPosition)
    {
        .value = position,
    }));

    ADD_COMPONENT(CDimension, ((CDimension)
    {
        .width = 16,
        .height = 16,
    }));

    ADD_COMPONENT(CColor, ((CColor)
    {
        .value = COLOR_WHITE,
    }));

    ADD_COMPONENT(CSprite, ((CSprite)
    {
        .source = (Rectangle) { 3 * 16, 5 * 16, 16, 16 },
        .offset = VECTOR2_ZERO,
    }));

    ADD_COMPONENT(CKinetic, ((CKinetic)
    {
        .velocity = Vector2Create(50, 0),
        .acceleration = Vector2Create(0, 1000),
    }));

    ADD_COMPONENT(CSmooth, ((CSmooth)
    {
        .previous = position,
    }));

    ADD_COMPONENT(CCollider, ((CCollider)
    {
        .layer = layerAll,
        .mask = layerAll,
    }));

    ADD_COMPONENT(CDamage, ((CDamage)
    {
        .value = 1,
    }));

    return components;
}

Deque ECreateCloudParticle
(
    const f32 centerX,
    const f32 centerY,
    const Vector2 direction
)
{
    Deque components = DEQUE_OF(Component);

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

    ADD_COMPONENT(CPosition, ((CPosition)
    {
        .value = position,
    }));

    ADD_COMPONENT(CDimension, ((CDimension)
    {
        .width = radius * 2,
        .height = radius * 2,
    }));

    ADD_COMPONENT(CColor, ((CColor)
    {
        .value = COLOR_WHITE,
    }));

    f32 speed = (f32)GetRandomValue(5, 15);

    ADD_COMPONENT(CKinetic, ((CKinetic)
    {
        .velocity = Vector2Scale(direction, speed),
        .acceleration = Vector2Create(0, 15),
    }));

    ADD_COMPONENT(CSmooth, ((CSmooth)
    {
        .previous = position,
    }));

    ADD_COMPONENT(CCollider, ((CCollider)
    {
        .layer = layerNone,
        .mask = layerAll,
    }));

    f32 lifetime = MIN(1.0f, (f32)GetRandomValue(1, 100) * 0.03f);

    ADD_COMPONENT(CFleeting, ((CFleeting)
    {
        .lifetime = lifetime,
        .age = 0,
    }));

    return components;
}
