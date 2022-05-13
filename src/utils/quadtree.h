#pragma once

#include "../collections/deque.h"
#include "../common.h"

#define QUADTREE_CAPACITY 4

typedef struct
{
    Rectangle region;
    usize entity;
} QuadtreeEntry;

struct Quadtree
{
    Rectangle region;
    QuadtreeEntry entries[QUADTREE_CAPACITY];
    usize headIndex;

    struct Quadtree* topLeft;
    struct Quadtree* topRight;
    struct Quadtree* bottomLeft;
    struct Quadtree* bottomRight;
};

typedef struct Quadtree Quadtree;

Quadtree QuadtreeCreate(Rectangle region);
bool QuadtreeAdd(Quadtree* self, usize entity, Rectangle aabb);
Deque QuadtreeQuery(const Quadtree* self, Rectangle region);
void QuadtreeClear(Quadtree* self);
void QuadtreeDestroy(Quadtree* self);
