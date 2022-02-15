#include "constants.h"
#include "player.h"
#include "polygon.h"
#include "raymath.h"

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
    Vector2 direction = VECTOR2_ZERO;

    if (IsKeyDown(KEY_UP))
    {
        direction.y = -1;
    }

    if (IsKeyDown(KEY_DOWN))
    {
        direction.y = 1;
    }

    if (IsKeyDown(KEY_LEFT))
    {
        direction.x = -1;
    }

    if (IsKeyDown(KEY_RIGHT))
    {
        direction.x = 1;
    }

    self->velocity = VECTOR2_ZERO;

    if (direction.x != 0 || direction.y != 0)
    {
        self->velocity = Vector2Normalize(direction);
        self->velocity = Vector2Scale(self->velocity, 100 * GetFrameTime());
    }

    PlayerTranslate(self, self->velocity);
}

void PlayerDraw(Player* self)
{
    DrawRectangleRec(self->aabb, RED);
}
