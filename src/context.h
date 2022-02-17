#pragma once

// Target (fixed) delta time.
#define CTX_DT (1.0 / 60.0)

// float ContextGetDeltaTime();
double ContextGetTotalTime();
void ContextSetTotalTime(double value);
float ContextGetAlpha();
void ContextSetAlpha(float value);
