#include "testing.h"
#include <stdio.h>

const char* evalPrompt = "\x1b[43m\x1b[30m EVAL \x1b[0m";
const char* passPrompt = "\x1b[42m\x1b[30m PASS \x1b[0m";
const char* failPrompt = "\x1b[41m\x1b[30m FAIL \x1b[0m";
const char* donePassPrompt = "\x1b[42m\x1b[30m DONE \x1b[0m";
const char* doneFailPrompt = "\x1b[41m\x1b[30m DONE \x1b[0m";

TestSuite TestSuiteCreate(const char* name)
{
    return (TestSuite)
    {
        .name = (char*)name,
        .records = DEQUE_OF(TestRecord),
        .passed = 0,
        .failed = 0,
    };
}

void TestSuiteAdd(TestSuite* self, const char* name, TestFn fn)
{
    bool passed = fn();

    TestRecord record = (TestRecord)
    {
        .name = (char*)name,
        .passed = passed,
    };

    DequePushFront(&self->records, &record);

    if (passed)
    {
        self->passed += 1;
    }
    else
    {
        self->failed += 1;
    }
}

bool TestSuitePresentResults(TestSuite* self)
{
    printf("%s %s\n", evalPrompt, self->name);

    for (usize i = 0; i < DequeGetSize(&self->records); ++i)
    {
        TestRecord record = DEQUE_GET_UNCHECKED(&self->records, TestRecord, i);

        if (record.passed)
        {
            printf("%s %s\n", passPrompt, record.name);
        }
        else
        {
            printf("%s %s\n", failPrompt, record.name);
        }
    }

    DequeDestroy(&self->records);

    if (self->failed == 0)
    {
        printf("%s %lu passed; %lu failed\n\n", donePassPrompt, self->passed, self->failed);
        return true;
    }
    else
    {
        printf("%s %lu passed; %lu failed\n\n", doneFailPrompt, self->passed, self->failed);
        return false;
    }
}
