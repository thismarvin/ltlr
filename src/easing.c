#include "easing.h"
#include <math.h>

f64 EaseLinear(const EasingFnParams* params)
{
    const f64 x = params->value;

    return x;
}

// https://easings.net/#easeInQuad
f64 EaseInQuad(const EasingFnParams* params)
{
    const f64 x = params->value;

    return x * x;
}

// https://easings.net/#easeOutQuad
f64 EaseOutQuad(const EasingFnParams* params)
{
    const f64 x = params->value;

    return 1.0 - (1.0 - x) * (1.0 - x);
}

// https://easings.net/#easeInOutQuad
f64 EaseInOutQuad(const EasingFnParams* params)
{
    const f64 x = params->value;

    return x < 0.5 ? 2.0 * x * x : 1.0 - pow(-2.0 * x + 2.0, 2.0) / 2.0;
}

Easer EaserCreate(const EasingFn ease, const f64 duration)
{
    return (Easer)
    {
        .elapsed = 0.0,
        .duration = duration,
        .value = 0.0,
        .ease = ease,
    };
}

static void EaserRecalculateValue(Easer* self)
{
    const f64 value = self->elapsed / self->duration;

    const EasingFnParams params = (EasingFnParams)
    {
        .value = value,
    };

    self->value = self->ease(&params);
}

bool EaserIsDone(const Easer* self)
{
    return self->elapsed >= self->duration;
}

void EaserUpdate(Easer* self, f64 deltaTime)
{
    self->elapsed += deltaTime;

    if (self->elapsed > self->duration)
    {
        self->elapsed = self->duration;
    }

    EaserRecalculateValue(self);
}

f64 EaserLerp(const Easer* self, const f64 start, const f64 end)
{
    return start + self->value * (end - start);
}

f64 EaserLerpPrecise(const Easer* self, const f64 start, const f64 end)
{
    return (1.0 - self->value) * start + self->value * end;
}

void EaserReset(Easer* self)
{
    self->elapsed = 0.0;

    EaserRecalculateValue(self);
}
