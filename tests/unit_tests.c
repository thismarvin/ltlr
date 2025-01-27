#include "../src/collections/deque.h"
#include "../src/utils/quadtree.h"
#include "testing.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef int32_t i32;

static bool DequeTestCreate(void)
{
	Deque newDeque = DEQUE_OF(i32);
	const usize size = DequeGetSize(&newDeque);
	DequeDestroy(&newDeque);

	return size == 0;
}

static bool DequeTestPushFront1(void)
{
	Deque deque = DEQUE_OF(i32);
	DEQUE_PUSH_FRONT(&deque, i32, 1);

	const bool result = DequeGetSize(&deque) == 1 && DEQUE_GET(&deque, i32, 0) == 1;

	DequeDestroy(&deque);
	return result;
}

static bool DequeTestPushFrontMultiple(void)
{
	Deque deque = DEQUE_OF(i32);
	DEQUE_PUSH_FRONT(&deque, i32, 1);
	DEQUE_PUSH_FRONT(&deque, i32, 2);
	DEQUE_PUSH_FRONT(&deque, i32, 3);

	bool result = DequeGetSize(&deque) == 3;

	const i32 expected[3] = { 1, 2, 3 };

	for (int i = 0; i < 3; ++i)
	{
		result &= DEQUE_GET(&deque, i32, i) == expected[i];
	}

	DequeDestroy(&deque);
	return result;
}

static bool DequeTestPushBack1(void)
{
	Deque deque = DEQUE_OF(i32);
	DEQUE_PUSH_BACK(&deque, i32, 1);

	const bool result = DequeGetSize(&deque) == 1 && DEQUE_GET(&deque, i32, 0) == 1;

	DequeDestroy(&deque);
	return result;
}

static bool DequeTestPushBackMultiple(void)
{
	Deque deque = DEQUE_OF(i32);
	DEQUE_PUSH_BACK(&deque, i32, 1);
	DEQUE_PUSH_BACK(&deque, i32, 2);
	DEQUE_PUSH_BACK(&deque, i32, 3);

	bool result = DequeGetSize(&deque) == 3;

	const i32 expected[3] = { 3, 2, 1 };

	for (int i = 0; i < 3; ++i)
	{
		result &= DEQUE_GET(&deque, i32, i) == expected[i];
	}

	DequeDestroy(&deque);
	return result;
}

static bool DequeTestPushFrontAndBack(void)
{
	Deque deque = DEQUE_OF(i32);
	DEQUE_PUSH_FRONT(&deque, i32, 1);
	DEQUE_PUSH_BACK(&deque, i32, 0);
	DEQUE_PUSH_BACK(&deque, i32, -1);
	DEQUE_PUSH_FRONT(&deque, i32, 2);
	DEQUE_PUSH_FRONT(&deque, i32, 3);
	DEQUE_PUSH_BACK(&deque, i32, -2);

	bool result = DequeGetSize(&deque) == 6;

	const i32 expected[6] = { -2, -1, 0, 1, 2, 3 };

	for (int i = 0; i < 6; ++i)
	{
		result &= DEQUE_GET(&deque, i32, i) == expected[i];
	}

	DequeDestroy(&deque);
	return result;
}

static bool DequeTestPopFront(void)
{
	Deque deque = DEQUE_OF(i32);
	DEQUE_PUSH_FRONT(&deque, i32, 1);
	DEQUE_PUSH_FRONT(&deque, i32, 2);
	const i32 pop = DEQUE_POP_FRONT(&deque, i32);

	const bool result = DequeGetSize(&deque) == 1 && pop == 2;

	DequeDestroy(&deque);
	return result;
}

static bool DequeTestPopBack(void)
{
	Deque deque = DEQUE_OF(i32);
	DEQUE_PUSH_FRONT(&deque, i32, 1);
	DEQUE_PUSH_FRONT(&deque, i32, 2);
	const i32 pop = DEQUE_POP_BACK(&deque, i32);

	const bool result = DequeGetSize(&deque) == 1 && pop == 1;

	DequeDestroy(&deque);
	return result;
}

static bool DequeTestClear(void)
{
	Deque deque = DEQUE_OF(i32);
	DEQUE_PUSH_FRONT(&deque, i32, 1);
	DEQUE_PUSH_BACK(&deque, i32, 0);
	DEQUE_PUSH_BACK(&deque, i32, -1);
	DEQUE_PUSH_FRONT(&deque, i32, 2);
	DEQUE_PUSH_FRONT(&deque, i32, 3);
	DEQUE_PUSH_BACK(&deque, i32, -2);

	bool result = DequeGetSize(&deque) == 6;

	DequeClear(&deque);

	result &= DequeGetSize(&deque) == 0;

	DequeDestroy(&deque);
	return result;
}

static bool DequeTestTailIsTail(void)
{
	Deque deque = DEQUE_OF(i32);
	DEQUE_PUSH_FRONT(&deque, i32, 1);
	DEQUE_PUSH_FRONT(&deque, i32, 2);
	DEQUE_PUSH_FRONT(&deque, i32, 3);
	DEQUE_PUSH_FRONT(&deque, i32, 4);
	DEQUE_PUSH_FRONT(&deque, i32, 5);

	i32 pop = DEQUE_POP_BACK(&deque, i32);

	bool result = pop == 1;

	pop = DEQUE_POP_BACK(&deque, i32);

	result &= pop == 2;

	DequeDestroy(&deque);
	return result;
}

static bool DequeTestHeadIsHead(void)
{
	Deque deque = DEQUE_OF(i32);
	DEQUE_PUSH_BACK(&deque, i32, 1);
	DEQUE_PUSH_BACK(&deque, i32, 2);
	DEQUE_PUSH_BACK(&deque, i32, 3);
	DEQUE_PUSH_BACK(&deque, i32, 4);
	DEQUE_PUSH_BACK(&deque, i32, 5);

	i32 pop = DEQUE_POP_FRONT(&deque, i32);

	bool result = pop == 1;

	pop = DEQUE_POP_FRONT(&deque, i32);

	result &= pop == 2;

	DequeDestroy(&deque);
	return result;
}

static bool DequeTestResize(void)
{
	Deque deque = DEQUE_WITH_CAPACITY(i32, 4);
	DEQUE_PUSH_FRONT(&deque, i32, 1);
	DEQUE_PUSH_FRONT(&deque, i32, 2);
	DEQUE_PUSH_BACK(&deque, i32, 0);
	DEQUE_PUSH_BACK(&deque, i32, -1);

	bool result = deque.m_capacity == 4;

	DEQUE_PUSH_BACK(&deque, i32, -2);

	result &= deque.m_capacity > 4;

	const i32 expected[] = { -2, -1, 0, 1, 2 };

	for (int i = 0; i < 5; ++i)
	{
		result &= DEQUE_GET(&deque, i32, i) == expected[i];
	}

	DequeDestroy(&deque);
	return result;
}

static bool ExecuteDequeTests(void)
{
	TestSuite suite = TestSuiteCreate("Deque Tests");

	TestSuiteAdd(&suite, "Deque starts with size 0", DequeTestCreate);
	TestSuiteAdd(&suite, "Push front 1 element", DequeTestPushFront1);
	TestSuiteAdd(&suite, "Push front multiple elements", DequeTestPushFrontMultiple);
	TestSuiteAdd(&suite, "Push back 1 element", DequeTestPushBack1);
	TestSuiteAdd(&suite, "Push back multiple elements", DequeTestPushBackMultiple);
	TestSuiteAdd(&suite, "Push front and back", DequeTestPushFrontAndBack);
	TestSuiteAdd(&suite, "Pop front", DequeTestPopFront);
	TestSuiteAdd(&suite, "Pop back", DequeTestPopBack);
	TestSuiteAdd(&suite, "Clear deque", DequeTestClear);
	TestSuiteAdd(&suite, "Tail always points to start", DequeTestTailIsTail);
	TestSuiteAdd(&suite, "Head always points to end", DequeTestHeadIsHead);
	TestSuiteAdd(&suite, "Resize expands capacity and retains order", DequeTestResize);

	return TestSuitePresentResults(&suite);
}

static bool TestQuadtreeNew(void)
{
	const Region region = (Region) {
		.x = 0,
		.y = 0,
		.width = 100,
		.height = 100,
	};
	Quadtree* quadtree = QuadtreeNew(region, 4);

	QuadtreeDestroy(quadtree);

	return true;
}

static bool TestQuadtreeAdd(void)
{
	const Region region = (Region) {
		.x = 0,
		.y = 0,
		.width = 100,
		.height = 100,
	};
	Quadtree* quadtree = QuadtreeNew(region, 4);

	usize totalAdded = 0;

	// Add something that is completely contained in the Quadtree's region (and small).
	{
		const Region aabb = (Region) {
			.x = 8,
			.y = 8,
			.width = 4,
			.height = 8,
		};

		if (QuadtreeAdd(quadtree, 1, aabb))
		{
			totalAdded += 1;
		}
	}
	// Add something that is completely contained in the Quadtree's region (and very wide).
	{
		const Region aabb = (Region) {
			.x = 15,
			.y = 75,
			.width = 60,
			.height = 10,
		};

		if (QuadtreeAdd(quadtree, 1, aabb))
		{
			totalAdded += 1;
		}
	}
	// Add something that is partially within the Quadtree's region.
	{
		const Region aabb = (Region) {
			.x = 80,
			.y = 16,
			.width = 64,
			.height = 32,
		};

		if (QuadtreeAdd(quadtree, 2, aabb))
		{
			totalAdded += 1;
		}
	}
	// Attempt to add something that is completely outside the Quadtree's region.
	{
		const Region aabb = (Region) {
			.x = 58008,
			.y = 1337,
			.width = 42,
			.height = 42,
		};

		if (QuadtreeAdd(quadtree, 3, aabb))
		{
			totalAdded += 1;
		}
	}

	QuadtreeDestroy(quadtree);

	return totalAdded == 1 + 1 + 0 + 0;
}

static bool TestQuadtreeQuery(void)
{
	const Region region = (Region) {
		.x = 0,
		.y = 0,
		.width = 100,
		.height = 100,
	};
	Quadtree* quadtree = QuadtreeNew(region, 4);

	QuadtreeAdd(quadtree, 1, (Region) { 10, 10, 30, 30 });
	QuadtreeAdd(quadtree, 2, (Region) { 60, 10, 30, 30 });
	QuadtreeAdd(quadtree, 3, (Region) { 10, 60, 30, 30 });
	QuadtreeAdd(quadtree, 4, (Region) { 5, 5, 80, 30 });

	for (usize i = 0; i < 5; ++i)
	{
		QuadtreeAdd(quadtree, 5 + i, (Region) { 80 + i, 80 + i, 1, 1 });
	}

	usize totalHits = 0;

	// Query over a single quadrant.
	{
		const Region aabb = (Region) {
			.x = 50,
			.y = 0,
			.width = 50,
			.height = 50,
		};
		Deque queryResults = QuadtreeQuery(quadtree, aabb);

		totalHits += DequeGetSize(&queryResults);

		DequeDestroy(&queryResults);
	}
	// Query over multiple quadrants.
	{
		const Region aabb = (Region) {
			.x = 0,
			.y = 0,
			.width = 100,
			.height = 50,
		};
		Deque queryResults = QuadtreeQuery(quadtree, aabb);

		totalHits += DequeGetSize(&queryResults);

		DequeDestroy(&queryResults);
	}
	// Query over an empty region.
	{
		const Region aabb = (Region) {
			.x = 0,
			.y = 90,
			.width = 100,
			.height = 200,
		};
		Deque queryResults = QuadtreeQuery(quadtree, aabb);

		totalHits += DequeGetSize(&queryResults);

		DequeDestroy(&queryResults);
	}
	// Query over a region completely outside of the Quadtree's region.
	{
		const Region aabb = (Region) {
			.x = 111111,
			.y = 101110,
			.width = 500,
			.height = 500,
		};
		Deque queryResults = QuadtreeQuery(quadtree, aabb);

		totalHits += DequeGetSize(&queryResults);

		DequeDestroy(&queryResults);
	}

	QuadtreeDestroy(quadtree);

	return totalHits == 2 + 3 + 0 + 0;
}

static bool TestQuadtreeClear(void)
{
	const Region region = (Region) {
		.x = 0,
		.y = 0,
		.width = 100,
		.height = 100,
	};
	Quadtree* quadtree = QuadtreeNew(region, 4);

	for (usize i = 0; i < 50; ++i)
	{
		QuadtreeAdd(quadtree, i, (Region) { i, 0, 16, 16 });
	}

	usize totalHits = 0;

	// Query over the entire Quadtree.
	{
		const Region aabb = (Region) {
			.x = -100,
			.y = -100,
			.width = 12345,
			.height = 12345,
		};
		Deque queryResults = QuadtreeQuery(quadtree, aabb);

		totalHits += DequeGetSize(&queryResults);

		DequeDestroy(&queryResults);
	}

	QuadtreeClear(quadtree);

	// Query over the entire (empty) Quadtree.
	{
		const Region aabb = (Region) {
			.x = -100,
			.y = -100,
			.width = 12345,
			.height = 12345,
		};
		Deque queryResults = QuadtreeQuery(quadtree, aabb);

		totalHits += DequeGetSize(&queryResults);

		DequeDestroy(&queryResults);
	}

	// Attempt to repurpose the existing Quadtree.
	{
		usize entityIndex = 1;

		for (usize i = 0; i < 50; ++i)
		{
			QuadtreeAdd(quadtree, entityIndex + i, (Region) { 25 + i, 60, 16, 16 });

			entityIndex += 1;
		}

		for (usize i = 0; i < 50; ++i)
		{
			QuadtreeAdd(quadtree, entityIndex + i, (Region) { 25 + i, 0, 16, 16 });

			entityIndex += 1;
		}
	}

	// Query over the bottom half of the Quadtree.
	{
		const Region aabb = (Region) {
			.x = 0,
			.y = 50,
			.width = 100,
			.height = 100,
		};
		Deque queryResults = QuadtreeQuery(quadtree, aabb);

		totalHits += DequeGetSize(&queryResults);

		DequeDestroy(&queryResults);
	}

	QuadtreeDestroy(quadtree);

	return totalHits == 50 + 0 + 50;
}

static bool ExecuteQuadtreeTests(void)
{
	TestSuite suite = TestSuiteCreate("Quadtree Tests");

	TestSuiteAdd(&suite, "Create an empty Quadtree", TestQuadtreeNew);
	TestSuiteAdd(&suite, "Add entries to a Quadtree", TestQuadtreeAdd);
	TestSuiteAdd(&suite, "Query a Quadtree", TestQuadtreeQuery);
	TestSuiteAdd(&suite, "Clear a Quadtree", TestQuadtreeClear);

	return TestSuitePresentResults(&suite);
}

int main(void)
{
	bool allPass = true;

	allPass &= ExecuteDequeTests();
	allPass &= ExecuteQuadtreeTests();

	if (!allPass)
	{
		return EXIT_FAILURE;
	}
}
