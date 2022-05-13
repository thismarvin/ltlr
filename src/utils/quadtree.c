#include "quadtree.h"

Quadtree QuadtreeCreate(const Rectangle region)
{
    return (Quadtree)
    {
        .region = region,
        .headIndex = 0,
        .topLeft = NULL,
        .topRight = NULL,
        .bottomLeft = NULL,
        .bottomRight = NULL,
    };
}

static bool QuadtreeHasDivided(const Quadtree* self)
{
    return self->topLeft != NULL
           || self->topRight != NULL
           || self->bottomLeft != NULL
           || self->bottomRight != NULL;
}

static void QuadtreeSubdivide(Quadtree* self)
{
    const f32 width = self->region.width * 0.5f;
    const f32 height = self->region.height * 0.5f;

    const Rectangle topLeft = (Rectangle)
    {
        .x = self->region.x,
        .y = self->region.y,
        .width = width,
        .height = height,
    };

    const Rectangle topRight = (Rectangle)
    {
        .x = self->region.x + width,
        .y = self->region.y,
        .width = width,
        .height = height,
    };

    const Rectangle bottomLeft = (Rectangle)
    {
        .x = self->region.x,
        .y = self->region.y + height,
        .width = width,
        .height = height,
    };

    const Rectangle bottomRight = (Rectangle)
    {
        .x = self->region.x + width,
        .y = self->region.y + height,
        .width = width,
        .height = height,
    };

    self->topLeft = (Quadtree*)malloc(sizeof(Quadtree));
    *self->topLeft = QuadtreeCreate(topLeft);

    self->topRight = (Quadtree*)malloc(sizeof(Quadtree));
    *self->topRight = QuadtreeCreate(topRight);

    self->bottomLeft = (Quadtree*)malloc(sizeof(Quadtree));
    *self->bottomLeft = QuadtreeCreate(bottomLeft);

    self->bottomRight = (Quadtree*)malloc(sizeof(Quadtree));
    *self->bottomRight = QuadtreeCreate(bottomRight);
}

bool QuadtreeAdd(Quadtree* self, const usize entity, const Rectangle aabb)
{
    if (!CheckCollisionRecs(self->region, aabb))
    {
        return false;
    }

    if (self->headIndex < QUADTREE_CAPACITY)
    {
        self->entries[self->headIndex++] = (QuadtreeEntry)
        {
            .entity = entity,
            .region = aabb,
        };
        return true;
    }

    if (!QuadtreeHasDivided(self))
    {
        QuadtreeSubdivide(self);
    }

    return QuadtreeAdd(self->topLeft, entity, aabb)
           || QuadtreeAdd(self->topRight, entity, aabb)
           || QuadtreeAdd(self->bottomRight, entity, aabb)
           || QuadtreeAdd(self->bottomLeft, entity, aabb);
}

static void QuadtreeQueryHelper(const Quadtree* current, const Rectangle region, Deque* result)
{
    if (current == NULL || !CheckCollisionRecs(current->region, region))
    {
        return;
    }

    for (usize i = 0; i < current->headIndex; ++i)
    {
        const QuadtreeEntry* entry = &current->entries[i];

        if (CheckCollisionRecs(entry->region, region))
        {
            DequePushBack(result, &entry->entity);
        }
    }

    QuadtreeQueryHelper(current->topLeft, region, result);
    QuadtreeQueryHelper(current->topRight, region, result);
    QuadtreeQueryHelper(current->bottomRight, region, result);
    QuadtreeQueryHelper(current->bottomLeft, region, result);
}

Deque QuadtreeQuery(const Quadtree* self, const Rectangle region)
{
    Deque result = DEQUE_OF(usize);

    QuadtreeQueryHelper(self, region, &result);

    return result;
}

void QuadtreeClear(Quadtree* self)
{
    self->headIndex = 0;

    QuadtreeDestroy(self->topLeft);
    QuadtreeDestroy(self->topRight);
    QuadtreeDestroy(self->bottomRight);
    QuadtreeDestroy(self->bottomLeft);
}

void QuadtreeDestroy(Quadtree* self)
{
    if (self == NULL)
    {
        return;
    }

    QuadtreeDestroy(self->topLeft);
    QuadtreeDestroy(self->topRight);
    QuadtreeDestroy(self->bottomRight);
    QuadtreeDestroy(self->bottomLeft);

    free(self);
}
