#pragma once

#include "common.h"
#include "easing.h"

typedef struct
{
    f32 duration;
    f32 timer;
    f32 previous;
    f32 current;
    Color color;
    EasingFn ease;
} Fader;

Fader FaderCreate(const EasingFn ease, Color color, f32 duration);
bool FaderDone(const Fader* self);
void FaderReset(Fader* self);
void FaderUpdate(Fader* self);
void FaderDraw(const Fader* self);
