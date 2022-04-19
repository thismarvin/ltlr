#pragma once

#include "common.h"
#include "components.h"

typedef struct
{
    usize entity;
    u64 tag;
} CommandSetTag;

typedef struct
{
    usize entity;
    Component component;
} CommandSetComponent;

typedef struct
{
    usize entity;
} CommandDeallocateEntity;

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
    CT_SET_TAG,
    CT_SET_COMPONENT,
    CT_DEALLOCATE_ENTITY,
    CT_ENABLE_COMPONENT,
    CT_DISABLE_COMPONENT,
} CommandType;

typedef struct
{
    CommandType type;
    union
    {
        CommandSetTag setTag;
        CommandSetComponent setComponent;
        CommandDeallocateEntity deallocateEntity;
        CommandEnableComponent enableComponent;
        CommandDisableComponent disableComponent;
    };
} Command;

Command CommandCreateSetTag(usize entity, u64 tag);
Command CommandCreateSetComponent(usize entity, const Component* component);
Command CommandCreateDeallocateEntity(usize entity);
Command CommandCreateEnableComponent(usize entity, u64 componentTag);
Command CommandCreateDisableComponent(usize entity, u64 componentTag);
