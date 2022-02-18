#include "common.h"
#include "context.h"
#include "player.h"
#include "raylib.h"
#include "scene.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

static void Initialize(void);
static void Update(void);
static void Draw(void);

static const int screenWidth = 320;
static const int screenHeight = 180;

static const float target = CTX_DT;
static const int maxFrameSkip = 25;
static const float maxDeltaTime = maxFrameSkip * target;
static float accumulator = 0.0;
static double previousTime = 0.0;

static Player player;
static Scene scene;

static void Timestep(void)
{
    // TODO(thismarvin): Handle resize here...

    double currentTime = GetTime();

    float deltaTime = currentTime - previousTime;

    // Set a maximum delta time in order to avoid a "spiral of death."
    if (deltaTime > maxDeltaTime) {
        deltaTime = maxDeltaTime;
    }

    previousTime = currentTime;

    accumulator += deltaTime;

    while (accumulator >= target) {
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

    CloseAudioDevice();
    CloseWindow();

    return 0;
}

static void Initialize(void)
{
    PlayerInit(&player, Vector2Create(32, 32));

    SceneInit(&scene, 320, 180, &player);
}

static void Update(void)
{
    SceneUpdate(&scene);
}

static void Draw(void)
{
    BeginDrawing();

    ClearBackground(WHITE);

    SceneDraw(&scene);

    EndDrawing();
}
