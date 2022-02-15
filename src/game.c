#include "constants.h"
#include "player.h"
#include "raylib.h"
#include "scene.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

static void Update(void);
static void Draw(void);

static const int screenWidth = 320;
static const int screenHeight = 180;

static Player player;
static Scene* scene;

static void UpdateDrawFrame(void)
{
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
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);

    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
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
