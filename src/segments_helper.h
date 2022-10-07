#pragma once

#define CREATE_SOLID_BLOCK(mX, mY, mWidth, mHeight) \
{ \
    const Rectangle aabb = (Rectangle) \
    { \
        .x = mX + offset.x, \
        .y = mY + offset.y, \
        .width = mWidth, \
        .height = mHeight, \
    }; \
    DEQUE_PUSH_BACK(&entities, EntityBuilder, BlockCreate(aabb, RESOLVE_ALL, LAYER_TERRAIN)); \
}

#define CREATE_ONE_WAY_BLOCK(mX, mY, mWidth, mHeight) \
{ \
    const Rectangle aabb = (Rectangle) \
    { \
        .x = mX + offset.x, \
        .y = mY + offset.y, \
        .width = mWidth, \
        .height = mHeight, \
    }; \
    DEQUE_PUSH_BACK(&entities, EntityBuilder, BlockCreate(aabb, RESOLVE_UP, LAYER_TERRAIN)); \
}

#define CREATE_INVISIBLE_BLOCK(mX, mY, mWidth, mHeight) \
{ \
    const Rectangle aabb = (Rectangle) \
    { \
        .x = mX + offset.x, \
        .y = mY + offset.y, \
        .width = mWidth, \
        .height = mHeight, \
    }; \
    DEQUE_PUSH_BACK(&entities, EntityBuilder, BlockCreate(aabb, RESOLVE_ALL, LAYER_INVISIBLE)); \
}

#define CREATE_SPIKE(mX, mY, mRotation) \
{ \
    const f32 x = mX + offset.x; \
    const f32 y = mY + offset.y; \
    DEQUE_PUSH_BACK(&entities, EntityBuilder, SpikeCreate(x, y, mRotation)); \
}

#define CREATE_WALKER(mX, mY) \
{ \
    const f32 x = mX + offset.x; \
    const f32 y = mY + offset.y; \
    DEQUE_PUSH_BACK(&entities, EntityBuilder, WalkerCreate(x, y)); \
}

#define CREATE_BATTERY(mX, mY) \
{ \
    const f32 x = mX + offset.x; \
    const f32 y = mY + offset.y; \
    DEQUE_PUSH_BACK(&entities, EntityBuilder, BatteryCreate(x, y)); \
}

#define CREATE_SOLAR_PANEL(mX, mY) \
{ \
    const f32 x = mX + offset.x - 36; \
    const f32 y = mY + offset.y - 24; \
    DEQUE_PUSH_BACK(&entities, EntityBuilder, SolarPanelCreate(x, y)); \
}

#define DRAW_SPRITE(mSprite, mX, mY) \
{ \
    const AtlasDrawParams params = (AtlasDrawParams) \
    { \
        .sprite = mSprite, \
        .position = Vector2Create(mX + offset.x, mY + offset.y), \
        .intramural = (Rectangle) { 0, 0, 0, 0 }, \
        .reflection = REFLECTION_NONE, \
        .tint = COLOR_WHITE, \
    }; \
    AtlasDraw(atlas, &params); \
}

#define DRAW_COVER_UP(mX, mY, mColor) \
{ \
    DrawRectangle(mX + offset.x, mY + offset.y, 4, 4, mColor); \
}
