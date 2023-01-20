#include "common.h"
#include "context.h"
#include "scene.h"
#include <math.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

#define FRAMERATE_SAMPLING_FREQUENCY (0.1f)

static void Initialize(void);
static void Update(void);
static void Draw(void);

static const f32 targetFrameTime = CTX_DT;
static const u8 maxFrameSkip = 25;
static const f32 maxDeltaTime = maxFrameSkip * targetFrameTime;
static f32 accumulator = 0.0;
static f64 previousTime = 0.0;

static usize currentFrame;
static f64 sampleTime;
static usize sampleFrame;
static f64 averageFps;

static bool debugging;

static Image icon;

static Scene scene;

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
    InitWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "Larry the Light-bulb Redux");
    InitAudioDevice();

    SetWindowState(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(CTX_VIEWPORT_WIDTH, CTX_VIEWPORT_HEIGHT);

    icon = LoadImage("./content/icon.png");

    SetWindowIcon(icon);

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

    UnloadImage(icon);

    return 0;
}

static void Initialize(void)
{
    ContextInit();

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

    const Camera2D camera = (Camera2D)
    {
        .offset = VECTOR2_ZERO,
        .target = VECTOR2_ZERO,
        .rotation = 0.0f,
        .zoom = zoom * 0.5f,
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

        const Color backgroundColor = (Color)
        {
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

static void Draw(void)
{
    SceneDraw(&scene);

    DrawDebugInformation();
}
