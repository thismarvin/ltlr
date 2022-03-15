#include "common.h"
#include "context.h"
#include "math.h"
#include "raylib.h"
#include "scene.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

static void Initialize(void);
static void Update(void);
static void Draw(void);

static const f32 targetFrameTime = CTX_DT;
static const u8 maxFrameSkip = 25;
static const f32 maxDeltaTime = maxFrameSkip * targetFrameTime;
static f32 accumulator = 0.0;
static f64 previousTime = 0.0;

// TODO(thismarvin): Expose preferences somehow...
static bool preferIntegerScaling = false;

static bool ensureResize;
static f32 delayedResizeTimer;

static Camera2D screenSpace;
static RenderTexture2D targetTexture;
static Rectangle targetTextureSource;
static Rectangle targetTextureDestination;
static Vector2 targetTextureOrigin;

static Texture2D atlas;

static Scene scene;

static void CalculateZoom(void)
{
    f32 screenWidth = GetScreenWidth();
    f32 screenHeight = GetScreenHeight();

    // Assume we need letterboxing.
    f32 zoom = screenWidth / CTX_VIEWPORT_WIDTH;

    // Check if pillarboxing is more appropriate.
    if (CTX_VIEWPORT_HEIGHT * zoom > screenHeight)
    {
        zoom = screenHeight / CTX_VIEWPORT_HEIGHT;
    }

    if (preferIntegerScaling)
    {
        zoom = floor(zoom);
    }

    screenSpace.zoom = zoom;

    // Account for letterboxing/pillarboxing.
    screenSpace.offset = (Vector2)
    {
        .x = (screenWidth - CTX_VIEWPORT_WIDTH * zoom) * 0.5,
        .y = (screenHeight - CTX_VIEWPORT_HEIGHT * zoom) * 0.5,
    };
}

static void Timestep(void)
{
    if (IsWindowResized())
    {
        CalculateZoom();

        ensureResize = true;
        delayedResizeTimer = 0;
    }

    f64 currentTime = GetTime();

    f32 deltaTime = currentTime - previousTime;

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

    if (ensureResize)
    {
        delayedResizeTimer += deltaTime;

        if (delayedResizeTimer > 1)
        {
            CalculateZoom();

            ensureResize = false;
            delayedResizeTimer = 0;
        }
    }
}

int main(void)
{
    // TODO(thismarvin): Incorporate a config file or cli options for window resolution.
    const f32 scale = 3;

    InitWindow(CTX_VIEWPORT_WIDTH * scale, CTX_VIEWPORT_HEIGHT * scale, "LTL");
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
    UnloadTexture(atlas);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}

static void Initialize(void)
{
    screenSpace = (Camera2D)
    {
        .offset = VECTOR2_ZERO,
        .target = VECTOR2_ZERO,
        .rotation = 0,
        .zoom = 1,
    };

    CalculateZoom();

    targetTexture = LoadRenderTexture(CTX_VIEWPORT_WIDTH, CTX_VIEWPORT_HEIGHT);

    targetTextureSource = (Rectangle)
    {
        .x = 0,
        .y = 0,
        .width = targetTexture.texture.width,
        .height = -targetTexture.texture.height,
    };

    targetTextureDestination = (Rectangle)
    {
        .x = targetTexture.texture.width * 0.5,
        .y = targetTexture.texture.height * 0.5,
        .width = targetTexture.texture.width,
        .height = targetTexture.texture.height,
    };

    targetTextureOrigin = (Vector2)
    {
        .x = targetTexture.texture.width * 0.5,
        .y = targetTexture.texture.height * 0.5,
    };

#if defined(PLATFORM_WEB)
    atlas = LoadTexture("./src/resources/build/atlas.png");
#else
    atlas = LoadTexture("./resources/build/atlas.png");
#endif

    SceneInit(&scene);
}

static void Update(void)
{
    SceneUpdate(&scene);
}

static void Draw(void)
{
    BeginTextureMode(targetTexture);
    SceneDraw(&scene, &atlas);
    EndTextureMode();

    BeginDrawing();
    BeginMode2D(screenSpace);
    ClearBackground(BLACK);
    DrawTexturePro(targetTexture.texture, targetTextureSource, targetTextureDestination,
                   targetTextureOrigin, 0, WHITE);
    EndMode2D();
    EndDrawing();
}
