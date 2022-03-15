#include "common.h"
#include "context.h"
#include "raylib.h"
#include "scene.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

static void Initialize(void);
static void Update(void);
static void Draw(void);

static const u16 screenWidth = 320 * CTX_ZOOM;
static const u16 screenHeight = 180 * CTX_ZOOM;
static const f32 targetFrameTime = CTX_DT;
static const u8 maxFrameSkip = 25;
static const f32 maxDeltaTime = maxFrameSkip * targetFrameTime;
static f32 accumulator = 0.0;
static f64 previousTime = 0.0;

static Camera2D screenSpace;
static RenderTexture2D targetTexture;
static Rectangle targetTextureSource;
static Rectangle targetTextureDestination;
static Vector2 targetTextureOrigin;

static Texture2D atlas;

static Scene scene;

static void Timestep(void)
{
    // TODO(thismarvin): Handle resize here...

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
}

int main(void)
{
    InitWindow(screenWidth, screenHeight, "LTL");
    InitAudioDevice();

    SetWindowState(FLAG_VSYNC_HINT);

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
        .zoom = CTX_ZOOM,
    };

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
