#pragma once

#include "../collections/deque.h"

#include <stddef.h>
#include <stdint.h>

typedef struct
{
	int32_t x;
	int32_t y;
	uint32_t width;
	uint32_t height;
} Region;

struct Quadtree
{
	Region region;
	uint8_t maxDepth;
	uint8_t depth;
	// `Deque<QuadtreeEntry>`
	Deque entries;
	struct Quadtree* topLeft;
	struct Quadtree* topRight;
	struct Quadtree* bottomLeft;
	struct Quadtree* bottomRight;
};

typedef struct Quadtree Quadtree;

Quadtree* QuadtreeNew(Region region, uint8_t maxDepth);
bool QuadtreeAdd(Quadtree* self, size_t id, Region aabb);
Deque QuadtreeQuery(const Quadtree* self, Region region);
void QuadtreeClear(Quadtree* self);
void QuadtreeDestroy(Quadtree* self);
