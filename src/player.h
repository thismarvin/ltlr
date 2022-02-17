#pragma once

#include "kinetic.h"
#include "polygon.h"
#include <raylib.h>

#define PLAYER_WIDTH 16.0f
#define PLAYER_HEIGHT 32.0f

typedef struct
{
    Kinetic kinetic;
    Rectangle aabb;
    int grounded;
    int jumping;
} Player;

void PlayerInit(Player* self, Vector2 position);
void PlayerSetPos(Player* self, Vector2 position);
void PlayerTranslate(Player* self, Vector2 delta);
void PlayerUpdate(Player* self);
void PlayerDraw(Player* self);
