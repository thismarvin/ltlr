#include <string.h>
#include "context.h"
#include "input.h"

static void ButtonBindingConsume(ButtonBinding* self);

ButtonBinding ButtonBindingCreate(char* name, usize keysCapacity, usize buttonsCapacity)
{
    return (ButtonBinding)
    {
        .name = name,
        .keys = calloc(keysCapacity, sizeof(KeyboardKey)),
        .keysCapacity = keysCapacity,
        .keysLength = 0,
        .buttons = calloc(buttonsCapacity, sizeof(GamepadButton)),
        .buttonsCapacity = buttonsCapacity,
        .buttonsLength = 0,
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

void ButtonBindingAddButton(ButtonBinding* self, GamepadButton button)
{
    if (self->buttonsLength == self->buttonsCapacity)
    {
        return;
    }

    self->buttons[self->buttonsLength] = button;

    self->buttonsLength += 1;
}

static void ButtonBindingUpdate(ButtonBinding* binding, usize gamepad)
{
    binding->bufferTimer += CTX_DT;

    bool released = true;

    for (usize i = 0; i < binding->keysLength; ++i)
    {
        if (IsKeyPressed(binding->keys[i]))
        {
            binding->bufferTimer = 0;
        }

        if (!IsKeyUp(binding->keys[i]))
        {
            released = false;
        }
    }

    if (IsGamepadAvailable(gamepad))
    {
        for (usize i = 0; i < binding->buttonsLength; ++i)
        {
            if (IsGamepadButtonPressed(gamepad, binding->buttons[i]))
            {
                binding->bufferTimer = 0;
            }

            if (!IsGamepadButtonUp(gamepad, binding->buttons[i]))
            {
                released = false;
            }
        }
    }

    if (released)
    {
        ButtonBindingConsume(binding);
    }
}

static bool ButtonBindingPressed(const ButtonBinding* binding, usize gamepad)
{
    if (binding->bufferDuration != 0 && binding->bufferTimer < binding->bufferDuration)
    {
        return true;
    }

    for (usize i = 0; i < binding->keysLength; ++i)
    {
        if (IsKeyPressed(binding->keys[i]))
        {
            return true;
        }
    }

    if (IsGamepadAvailable(gamepad))
    {
        for (usize i = 0; i < binding->buttonsLength; ++i)
        {
            if (IsGamepadButtonPressed(gamepad, binding->buttons[i]))
            {
                return true;
            }
        }
    }

    return false;
}

static bool ButtonBindingPressing(const ButtonBinding* binding, usize gamepad)
{
    for (usize i = 0; i < binding->keysLength; ++i)
    {
        if (IsKeyDown(binding->keys[i]))
        {
            return true;
        }
    }

    if (IsGamepadAvailable(gamepad))
    {
        for (usize i = 0; i < binding->buttonsLength; ++i)
        {
            if (IsGamepadButtonDown(gamepad, binding->buttons[i]))
            {
                return true;
            }
        }
    }

    return false;
}

static void ButtonBindingConsume(ButtonBinding* self)
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

InputHandler InputHandlerCreate(usize gamepad)
{
    return (InputHandler)
    {
        .gamepad = gamepad,
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
        ButtonBindingUpdate(&self->profile.bindings[i], self->gamepad);
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
            return ButtonBindingPressed(&self->profile.bindings[i], self->gamepad);
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
            return ButtonBindingPressing(&self->profile.bindings[i], self->gamepad);
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
