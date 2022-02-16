#include "collider.h"
#include "constants.h"
#include "context.h"
#include "player.h"
#include "polygon.h"
#include "raymath.h"
#include "sat.h"

static Rectangle blocks[4];

static void PlayerInput(Player* self)
{
    float strafe = 0;

    if (IsKeyDown(KEY_LEFT))
    {
        strafe = -1;
    }

    if (IsKeyDown(KEY_RIGHT))
    {
        strafe = 1;
    }

    self->velocity.x = strafe * 500;

    if (self->grounded && !self->jumping && IsKeyDown(KEY_SPACE)) {
        self->velocity.y = -300;
        self->jumping = 1;
        self->grounded = 0;
    }

    if (self->jumping && !IsKeyDown(KEY_SPACE) && self->velocity.y < 0) {
        self->velocity.y = -50;
        self->jumping = 0;
    }
}

static void PlayerPhysics(Player* self)
{
    // Semi-Implict Euler Integration
    self->velocity.x += self->acceleration.x * CTX_DT;
    self->velocity.y += self->acceleration.y * CTX_DT;

    self->position.x += self->velocity.x * CTX_DT;
    self->position.y += self->velocity.y * CTX_DT;

    PlayerSetPos(self, self->position);
}

static void PlayerCollision(Player* self)
{
    // Grounded
    if (self->position.y + self->aabb.height > GetScreenHeight()) {
        PlayerSetPos(self, Vector2Create(self->position.x, GetScreenHeight() - self->aabb.height));
        self->velocity.y = 0;
        self->jumping = 0;
        self->grounded = 1;
    }

    // TODO(thismarvin): Support arbitrary convex polygons!
    // Polygon playerPolygon = PolygonFromRectangle(self->aabb);

    for (int i = 0; i < 4; ++i)
    {
        Vector2 resolution = RectangleRectangleResolution(self->aabb, blocks[i]);

        PlayerTranslate(self, resolution);

        if (resolution.y < 0) {
            self->velocity.y = 0;
            self->jumping = 0;
            self->grounded = 1;
        }
    }

    // PolygonDestroy(&playerPolygon);
}

void PlayerInit(Player* self, Vector2 position)
{
    self->position = position;
    self->aabb = (Rectangle)
    {
        .x = self->position.x,
        .y = self->position.y,
        .width = PLAYER_WIDTH,
        .height = PLAYER_HEIGHT,
    };
    self->velocity = VECTOR2_ZERO;
    self->acceleration = Vector2Create(0, 1000);
    self->jumping = 0;
    self->grounded = 0;

    blocks[0] = (Rectangle) {
        200, 180 - 120, 64, 32
    };
    blocks[1] = (Rectangle) {
        200, 180 - 32 - 4, 32, 32
    };
    blocks[2] = (Rectangle) {
        200 + 64, 180 - 32 - 4, 32, 32
    };
}

void PlayerSetPos(Player* self, Vector2 position)
{
    self->position = position;

    self->aabb.x = self->position.x;
    self->aabb.y = self->position.y;
}

void PlayerTranslate(Player* self, Vector2 delta)
{
    Vector2 position = Vector2Add(self->position, delta);

    PlayerSetPos(self, position);
}

void PlayerUpdate(Player* self)
{
    PlayerInput(self);
    PlayerPhysics(self);
    PlayerCollision(self);
}

void PlayerDraw(Player* self)
{
    DrawRectangleRec(self->aabb, RED);

    for (int i = 0; i < 4; ++i) {
        DrawRectangleLinesEx(blocks[i], 4, BLACK);
    }
}
