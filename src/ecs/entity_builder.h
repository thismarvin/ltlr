#pragma once

#include "../collections/deque.h"
#include "../common.h"

typedef struct
{
    u64 tags;
    Deque components;
} EntityBuilder;
