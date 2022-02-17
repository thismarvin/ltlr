#include "context.h"
#include "kinetic.h"
#include "raymath.h"

Kinetic KineticCreate(Vector2 position, Vector2 velocity, Vector2 acceleration)
{
    return (Kinetic)
    {
        .previousPosition = position,
        .currentPosition = position,
        .velocity = velocity,
        .acceleration = acceleration,
        .teleported = false,
    };
}

void KineticIntegrate(Kinetic* self)
{
    self->teleported = false;
    self->previousPosition = self->currentPosition;

    // Semi-Implict Euler Integration
    self->velocity.x += self->acceleration.x * CTX_DT;
    self->velocity.y += self->acceleration.y * CTX_DT;

    self->currentPosition.x += self->velocity.x * CTX_DT;
    self->currentPosition.y += self->velocity.y * CTX_DT;
}

void KineticTeleport(Kinetic* self, Vector2 position)
{
    self->currentPosition = position;
    self->teleported = true;
}

Vector2 KineticGetRenderPosition(Kinetic* self)
{
    if (self->teleported)
    {
        return self->currentPosition;
    }

    return Vector2Lerp(self->previousPosition, self->currentPosition, ContextGetAlpha());
}
