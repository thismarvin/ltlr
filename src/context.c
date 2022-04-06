#include "context.h"

static f64 totalTime = 0.0;
static f32 alpha = 0.0;

// f32 ContextGetDeltaTime()
// {
//     return CTX_DT;
// }

f64 ContextGetTotalTime()
{
    return totalTime;
}

void ContextSetTotalTime(const f64 value)
{
    totalTime = value;
}

f32 ContextGetAlpha()
{
    return alpha;
}

void ContextSetAlpha(const f32 value)
{
    alpha = value;
}
