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

int main(void)
{
    bool allPass = true;
    allPass &= ExecuteDequeTests();

    if (!allPass)
    {
        return 1;
    }
}
