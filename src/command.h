#pragma once

#include "common.h"
#include "components.h"

typedef struct
{
    usize entity;
    Component component;
} CommandSetComponent;

typedef struct
{
    usize entity;
    u64 componentTag;
} CommandEnableComponent;

typedef struct
{
    usize entity;
    u64 componentTag;
} CommandDisableComponent;

typedef enum
{
    CT_NONE,
    CT_SET_COMPONENT,
    CT_ENABLE_COMPONENT,
    CT_DISABLE_COMPONENT,
} CommandType;

typedef struct
{
    CommandType type;
    union
    {
        CommandSetComponent setComponent;
        CommandEnableComponent enableComponent;
        CommandDisableComponent disableComponent;
    };
} Command;

Command CommandCreateSetComponent(usize entity, const Component* component);
Command CommandCreateEnableComponent(usize entity, u64 componentTag);
Command CommandCreateDisableComponent(usize entity, u64 componentTag);
