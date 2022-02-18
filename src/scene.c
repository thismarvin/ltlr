#include "scene.h"
#include "player.h"

void SceneInit(Scene* self, u32 width, u32 height, Player* player)
{
    self->width = width;
    self->height = height;
    self->player = player;
}

void SceneUpdate(Scene* self)
{
    PlayerUpdate(self->player);
}

void SceneDraw(Scene* self)
{
    PlayerDraw(self->player);
}
