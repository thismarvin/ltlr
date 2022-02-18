#pragma once

#include "common.h"

// Target (fixed) delta time.
#define CTX_DT (1.0 / 60.0)

// float ContextGetDeltaTime();
f64 ContextGetTotalTime();
void ContextSetTotalTime(f64 value);
f32 ContextGetAlpha();
void ContextSetAlpha(f32 value);
