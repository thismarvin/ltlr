#ifndef PLAYER_H
#define PLAYER_H

#include "polygon.h"
#include <raylib.h>

#define PLAYER_WIDTH 16.0f
#define PLAYER_HEIGHT 32.0f

typedef struct
{
    Rectangle aabb;
    Vector2 position;
    Vector2 velocity;
} Player;

void PlayerInit(Player* self, Vector2 position);
void PlayerSetPos(Player* self, Vector2 position);
void PlayerTranslate(Player* self, Vector2 delta);
void PlayerUpdate(Player* self);
void PlayerDraw(Player* self);

#endif
