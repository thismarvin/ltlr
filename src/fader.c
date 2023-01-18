#include "context.h"
#include "fader.h"
#include <raymath.h>

Fader FaderDefault(void)
{
    return (Fader)
    {
        .easer = EaserCreate(EaseLinear, CTX_DT * 60),
        .previous = 0.0,
        .current = 0.0,
        .color = COLOR_BLACK,
        .type = FADE_IN,
    };
}

bool FaderIsDone(const Fader* self)
{
    return EaserIsDone(&self->easer);
}

void FaderReset(Fader* self)
{
    EaserReset(&self->easer);

    self->previous = 0.0;
    self->current = 0.0;
}

void FaderUpdate(Fader* self)
{
    EaserUpdate(&self->easer, CTX_DT);

    self->previous = self->current;
    self->current = self->easer.value;
}

void FaderDraw(const Fader* self)
{
    const f32 value = Lerp(self->previous, self->current, ContextGetAlpha());

    const f32 start = self->type == FADE_IN ? 1 : 0;
    const f32 end = self->type == FADE_IN ? 0 : 1;

    const f32 alpha = Lerp(start, end, value);

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
