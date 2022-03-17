#include <string.h>
#include "context.h"
#include "input.h"

ButtonBinding ButtonBindingCreate(char* name, usize keysCapacity)
{
    return (ButtonBinding)
    {
        .name = name,
        .keys = calloc(keysCapacity, sizeof(KeyboardKey)),
        .keysCapacity = keysCapacity,
        .keysLength = 0,
        .bufferDuration = 0,
        .bufferTimer = 0
    };
}

void ButtonBindingSetBuffer(ButtonBinding* self, f32 bufferDuration)
{
    self->bufferDuration = bufferDuration;
    self->bufferTimer = bufferDuration;
}

void ButtonBindingAddKey(ButtonBinding* self, KeyboardKey key)
{
    if (self->keysLength == self->keysCapacity)
    {
        return;
    }

    self->keys[self->keysLength] = key;

    self->keysLength += 1;
}

void ButtonBindingUpdate(ButtonBinding* self)
{
    self->bufferTimer += CTX_DT;

    bool released = true;

    for (usize i = 0; i < self->keysLength; ++i)
    {
        if (IsKeyPressed(self->keys[i]))
        {
            self->bufferTimer = 0;
        }

        if (!IsKeyUp(self->keys[i]))
        {
            released = false;
        }
    }

    if (released)
    {
        ButtonBindingConsume(self);
    }
}

bool ButtonBindingPressed(const ButtonBinding* self)
{
    if (self->bufferDuration != 0 && self->bufferTimer < self->bufferDuration)
    {
        return true;
    }

    for (usize i = 0; i < self->keysLength; ++i)
    {
        if (IsKeyPressed(self->keys[i]))
        {
            return true;
        }
    }

    return false;
}

bool ButtonBindingPressing(const ButtonBinding* self)
{
    for (usize i = 0; i < self->keysLength; ++i)
    {
        if (IsKeyDown(self->keys[i]))
        {
            return true;
        }
    }

    return false;
}

void ButtonBindingConsume(ButtonBinding* self)
{
    self->bufferTimer = self->bufferDuration;
}

InputProfile InputProfileCreate(usize totalBindings)
{
    return (InputProfile)
    {
        .bindings = calloc(totalBindings, sizeof(ButtonBinding)),
        .bindingsCapacity = totalBindings,
        .bindingsLength = 0,
    };
}

void InputProfileAddBinding(InputProfile* self, ButtonBinding binding)
{
    if (self->bindingsLength == self->bindingsCapacity)
    {
        return;
    }

    self->bindings[self->bindingsLength] = binding;

    self->bindingsLength += 1;
}

InputHandler InputHandlerCreate(usize playerIndex)
{
    return (InputHandler)
    {
        .playerIndex = playerIndex,
        .enabled = false,
    };
}

void InputHandlerSetProfile(InputHandler* self, InputProfile profile)
{
    self->profile = profile;

    self->enabled = true;
}

void InputHandlerUpdate(InputHandler* self)
{
    if (!self->enabled)
    {
        return;
    }

    for (usize i = 0; i < self->profile.bindingsLength; ++i)
    {
        ButtonBindingUpdate(&self->profile.bindings[i]);
    }
}

bool InputHandlerPressed(const InputHandler* self, char* binding)
{
    if (!self->enabled)
    {
        return false;
    }

    for (usize i = 0; i < self->profile.bindingsLength; ++i)
    {
        if (strcmp(self->profile.bindings[i].name, binding) == 0)
        {
            return ButtonBindingPressed(&self->profile.bindings[i]);
        }
    }

    return false;
}

bool InputHandlerPressing(const InputHandler* self, char* binding)
{
    if (!self->enabled)
    {
        return false;
    }

    for (usize i = 0; i < self->profile.bindingsLength; ++i)
    {
        if (strcmp(self->profile.bindings[i].name, binding) == 0)
        {
            return ButtonBindingPressing(&self->profile.bindings[i]);
        }
    }

    return false;
}

void InputHandlerConsume(InputHandler* self, char* binding)
{
    if (!self->enabled)
    {
        return;
    }

    for (usize i = 0; i < self->profile.bindingsLength; ++i)
    {
        if (strcmp(self->profile.bindings[i].name, binding) == 0)
        {
            ButtonBindingConsume(&self->profile.bindings[i]);

            break;
        }
    }
}
