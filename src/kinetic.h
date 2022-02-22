#pragma once

#include "raylib.h"

typedef struct
{
    Vector2 previousPosition;
    Vector2 currentPosition;
    Vector2 velocity;
    Vector2 acceleration;
    bool teleported;
} Kinetic;

Kinetic KineticCreate(Vector2 position, Vector2 velocity, Vector2 acceleration);
void KineticIntegrate(Kinetic* self);
void KineticTeleport(Kinetic* self, Vector2 position);
Vector2 KineticGetRenderPosition(Kinetic* self);
