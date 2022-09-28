#pragma once

#include "../collections/deque.h"
#include "../common.h"

struct Quadtree
{
    Rectangle region;
    u8 maxDepth;
    u8 depth;
    // `Deque<QuadtreeEntry>`
    Deque entries;
    struct Quadtree* topLeft;
    struct Quadtree* topRight;
    struct Quadtree* bottomLeft;
    struct Quadtree* bottomRight;
};

typedef struct Quadtree Quadtree;

Quadtree* QuadtreeNew(Rectangle region, u8 maxDepth);
bool QuadtreeAdd(Quadtree* self, usize id, Rectangle aabb);
Deque QuadtreeQuery(const Quadtree* self, Rectangle region);
void QuadtreeClear(Quadtree* self);
void QuadtreeDestroy(Quadtree* self);
