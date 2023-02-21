#pragma once

#include "common.h"

#define MAGIC_NUMBER (20180217)

#define CTX_VIEWPORT_WIDTH (320)
#define CTX_VIEWPORT_HEIGHT (180)

#define CTX_VIEWPORT ((Rectangle) { 0, 0, CTX_VIEWPORT_WIDTH, CTX_VIEWPORT_HEIGHT })

#define DEFAULT_WINDOW_WIDTH (CTX_VIEWPORT_WIDTH * 3)
#define DEFAULT_WINDOW_HEIGHT (CTX_VIEWPORT_HEIGHT * 3)

// Target (fixed) delta time.
#define CTX_DT (1.0 / 60.0)

typedef struct
{
	void* scene;
	Rectangle cameraBounds;
} RenderFnParams;

typedef void (*RenderFn)(const RenderFnParams*);

void ContextInit(void);
f64 ContextGetTotalTime(void);
void ContextSetTotalTime(f64 value);
f32 ContextGetAlpha(void);
void ContextSetAlpha(f32 value);

Rectangle GetMonitorResolution(void);
Rectangle GetScreenResolution(void);

Rectangle RectangleFromRenderTexture(const RenderTexture* renderTexture);
f32 CalculateZoom(Rectangle region, Rectangle container);

void RenderLayer(const RenderTexture* renderTexture, RenderFn fn, const RenderFnParams* params);
void DrawLayers(const RenderTexture* renderTextures, usize renderTexturesLength);
