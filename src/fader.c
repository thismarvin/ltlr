#include "context.h"
#include "fader.h"
#include <raymath.h>

Fader FaderCreate(const Color color, const f32 duration)
{
    return (Fader)
    {
        .duration = duration,
        .timer = 0.0,
        .previous = 0.0,
        .current = 0.0,
        .color = color,
    };
}

bool FaderDone(const Fader* self)
{
    return self->timer >= self->duration;
}

void FaderReset(Fader* self)
{
    self->timer = 0.0;
    self->previous = 0.0;
    self->current = 0.0;
}

void FaderUpdate(Fader* self)
{
    self->timer += CTX_DT;

    self->previous = self->current;
    self->current = self->timer / self->duration;

    self->current = MIN(self->current, 1.0);
}

void FaderDraw(const Fader* self)
{
    const f32 alpha = Lerp(self->previous, self->current, ContextGetAlpha());

    const u8 r = (u8)floorf(self->color.r * alpha);
    const u8 g = (u8)floorf(self->color.g * alpha);
    const u8 b = (u8)floorf(self->color.b * alpha);
    const u8 a = (u8)floorf(self->color.a * alpha);

    const Color color = (Color)
    {
        .r = r,
        .g = g,
        .b = b,
        .a = a,
    };

    DrawRectangle(0, 0, CTX_VIEWPORT_WIDTH, CTX_VIEWPORT_HEIGHT, color);
}
