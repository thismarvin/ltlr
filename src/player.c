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

    self->kinetic.velocity.x = strafe * 500;

    if (self->grounded && !self->jumping && IsKeyDown(KEY_SPACE)) {
        self->kinetic.velocity.y = -300;
        self->jumping = 1;
        self->grounded = 0;
    }

    if (self->jumping && !IsKeyDown(KEY_SPACE) && self->kinetic.velocity.y < 0) {
        self->kinetic.velocity.y = -50;
        self->jumping = 0;
    }
}

static void PlayerPhysics(Player* self)
{
    KineticIntegrate(&self->kinetic);

    PlayerSetPos(self, self->kinetic.currentPosition);
}

static void PlayerCollision(Player* self)
{
    if (self->kinetic.currentPosition.x > 320) {
        KineticTeleport(&self->kinetic, Vector2Create(-self->aabb.width, self->kinetic.currentPosition.y));
        // PlayerSetPos(self, Vector2Create(-self->aabb.width, self->kinetic.currentPosition.y));
    }

    // Grounded
    if (self->kinetic.currentPosition.y + self->aabb.height > GetScreenHeight()) {
        PlayerSetPos(self, Vector2Create(self->kinetic.currentPosition.x, GetScreenHeight() - self->aabb.height));
        self->kinetic.velocity.y = 0;
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
            self->kinetic.velocity.y = 0;
            self->jumping = 0;
            self->grounded = 1;
        }
    }

    // PolygonDestroy(&playerPolygon);
}

void PlayerInit(Player* self, Vector2 position)
{
    self->kinetic = KineticCreate(position, VECTOR2_ZERO, Vector2Create(0, 1000));
    self->aabb = (Rectangle)
    {
        .x = self->kinetic.currentPosition.x,
        .y = self->kinetic.currentPosition.y,
        .width = PLAYER_WIDTH,
        .height = PLAYER_HEIGHT,
    };
    self->grounded = 0;
    self->jumping = 0;

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
    // TODO(thismarvin): Should this be here...? Shouldn't Kinetic deal with this or...
    self->kinetic.currentPosition = position;

    self->aabb.x = self->kinetic.currentPosition.x;
    self->aabb.y = self->kinetic.currentPosition.y;
}

void PlayerTranslate(Player* self, Vector2 delta)
{
    Vector2 position = Vector2Add(self->kinetic.currentPosition, delta);

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
    Vector2 interpolated = KineticGetRenderPosition(&self->kinetic);

    Rectangle rectangle = (Rectangle) {
        .x = interpolated.x,
        .y = interpolated.y,
        .width = self->aabb.width,
        .height = self->aabb.height,
    };
    DrawRectangleRec(rectangle, RED);

    for (int i = 0; i < 4; ++i) {
        DrawRectangleLinesEx(blocks[i], 4, BLACK);
    }
}
