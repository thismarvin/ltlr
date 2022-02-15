#ifndef SCENE_H
#define SCENE_H

#include "player.h"

// TODO(thismarvin): How do we incorporate the following:
// - Collide-ables
// - Enemies
// - Sprites
//   - Blocks
//   - Decorations
//   - Background
// - Interactable Stuff (e.g. solar panels, batteries, etc.)

typedef struct
{
    int width;
    int height;
    Player* player;
} Scene;

void SceneInit(Scene* self, int width, int height, Player* player);
void SceneUpdate(Scene* self);
void SceneDraw(Scene* self);

#endif
