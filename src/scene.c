#include "scene.h"
#include "player.h"

void SceneInit(Scene* self, int width, int height, Player* player)
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
