#include "common.h"
#include "context.h"
#include "scene.h"
#include <math.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

#define MAX_FRAMERATE_SAMPLES 300

static void Initialize(void);
static void Update(void);
static void Draw(void);

static const f32 targetFrameTime = CTX_DT;
static const u8 maxFrameSkip = 25;
static const f32 maxDeltaTime = maxFrameSkip * targetFrameTime;
static f32 accumulator = 0.0;
static f64 previousTime = 0.0;

static f64 framerateSamples[MAX_FRAMERATE_SAMPLES];
static usize framerateSamplesHead;
static bool collectedEnoughSamples;
static f64 averageFps;

static bool debugging;

static Scene scene;

static void Timestep(void)
{
    f64 currentTime = GetTime();

    f32 deltaTime = currentTime - previousTime;

    // Calculate the average frames per second.
    {
        framerateSamples[framerateSamplesHead] = 1 / deltaTime;
        framerateSamplesHead += 1;

        if (framerateSamplesHead >= MAX_FRAMERATE_SAMPLES)
        {
            collectedEnoughSamples = true;
            framerateSamplesHead = 0;
        }

        averageFps = 0.0f;

        for (usize i = 0; i < MAX_FRAMERATE_SAMPLES; ++i)
        {
            averageFps += framerateSamples[i];
        }

        averageFps /= MAX_FRAMERATE_SAMPLES;
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
        Update();

        accumulator -= targetFrameTime;
        ContextSetTotalTime(ContextGetTotalTime() + targetFrameTime);

        PollInputEvents();
    }

    ContextSetAlpha(accumulator / targetFrameTime);

    Draw();

    SwapScreenBuffer();
}

int main(void)
{
    // TODO(thismarvin): Incorporate a config file or cli options for window resolution.
    InitWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "LTL");
    InitAudioDevice();

    SetWindowState(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(CTX_VIEWPORT_WIDTH, CTX_VIEWPORT_HEIGHT);

    Initialize();

    previousTime = GetTime();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(Timestep, 0, 1);
#else

    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        Timestep();
    }

#endif

    SceneDestroy(&scene);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}

static void Initialize(void)
{
    SceneInit(&scene);
}

static void Update(void)
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

    f32 screenWidth = GetScreenWidth();
    f32 screenHeight = GetScreenHeight();

    // Assume we need letterboxing.
    f32 zoom = screenWidth / CTX_VIEWPORT_WIDTH;

    // Check if pillarboxing is more appropriate.
    if (CTX_VIEWPORT_HEIGHT * zoom > screenHeight)
    {
        zoom = screenHeight / CTX_VIEWPORT_HEIGHT;
    }

    zoom = floor(zoom);

    Camera2D camera = (Camera2D)
    {
        .offset = VECTOR2_ZERO,
        .target = VECTOR2_ZERO,
        .rotation = 0.0f,
        .zoom = zoom * 0.5f,
    };

    BeginMode2D(camera);
    {
        const char* text = "Sampling Framerate...";

        if (collectedEnoughSamples)
        {
            text = TextFormat("%.f FPS", averageFps);
        }

        const usize fontSize = 20;
        const usize textWidth = MeasureText(text, fontSize);
        const f32 x = 0;
        const f32 y = 0;
        const usize xPadding = 8;
        const usize yPadding = 8;

        DrawRectangle(x, y, textWidth + xPadding * 2, fontSize + yPadding * 2 - 1, (Color)
        {
            0, 0, 0, 150
        });
        DrawText(text, x + xPadding + 2, y + yPadding + 2, fontSize, COLOR_BLACK);
        DrawText(text, x + xPadding, y + yPadding, fontSize, COLOR_WHITE);
    }
    EndMode2D();
}

static void Draw(void)
{
    SceneDraw(&scene);

    DrawDebugInformation();
}
