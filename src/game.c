#include "game.h"

#include "common.h"
#include "context.h"
#include "raylib.h"
#include "replay.h"
#include "scene.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(PLATFORM_WEB)
	#include <emscripten/emscripten.h>
#endif

#define FRAMERATE_SAMPLING_FREQUENCY (0.1F)

static const f32 targetFrameTime = CTX_DT;
static const u8 maxFrameSkip = 25;
static const f32 maxDeltaTime = maxFrameSkip * targetFrameTime;
static f32 accumulator = 0.0F;
static f64 previousTime = 0.0;

static usize currentFrame;
static f64 sampleTime;
static usize sampleFrame;
static f64 averageFps;

static bool debugging;

static Image icon;

static Scene scene;

void GameInitialize(void)
{
	ContextInit();

	SceneInit(&scene);
}

void GameUpdate(void)
{
	if (IsKeyPressed(KEY_EQUAL))
	{
		debugging = !debugging;
	}

	SceneUpdate(&scene);
}

static void DrawDebugInformation(void)
{
	if (!debugging)
	{
		return;
	}

	const f32 screenWidth = GetScreenWidth();
	const f32 screenHeight = GetScreenHeight();

	// Assume we need letterboxing.
	f32 zoom = screenWidth / CTX_VIEWPORT_WIDTH;

	// Check if pillarboxing is more appropriate.
	if (CTX_VIEWPORT_HEIGHT * zoom > screenHeight)
	{
		zoom = screenHeight / CTX_VIEWPORT_HEIGHT;
	}

	zoom = floor(zoom);

	const Camera2D camera = (Camera2D) {
		.offset = VECTOR2_ZERO,
		.target = VECTOR2_ZERO,
		.rotation = 0.0F,
		.zoom = zoom * 0.5F,
	};

	BeginMode2D(camera);
	{
		static const usize fontSize = 20;
		static const f32 x = 0;
		static const f32 y = 0;
		static const usize xPadding = 8;
		static const usize yPadding = 8;

		const char* text = TextFormat("%.f FPS", averageFps);
		const usize textWidth = MeasureText(text, fontSize);

		const Color backgroundColor = (Color) {
			.r = 0,
			.g = 0,
			.b = 0,
			.a = 150,
		};
		DrawRectangle(x, y, textWidth + xPadding * 2, fontSize + yPadding * 2 - 1, backgroundColor);

		DrawText(text, x + xPadding + 2, y + yPadding + 2, fontSize, COLOR_BLACK);
		DrawText(text, x + xPadding, y + yPadding, fontSize, COLOR_WHITE);
	}
	EndMode2D();
}

void GameDraw(void)
{
	SceneDraw(&scene);

	DrawDebugInformation();
}

static void Timestep(void)
{
	const f64 currentTime = GetTime();

	f32 deltaTime = currentTime - previousTime;

	// Calculate the average frames per second.
	{
		currentFrame += 1;

		if ((currentTime - sampleTime) >= FRAMERATE_SAMPLING_FREQUENCY)
		{
			averageFps = (currentFrame - sampleFrame) / (currentTime - sampleTime);

			sampleFrame = currentFrame;
			sampleTime = currentTime;
		}
	}

	// Set a maximum delta time in order to avoid a "spiral of death."
	if (deltaTime > maxDeltaTime)
	{
		deltaTime = maxDeltaTime;
	}

	previousTime = currentTime;

	accumulator += deltaTime;

	while (accumulator >= targetFrameTime)
	{
		GameUpdate();

		accumulator -= targetFrameTime;
		ContextSetTotalTime(ContextGetTotalTime() + targetFrameTime);

		PollInputEvents();
	}

	ContextSetAlpha(accumulator / targetFrameTime);

	GameDraw();

	SwapScreenBuffer();
}

void GameRun(void)
{
#if defined(BENCHMARKING)
	SetTraceLogLevel(LOG_NONE);
	SceneInit(&scene);

	u32 size;
	const u8* data = LoadFileData("baseline.ltlrr", &size);

	ReplayResult result = ReplayTryFromBytes(data, size);

	if (result.type == REPLAY_RESULT_TYPE_ERR)
	{
		fprintf(stderr, "%s\n", StringFromReplayError(result.contents.err));
		exit(EXIT_FAILURE);
	}

	bool loaded = InputStreamLoadReplay(&scene.inputStreams[0], &result.contents.ok);

	if (!loaded)
	{
		fprintf(stderr, "Replay is too long to fit inside InputStream.\n");
		exit(EXIT_FAILURE);
	}

	scene.seed = result.contents.ok.seed;

	for (usize i = 0; i < result.contents.ok.length; ++i)
	{
		SceneUpdate(&scene);
	}

	return;
#endif

	// TODO(thismarvin): Incorporate a config file or cli options for window resolution.
	InitWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "Larry the Light-bulb Redux");
	InitAudioDevice();

	SetWindowState(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
	SetWindowMinSize(CTX_VIEWPORT_WIDTH, CTX_VIEWPORT_HEIGHT);

	icon = LoadImage(DATADIR "content/icon.png");

	SetWindowIcon(icon);

	GameInitialize();

	previousTime = GetTime();

#if defined(PLATFORM_WEB)
	emscripten_set_main_loop(Timestep, 0, 1);
#else

	// Detect window close button or ESC key.
	while (!WindowShouldClose())
	{
		Timestep();
	}

#endif

	SceneDestroy(&scene);

	CloseAudioDevice();
	CloseWindow();

	UnloadImage(icon);
}
