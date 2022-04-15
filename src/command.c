#include "command.h"

Command CommandCreateSetComponent(const usize entity, const Component* component)
{
    return (Command)
    {
        .type = CT_SET_COMPONENT,
        .setComponent = (CommandSetComponent)
        {
            .entity = entity,
            .component = *component,
        },
    };
}

Command CommandCreateEnableComponent(const usize entity, const u64 componentTag)
{
    return (Command)
    {
        .type = CT_ENABLE_COMPONENT,
        .enableComponent = (CommandEnableComponent)
        {
            .entity = entity,
            .componentTag = componentTag,
        },
    };
}

Command CommandCreateDisableComponent(const usize entity, const u64 componentTag)
{
    return (Command)
    {
        .type = CT_DISABLE_COMPONENT,
        .disableComponent = (CommandDisableComponent)
        {
            .entity = entity,
            .componentTag = componentTag,
        },
    };
}
