#include "context.h"

static double totalTime = 0.0;
static float alpha = 0.0;

// float ContextGetDeltaTime()
// {
//     return CTX_DT;
// }

double ContextGetTotalTime()
{
    return totalTime;
}

void ContextSetTotalTime(double value)
{
    totalTime = value;
}

float ContextGetAlpha()
{
    return alpha;
}

void ContextSetAlpha(float value)
{
    alpha = value;
}
