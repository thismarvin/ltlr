#include "quadtree.h"

typedef struct
{
	Region aabb;
	size_t id;
} QuadtreeEntry;

static int32_t RegionLeft(const Region self)
{
	return self.x;
}

static int32_t RegionRight(const Region self)
{
	return self.x + self.width;
}

static int32_t RegionBottom(const Region self)
{
	return self.y + self.height;
}

static int32_t RegionTop(const Region self)
{
	return self.y;
}

static bool RegionIntersects(const Region self, const Region other)
{
	return RegionLeft(self) < RegionRight(other) && RegionRight(self) > RegionLeft(other)
		   && RegionTop(self) < RegionBottom(other) && RegionBottom(self) > RegionTop(other);
}

static bool RegionContains(const Region self, const Region other)
{
	return RegionLeft(other) >= RegionLeft(self) && RegionRight(other) <= RegionRight(self)
		   && RegionTop(other) >= RegionTop(self) && RegionBottom(other) <= RegionBottom(self);
}

static Quadtree* New(const Region region, const uint8_t maxDepth, const uint8_t depth)
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

	const uint8_t depth = self->depth + 1;

	const int32_t width = self->region.width >> 1;
	const int32_t height = self->region.height >> 1;

	const Region topLeft = (Region) {
		.x = self->region.x,
		.y = self->region.y,
		.width = width,
		.height = height,
	};
	const Region topRight = (Region) {
		.x = self->region.x + width,
		.y = self->region.y,
		.width = width,
		.height = height,
	};
	const Region bottomLeft = (Region) {
		.x = self->region.x,
		.y = self->region.y + height,
		.width = width,
		.height = height,
	};
	const Region bottomRight = (Region) {
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

Quadtree* QuadtreeNew(const Region region, const uint8_t maxDepth)
{
	Quadtree* quadtree = New(region, maxDepth, 0);
	QuadtreeSubdivide(quadtree);

	return quadtree;
}

static bool QuadtreeIsLeaf(const Quadtree* self)
{
	return self->depth == self->maxDepth;
}

bool QuadtreeAdd(Quadtree* self, const size_t id, const Region aabb)
{
	if (!RegionContains(self->region, aabb))
	{
		return false;
	}

	if (QuadtreeIsLeaf(self))
	{
		const QuadtreeEntry entry = (QuadtreeEntry) {
			.id = id,
			.aabb = aabb,
		};
		DequePushBack(&self->entries, &entry);

		return true;
	}

	if (QuadtreeAdd(self->topLeft, id, aabb) || QuadtreeAdd(self->topRight, id, aabb)
		|| QuadtreeAdd(self->bottomRight, id, aabb) || QuadtreeAdd(self->bottomLeft, id, aabb))
	{
		return true;
	}

	const QuadtreeEntry entry = (QuadtreeEntry) {
		.id = id,
		.aabb = aabb,
	};
	DequePushBack(&self->entries, &entry);

	return true;
}

static void QuadtreeQueryHelper(const Quadtree* current, const Region region, Deque* result)
{
	if (!RegionIntersects(current->region, region))
	{
		return;
	}

	if (RegionContains(region, current->region))
	{
		for (size_t i = 0; i < DequeGetSize(&current->entries); ++i)
		{
			const QuadtreeEntry* entry = &DEQUE_GET_UNCHECKED(&current->entries, QuadtreeEntry, i);
			DequePushBack(result, &entry->id);
		}
	}
	else
	{
		for (size_t i = 0; i < DequeGetSize(&current->entries); ++i)
		{
			const QuadtreeEntry* entry = &DEQUE_GET_UNCHECKED(&current->entries, QuadtreeEntry, i);

			if (RegionIntersects(entry->aabb, region))
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
Deque QuadtreeQuery(const Quadtree* self, const Region region)
{
	Deque result = DEQUE_OF(size_t);
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
