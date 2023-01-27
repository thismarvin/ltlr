#pragma once

#include <stdbool.h>

typedef double f64;

typedef struct
{
	f64 value;
} EasingFnParams;

typedef f64 (*EasingFn)(const EasingFnParams*);

f64 EaseLinear(const EasingFnParams* params);

f64 EaseInQuad(const EasingFnParams* params);
f64 EaseOutQuad(const EasingFnParams* params);
f64 EaseInOutQuad(const EasingFnParams* params);

typedef struct
{
	f64 elapsed;
	f64 duration;
	f64 value;
	EasingFn ease;
} Easer;

Easer EaserCreate(const EasingFn ease, f64 duration);
bool EaserIsDone(const Easer* self);
void EaserUpdate(Easer* self, f64 deltaTime);
f64 EaserLerp(const Easer* self, f64 start, f64 end);
f64 EaserLerpPrecise(const Easer* self, f64 start, f64 end);
void EaserReset(Easer* self);
