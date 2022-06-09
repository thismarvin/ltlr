#include "../src/utils/quadtree.h"
#include "testing.h"
#include <stdint.h>

typedef int32_t i32;

static bool DequeTestCreate(void)
{
    Deque newDeque = DEQUE_OF(i32);
    usize size = DequeGetSize(&newDeque);
    DequeDestroy(&newDeque);

    return size == 0;
}

static bool DequeTestPushFront1(void)
{
    Deque deque = DEQUE_OF(i32);
    DEQUE_PUSH_FRONT(&deque, i32, 1);

    bool result = DequeGetSize(&deque) == 1 && DEQUE_GET(&deque, i32, 0) == 1;

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

    i32 expected[3] = { 1, 2, 3 };

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

    bool result = DequeGetSize(&deque) == 1 && DEQUE_GET(&deque, i32, 0) == 1;

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

    i32 expected[3] = { 3, 2, 1 };

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

    i32 expected[6] = { -2, -1, 0, 1, 2, 3 };

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
    i32 pop = DEQUE_POP_FRONT(&deque, i32);

    bool result = DequeGetSize(&deque) == 1 && pop == 2;

    DequeDestroy(&deque);
    return result;
}

static bool DequeTestPopBack(void)
{
    Deque deque = DEQUE_OF(i32);
    DEQUE_PUSH_FRONT(&deque, i32, 1);
    DEQUE_PUSH_FRONT(&deque, i32, 2);
    i32 pop = DEQUE_POP_BACK(&deque, i32);

    bool result = DequeGetSize(&deque) == 1 && pop == 1;

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

    i32 expected[] = {-2, -1, 0, 1, 2};

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
    const Rectangle region = (Rectangle)
    {
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
    const Rectangle region = (Rectangle)
    {
        .x = 0,
        .y = 0,
        .width = 100,
        .height = 100,
    };
    Quadtree* quadtree = QuadtreeNew(region, 4);

    usize totalAdded = 0;

    // Add something that is completely contained in the Quadtree's region (and small).
    {
        const Rectangle aabb = (Rectangle)
        {
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
        const Rectangle aabb = (Rectangle)
        {
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
        const Rectangle aabb = (Rectangle)
        {
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
        const Rectangle aabb = (Rectangle)
        {
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
    const Rectangle region = (Rectangle)
    {
        .x = 0,
        .y = 0,
        .width = 100,
        .height = 100,
    };
    Quadtree* quadtree = QuadtreeNew(region, 4);

    QuadtreeAdd(quadtree, 1, (Rectangle)
    {
        10, 10, 30, 30
    });
    QuadtreeAdd(quadtree, 2, (Rectangle)
    {
        60, 10, 30, 30
    });
    QuadtreeAdd(quadtree, 3, (Rectangle)
    {
        10, 60, 30, 30
    });
    QuadtreeAdd(quadtree, 4, (Rectangle)
    {
        5, 5, 80, 30
    });

    for (usize i = 0; i < 5; ++i)
    {
        QuadtreeAdd(quadtree, 5 + i, (Rectangle)
        {
            80 + i, 80 + i, 1, 1
        });
    }

    usize totalHits = 0;

    // Query over a single quadrant.
    {
        const Rectangle aabb = (Rectangle)
        {
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
        const Rectangle aabb = (Rectangle)
        {
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
        const Rectangle aabb = (Rectangle)
        {
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
        const Rectangle aabb = (Rectangle)
        {
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
    const Rectangle region = (Rectangle)
    {
        .x = 0,
        .y = 0,
        .width = 100,
        .height = 100,
    };
    Quadtree* quadtree = QuadtreeNew(region, 4);

    for (usize i = 0; i < 50; ++i)
    {
        QuadtreeAdd(quadtree, i, (Rectangle)
        {
            i, 0, 16, 16
        });
    }

    usize totalHits = 0;

    // Query over the entire Quadtree.
    {
        const Rectangle aabb = (Rectangle)
        {
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
        const Rectangle aabb = (Rectangle)
        {
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
            QuadtreeAdd(quadtree, entityIndex + i, (Rectangle)
            {
                25 + i, 60, 16, 16
            });

            entityIndex += 1;
        }

        for (usize i = 0; i < 50; ++i)
        {
            QuadtreeAdd(quadtree, entityIndex + i, (Rectangle)
            {
                25 + i, 0, 16, 16
            });

            entityIndex += 1;
        }
    }

    // Query over the bottom half of the Quadtree.
    {
        const Rectangle aabb = (Rectangle)
        {
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
        return 1;
    }
}
