#include "entities.h"
#include "scene.h"
#include "systems.h"
#include <assert.h>
#include <string.h>

static void PushFree(Scene* self, usize value)
{
    assert(self->nextFreeSlot < MAX_ENTITIES);

    self->freeSlots[self->nextFreeSlot] = value;
    self->nextFreeSlot += 1;
}

static usize PopFree(Scene* self)
{
    assert(self->nextFreeSlot > 0);

    usize slot = self->freeSlots[self->nextFreeSlot - 1];
    self->nextFreeSlot -= 1;

    return slot;
}

usize SceneAllocateEntity(Scene* self)
{
    if (self->nextFreeSlot == 0)
    {
        usize next = MIN(self->nextEntity, MAX_ENTITIES - 1);

        self->nextEntity = self->nextEntity + 1;
        self->nextEntity = MIN(self->nextEntity, MAX_ENTITIES);

        return next;
    }
    else
    {
        return PopFree(self);
    }
}

void SceneDeallocateEntity(Scene* self, usize entity)
{
    self->components.tags[entity] = 0;
    PushFree(self, entity);
}

void SceneInit(Scene* self)
{
    memset(&self->components.tags, 0, sizeof(u64) * MAX_ENTITIES);

    // TODO(thismarvin): Do we need to initialize positions, dimensions, etc.?

    self->nextEntity = 0;

    self->nextFreeSlot = 0;
    memset(&self->freeSlots, 0, sizeof(u64));

    self->debugging = false;

    ECreatePlayer(self, 8, 8);
    ECreateBlock(self, 0, 180 - 32, 320, 32);
}

usize SceneGetEntityCount(Scene* self)
{
    return self->nextEntity;
}

void SceneUpdate(Scene* self)
{
    if (IsKeyPressed(KEY_F3))
    {
        self->debugging = !self->debugging;
    }

    for (usize i = 0; i < self->nextEntity; ++i)
    {
        SSmoothUpdate(&self->components, i);
        SPlayerUpdate(&self->components, i);
        SKineticUpdate(&self->components, i);
        SCollisionUpdate(&self->components, self->nextEntity, i);
    }
}

void SceneDraw(Scene* self, Texture2D* atlas)
{
    for (usize i = 0; i < self->nextEntity; ++i)
    {
        SSpriteDraw(&self->components, atlas, i);

        if (self->debugging)
        {
            SDebugDraw(&self->components, i);
        }
    }

    if (self->debugging)
    {
        DrawFPS(8, 8);
    }
}
