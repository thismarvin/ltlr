#pragma once

#include "common.h"

#define CTX_VIEWPORT_WIDTH 320
#define CTX_VIEWPORT_HEIGHT 180

#define DEFAULT_WINDOW_WIDTH CTX_VIEWPORT_WIDTH * 3
#define DEFAULT_WINDOW_HEIGHT CTX_VIEWPORT_HEIGHT * 3

// Target (fixed) delta time.
#define CTX_DT (1.0 / 60.0)

// f32 ContextGetDeltaTime();
f64 ContextGetTotalTime();
void ContextSetTotalTime(f64 value);
f32 ContextGetAlpha();
void ContextSetAlpha(f32 value);
