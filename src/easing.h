#pragma once

typedef double f64;

typedef struct
{
    f64 value;
} EasingFnParams;

typedef f64 (*EasingFn)(const EasingFnParams*);

f64 EaseInQuad(const EasingFnParams* params);
f64 EaseOutQuad(const EasingFnParams* params);
f64 EaseInOutQuad(const EasingFnParams* params);
