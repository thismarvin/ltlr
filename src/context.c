#include "context.h"

#include <raymath.h>

static f64 totalTime;
static f32 alpha;

static Rectangle monitorRectangle;
static Rectangle previousRenderRectangle;

void ContextInit(void)
{
	totalTime = 0.0;
	alpha = 0.0;

	// Calculate the monitor's resolution.
	{
#if defined(PLATFORM_DESKTOP)
		const i32 currentMonitor = GetCurrentMonitor();

		const i32 width = GetMonitorWidth(currentMonitor);
		const i32 height = GetMonitorHeight(currentMonitor);
#else
		const i32 width = GetRenderWidth();
		const i32 height = GetRenderHeight();
#endif

		monitorRectangle = (Rectangle) {
			.x = 0,
			.y = 0,
			.width = width,
			.height = height,
		};
	}

	previousRenderRectangle = GetRenderRectangle();
}

f64 ContextGetTotalTime(void)
{
	return totalTime;
}

void ContextSetTotalTime(const f64 value)
{
	totalTime = value;
}

f32 ContextGetAlpha(void)
{
	return alpha;
}

void ContextSetAlpha(const f32 value)
{
	alpha = value;
}

Rectangle GetMonitorRectangle(void)
{
	return monitorRectangle;
}

Rectangle GetRenderRectangle(void)
{
	const Vector2 windowPosition = GetWindowPosition();

	return (Rectangle) {
		.x = windowPosition.x,
		.y = windowPosition.y,
		.width = GetRenderWidth(),
		.height = GetRenderHeight(),
	};
}

void ToggleFullscreenShim(void)
{
	// I'm probably missing something, but ToggleFullscreen's behavior is inconsistent across Linux
	// and Windows; the following code tries to fix said issue...

	if (IsWindowFullscreen())
	{
		ToggleFullscreen();

		SetWindowPosition(previousRenderRectangle.x, previousRenderRectangle.y);
		SetWindowSize(previousRenderRectangle.width, previousRenderRectangle.height);

		return;
	}

	// Save the window's bounds so we can switch back to it later.
	previousRenderRectangle = GetRenderRectangle();

	ToggleFullscreen();

	SetWindowPosition(monitorRectangle.x, monitorRectangle.y);
	SetWindowSize(monitorRectangle.width, monitorRectangle.height);
}

Rectangle RectangleFromRenderTexture(const RenderTexture* renderTexture)
{
	return (Rectangle) {
		.x = 0,
		.y = 0,
		.width = renderTexture->texture.width,
		.height = renderTexture->texture.height,
	};
}

// Returns the maximum value the dimensions of a given region can be multiplied by and still fit
// within a given container.
f32 CalculateZoom(const Rectangle region, const Rectangle container)
{
	// Assume we need letterboxing.
	f32 zoom = container.width / region.width;

	// Check if pillarboxing is more appropriate.
	if (region.height * zoom > container.height)
	{
		zoom = container.height / region.height;
	}

	return zoom;
}

void RenderLayer(
	const RenderTexture* renderTexture,
	const RenderFn fn,
	const RenderFnParams* params
)
{
	const Rectangle bounds = RectangleFromRenderTexture(renderTexture);
	const f32 zoom = CalculateZoom(CTX_VIEWPORT, bounds);

	const Vector2 cameraCenter = (Vector2) {
		.x = params->cameraBounds.x + CTX_VIEWPORT_WIDTH * 0.5,
		.y = params->cameraBounds.y + CTX_VIEWPORT_HEIGHT * 0.5,
	};

	const Camera2D camera = (Camera2D) {
		.zoom = zoom,
		.offset = Vector2Scale(cameraCenter, -zoom),
		.target =
			(Vector2) {
				.x = -CTX_VIEWPORT_WIDTH * 0.5,
				.y = -CTX_VIEWPORT_HEIGHT * 0.5,
			},
		.rotation = 0,
	};

	BeginTextureMode(*renderTexture);
	BeginMode2D(camera);
	{
		fn(params);
	}
	EndMode2D();
	EndTextureMode();
}

void DrawLayers(const RenderTexture* renderTextures, const usize renderTexturesLength)
{
	const Rectangle renderResolution = GetRenderRectangle();

	f32 zoom = CalculateZoom(CTX_VIEWPORT, renderResolution);

	// TODO(thismarvin): Expose "preferIntegerScaling" option.
	// Prefer integer scaling.
	zoom = floor(zoom);

	const i32 width = CTX_VIEWPORT_WIDTH * zoom;
	const i32 height = CTX_VIEWPORT_HEIGHT * zoom;

	const Rectangle destination = (Rectangle) {
		.x = floor(renderResolution.width * 0.5),
		.y = floor(renderResolution.height * 0.5),
		.width = width,
		.height = height,
	};

	const Vector2 origin = (Vector2) {
		.x = floor(width * 0.5),
		.y = floor(height * 0.5),
	};

	BeginDrawing();
	{
		ClearBackground(COLOR_BLACK);

		for (usize i = 0; i < renderTexturesLength; ++i)
		{
			const RenderTexture* renderTexture = &renderTextures[i];

			Rectangle source = RectangleFromRenderTexture(renderTexture);
			source.height *= -1;

			DrawTexturePro(renderTexture->texture, source, destination, origin, 0, WHITE);
		}
	}
	EndDrawing();
}
