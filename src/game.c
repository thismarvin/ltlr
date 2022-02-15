#include "constants.h"
#include "context.h"
#include "player.h"
#include "raylib.h"
#include "scene.h"
#include <stdio.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

static void Update(void);
static void Draw(void);

static const int screenWidth = 320;
static const int screenHeight = 180;

static float accumulator = 0.0;
static const float maxFrameSkip = 25;
static const float maxDeltaTime = maxFrameSkip * CTX_DT;

static Player player;
static Scene* scene;

static void UpdateDrawFrame(void)
{
    // TODO(thismarvin): Move this outside of the function.
    double previousTime = GetTime();

    while (!WindowShouldClose()) {
        // TODO(thismarvin): Handle resize here...

        double currentTime = GetTime();
        float deltaTime = currentTime - previousTime;

        // Set a maximum delta time in order to avoid a "spiral of death."
        if (deltaTime > maxDeltaTime) {
            deltaTime = maxDeltaTime;
        }

        previousTime = currentTime;

        accumulator += deltaTime;

        while (accumulator >= CTX_DT) {
            Update();

            accumulator -= CTX_DT;
            ContextSetTotalTime(ContextGetTotalTime() + CTX_DT);
        }

        ContextSetAlpha(accumulator / CTX_DT);

        Draw();

        // TODO(thismarvin): VSync?
    }

    Update();
    Draw();
}

int main(void)
{
    InitWindow(screenWidth, screenHeight, "LTL");
    InitAudioDevice();

    PlayerInit(&player, Vector2Create(32, 32));

    Scene currentScene;
    scene = &currentScene;

    SceneInit(scene, 320, 180, &player);

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);

    // FIXME(thismarvin): Extract WindowShouldClose back here!
    // while (!WindowShouldClose()) // Detect window close button or ESC key
    // {
    UpdateDrawFrame();
    // }
#endif

    CloseAudioDevice();
    CloseWindow();

    return 0;
}

static void Update(void)
{
    SceneUpdate(scene);
}

static void Draw(void)
{
    BeginDrawing();

    ClearBackground(WHITE);

    SceneDraw(scene);

    EndDrawing();
}
