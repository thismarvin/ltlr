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

static const f32 target = CTX_DT;
static const u8 maxFrameSkip = 25;
static const f32 maxDeltaTime = maxFrameSkip * target;
static f32 accumulator = 0.0;
static f64 previousTime = 0.0;

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

    while (accumulator >= target)
    {
        Update();

        accumulator -= target;
        ContextSetTotalTime(ContextGetTotalTime() + target);
    }

    ContextSetAlpha(accumulator / target);

    Draw();
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
#if defined(PLATFORM_WEB)
    atlas = LoadTexture("./src/resources/sprites.png");
#else
    atlas = LoadTexture("./resources/sprites.png");
#endif

    SceneInit(&scene);
}

static void Update(void)
{
    SceneUpdate(&scene);
}

static void Draw(void)
{
    BeginDrawing();

    ClearBackground(WHITE);

    SceneDraw(&scene, &atlas);

    EndDrawing();
}
