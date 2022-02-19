#include "scene.h"
#include "systems.h"
#include <assert.h>
#include <stdio.h>
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

static usize AllocateEntity(Scene* self)
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

static void DeallocateEntity(Scene* self, usize entity) {
    self->components.tags[entity] = 0;
    PushFree(self, entity);
}

static void CreateDummy(Scene* self, f32 x, f32 y) {
    usize entity = AllocateEntity(self);

    self->components.tags[entity] = tagPosition | tagDimension;

    self->components.positions[entity].value = Vector2Create(x, y);
    self->components.dimensions[entity] = (CDimension)
    {
        .width = 32,
        .height = 16
    };
}

void SceneInit(Scene* self)
{
    memset(&self->components.tags, 0, sizeof(u64) * MAX_ENTITIES);

    // TODO(thismarvin): Do we need to initialize positions, dimensions, etc.?

    self->nextEntity = 0;

    self->nextFreeSlot = 0;
    memset(&self->freeSlots, 0, sizeof(u64));

    // Testing!
    {
        CreateDummy(self, 32, 32);
        CreateDummy(self, 64, 64);
        CreateDummy(self, 0, 80);
        CreateDummy(self, 16, 120);

        self->components.tags[0] = tagPosition | tagDimension | tagColor;
        self->components.colors[0].value = BLUE;

        DeallocateEntity(self, 1);
        DeallocateEntity(self, 3);

        CreateDummy(self, 16, 120);
        CreateDummy(self, 64, 64);

        CreateDummy(self, 32, 32);
        CreateDummy(self, 32, 150);
    }
}

void SceneUpdate(Scene* self)
{
    for (usize i = 0; i < self->nextEntity; ++i)
    {
        SDummyUpdate(&self->components, i);
    }
}

void SceneDraw(Scene* self)
{
    for (usize i = 0; i < self->nextEntity; ++i)
    {
        SDummyDraw(&self->components, i);
    }
}
