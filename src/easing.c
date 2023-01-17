#include "easing.h"
#include <math.h>

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
