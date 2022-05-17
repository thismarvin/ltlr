#pragma once

#include "../src/collections/deque.h"
#include <stdlib.h>

typedef size_t usize;

typedef bool (*TestFn)(void);

typedef struct
{
    char* name;
    bool passed;
} TestRecord;

typedef struct
{
    char* name;
    Deque records;
    usize passed;
    usize failed;
} TestSuite;

TestSuite TestSuiteCreate(const char* name);
void TestSuiteAdd(TestSuite* self, const char* name, TestFn fn);
bool TestSuitePresentResults(TestSuite* self);
