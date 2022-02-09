#include "raylib.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

static void Update(void);
static void Draw(void);

static const int screenWidth = 320;
static const int screenHeight = 180;

static void UpdateDrawFrame(void)
{
    Update();
    Draw();
}

int main(void)
{
    InitWindow(screenWidth, screenHeight, "LTL");
    InitAudioDevice();

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

}

static void Draw(void)
{
    BeginDrawing();

    ClearBackground(WHITE);

    EndDrawing();
}
