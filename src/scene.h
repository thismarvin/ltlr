#pragma once

#include "common.h"
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
    i32 width;
    i32 height;
    Player* player;
} Scene;

void SceneInit(Scene* self, i32 width, i32 height, Player* player);
void SceneUpdate(Scene* self);
void SceneDraw(Scene* self);
