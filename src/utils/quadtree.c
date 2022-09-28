#include "quadtree.h"

typedef struct
{
    Rectangle aabb;
    usize id;
} QuadtreeEntry;

static Quadtree* New(const Rectangle region, const u8 maxDepth, const u8 depth)
{
    Quadtree* quadtree = malloc(sizeof(Quadtree));

    quadtree->region = region;
    quadtree->maxDepth = maxDepth;
    quadtree->depth = depth;
    quadtree->entries = DEQUE_OF(QuadtreeEntry);
    quadtree->topLeft = NULL;
    quadtree->topRight = NULL;
    quadtree->bottomLeft = NULL;
    quadtree->bottomRight = NULL;

    return quadtree;
}

static void QuadtreeSubdivide(Quadtree* self)
{
    if (self->depth == self->maxDepth)
    {
        return;
    }

    const u8 depth = self->depth + 1;

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

    self->topLeft = New(topLeft, self->maxDepth, depth);
    self->topRight = New(topRight, self->maxDepth, depth);
    self->bottomLeft = New(bottomLeft, self->maxDepth, depth);
    self->bottomRight = New(bottomRight, self->maxDepth, depth);

    QuadtreeSubdivide(self->topLeft);
    QuadtreeSubdivide(self->topRight);
    QuadtreeSubdivide(self->bottomLeft);
    QuadtreeSubdivide(self->bottomRight);
}

Quadtree* QuadtreeNew(const Rectangle region, const u8 maxDepth)
{
    Quadtree* quadtree = New(region, maxDepth, 0);
    QuadtreeSubdivide(quadtree);

    return quadtree;
}

static bool QuadtreeIsLeaf(const Quadtree* self)
{
    return self->depth == self->maxDepth;
}

bool QuadtreeAdd(Quadtree* self, const usize id, const Rectangle aabb)
{
    if (!RectangleContains(self->region, aabb))
    {
        return false;
    }

    if (QuadtreeIsLeaf(self))
    {
        QuadtreeEntry entry = (QuadtreeEntry)
        {
            .id = id,
            .aabb = aabb,
        };
        DequePushBack(&self->entries, &entry);

        return true;
    }

    if (QuadtreeAdd(self->topLeft, id, aabb)
            || QuadtreeAdd(self->topRight, id, aabb)
            || QuadtreeAdd(self->bottomRight, id, aabb)
            || QuadtreeAdd(self->bottomLeft, id, aabb))
    {
        return true;
    }

    QuadtreeEntry entry = (QuadtreeEntry)
    {
        .id = id,
        .aabb = aabb,
    };
    DequePushBack(&self->entries, &entry);

    return true;
}

static void QuadtreeQueryHelper(const Quadtree* current, const Rectangle region, Deque* result)
{
    if (!CheckCollisionRecs(current->region, region))
    {
        return;
    }

    if (RectangleContains(region, current->region))
    {
        for (usize i = 0; i < DequeGetSize(&current->entries); ++i)
        {
            QuadtreeEntry* entry = &DEQUE_GET_UNCHECKED(&current->entries, QuadtreeEntry, i);
            DequePushBack(result, &entry->id);
        }
    }
    else
    {
        for (usize i = 0; i < DequeGetSize(&current->entries); ++i)
        {
            QuadtreeEntry* entry = &DEQUE_GET_UNCHECKED(&current->entries, QuadtreeEntry, i);

            if (CheckCollisionRecs(entry->aabb, region))
            {
                DequePushBack(result, &entry->id);
            }
        }
    }

    if (QuadtreeIsLeaf(current))
    {
        return;
    }

    QuadtreeQueryHelper(current->topLeft, region, result);
    QuadtreeQueryHelper(current->topRight, region, result);
    QuadtreeQueryHelper(current->bottomRight, region, result);
    QuadtreeQueryHelper(current->bottomLeft, region, result);
}

// Returns a `Deque<usize>` of the entities that are within the given region.
Deque QuadtreeQuery(const Quadtree* self, const Rectangle region)
{
    Deque result = DEQUE_OF(usize);
    QuadtreeQueryHelper(self, region, &result);

    return result;
}

void QuadtreeClear(Quadtree* self)
{
    DequeClear(&self->entries);

    if (!QuadtreeIsLeaf(self))
    {
        QuadtreeClear(self->topLeft);
        QuadtreeClear(self->topRight);
        QuadtreeClear(self->bottomRight);
        QuadtreeClear(self->bottomLeft);
    }
}

void QuadtreeDestroy(Quadtree* self)
{
    DequeDestroy(&self->entries);

    if (!QuadtreeIsLeaf(self))
    {
        QuadtreeDestroy(self->topLeft);
        QuadtreeDestroy(self->topRight);
        QuadtreeDestroy(self->bottomRight);
        QuadtreeDestroy(self->bottomLeft);
    }

    free(self);
}
