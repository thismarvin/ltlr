#include <string.h>
#include "context.h"
#include "input.h"

static void KeyboardBindingConsume(KeyboardBinding* binding)
{
    binding->bufferTimer = binding->bufferDuration;
}

static void GamepadBindingConsume(GamepadBinding* binding, usize gamepad)
{
    if (!IsGamepadAvailable(gamepad))
    {
        return;
    }

    binding->bufferTimer = binding->bufferDuration;
}

static void MouseBindingConsume(MouseBinding* binding)
{
    binding->bufferTimer = binding->bufferDuration;
}

KeyboardBinding KeyboardBindingCreate(char* name, usize keysCapacity)
{
    return (KeyboardBinding)
    {
        .name = name,
        .keys = calloc(keysCapacity, sizeof(KeyboardKey)),
        .keysCapacity = keysCapacity,
        .keysLength = 0,
        .bufferDuration = 0,
        .bufferTimer = 0
    };
}

void KeyboardBindingSetBuffer(KeyboardBinding* self, f32 bufferDuration)
{
    self->bufferDuration = bufferDuration;
    self->bufferTimer = bufferDuration;
}

void KeyboardBindingAddKey(KeyboardBinding* self, KeyboardKey key)
{
    if (self->keysLength == self->keysCapacity)
    {
        return;
    }

    self->keys[self->keysLength] = key;

    self->keysLength += 1;
}

GamepadBinding GamepadBindingCreate(char* name, usize buttonsCapacity)
{
    return (GamepadBinding)
    {
        .name = name,
        .buttons = calloc(buttonsCapacity, sizeof(GamepadButton)),
        .buttonsCapacity = buttonsCapacity,
        .buttonsLength = 0,
        .bufferDuration = 0,
        .bufferTimer = 0,
    };
}

void GamepadBindingSetBuffer(GamepadBinding* self, f32 bufferDuration)
{
    self->bufferDuration = bufferDuration;
    self->bufferTimer = bufferDuration;
}

void GamepadBindingAddButton(GamepadBinding* self, GamepadButton button)
{
    if (self->buttonsLength == self->buttonsCapacity)
    {
        return;
    }

    self->buttons[self->buttonsLength] = button;

    self->buttonsLength += 1;
}

MouseBinding MouseBindingCreate(char* name, usize buttonsCapacity)
{
    return (MouseBinding)
    {
        .name = name,
        .buttons = calloc(buttonsCapacity, sizeof(MouseButton)),
        .buttonsCapacity = buttonsCapacity,
        .buttonsLength = 0,
        .bufferDuration = 0,
        .bufferTimer = 0,
    };
}

void MouseBindingSetBuffer(MouseBinding* self, f32 bufferDuration)
{
    self->bufferDuration = bufferDuration;
    self->bufferTimer = bufferDuration;
}

void MouseBindingAddButton(MouseBinding* self, MouseButton button)
{
    if (self->buttonsLength == self->buttonsCapacity)
    {
        return;
    }

    self->buttons[self->buttonsLength] = button;

    self->buttonsLength += 1;
}

static void KeyboardBindingUpdate(KeyboardBinding* binding)
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

    if (released)
    {
        KeyboardBindingConsume(binding);
    }
}

static void GamepadBindingUpdate(GamepadBinding* binding, usize gamepad)
{
    if (!IsGamepadAvailable(gamepad))
    {
        return;
    }

    binding->bufferTimer += CTX_DT;

    bool released = true;

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

    if (released)
    {
        GamepadBindingConsume(binding, gamepad);
    }
}

static void MouseBindingUpdate(MouseBinding* binding)
{
    binding->bufferTimer += CTX_DT;

    bool released = true;

    for (usize i = 0; i < binding->buttonsLength; ++i)
    {
        if (IsMouseButtonPressed(binding->buttons[i]))
        {
            binding->bufferTimer = 0;
        }

        if (!IsMouseButtonUp(binding->buttons[i]))
        {
            released = false;
        }
    }

    if (released)
    {
        MouseBindingConsume(binding);
    }
}

static bool KeyboardBindingPressed(const KeyboardBinding* binding)
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

    return false;
}

static bool GamepadBindingPressed(const GamepadBinding* binding, usize gamepad)
{
    if (!IsGamepadAvailable(gamepad))
    {
        return false;
    }

    if (binding->bufferDuration != 0 && binding->bufferTimer < binding->bufferDuration)
    {
        return true;
    }

    for (usize i = 0; i < binding->buttonsLength; ++i)
    {
        if (IsGamepadButtonPressed(gamepad, binding->buttons[i]))
        {
            return true;
        }
    }

    return false;
}

static bool MouseBindingPressed(const MouseBinding* binding)
{
    if (binding->bufferDuration != 0 && binding->bufferTimer < binding->bufferDuration)
    {
        return true;
    }

    for (usize i = 0; i < binding->buttonsLength; ++i)
    {
        if (IsMouseButtonPressed(binding->buttons[i]))
        {
            return true;
        }
    }

    return false;
}

static bool KeyboardBindingPressing(const KeyboardBinding* binding)
{
    for (usize i = 0; i < binding->keysLength; ++i)
    {
        if (IsKeyDown(binding->keys[i]))
        {
            return true;
        }
    }

    return false;
}

static bool GamepadBindingPressing(const GamepadBinding* binding, usize gamepad)
{
    if (!IsGamepadAvailable(gamepad))
    {
        return false;
    }

    for (usize i = 0; i < binding->buttonsLength; ++i)
    {
        if (IsGamepadButtonDown(gamepad, binding->buttons[i]))
        {
            return true;
        }
    }

    return false;
}

static bool MouseBindingPressing(const MouseBinding* binding)
{
    for (usize i = 0; i < binding->buttonsLength; ++i)
    {
        if (IsMouseButtonDown(binding->buttons[i]))
        {
            return true;
        }
    }

    return false;
}

InputProfile InputProfileCreate(usize bindingsCapactity)
{
    return (InputProfile)
    {
        .bindingsCapacity = bindingsCapactity,
        .keyboardBindings = calloc(bindingsCapactity, sizeof(KeyboardBinding)),
        .keyboardBindingsLength = 0,
        .gamepadBindings = calloc(bindingsCapactity, sizeof(GamepadBinding)),
        .gamepadBindingsLength = 0,
        .mouseBindings = calloc(bindingsCapactity, sizeof(MouseBinding)),
        .mouseBindingsLength = 0,
    };
}

void InputProfileAddKeyboardBinding(InputProfile* self, KeyboardBinding binding)
{
    if (self->keyboardBindingsLength == self->bindingsCapacity)
    {
        return;
    }

    self->keyboardBindings[self->keyboardBindingsLength] = binding;

    self->keyboardBindingsLength += 1;
}

void InputProfileAddGamepadBinding(InputProfile* self, GamepadBinding binding)
{
    if (self->gamepadBindingsLength == self->bindingsCapacity)
    {
        return;
    }

    self->gamepadBindings[self->gamepadBindingsLength] = binding;

    self->gamepadBindingsLength += 1;
}

void InputProfileAddMouseBinding(InputProfile* self, MouseBinding binding)
{
    if (self->gamepadBindingsLength == self->bindingsCapacity)
    {
        return;
    }

    self->mouseBindings[self->mouseBindingsLength] = binding;

    self->mouseBindingsLength += 1;
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

    for (usize i = 0; i < self->profile.keyboardBindingsLength; ++i)
    {
        KeyboardBindingUpdate(&self->profile.keyboardBindings[i]);
    }

    for (usize i = 0; i < self->profile.gamepadBindingsLength; ++i)
    {
        GamepadBindingUpdate(&self->profile.gamepadBindings[i], self->gamepad);
    }

    for (usize i = 0; i < self->profile.mouseBindingsLength; ++i)
    {
        MouseBindingUpdate(&self->profile.mouseBindings[i]);
    }
}

bool InputHandlerPressed(const InputHandler* self, char* binding)
{
    if (!self->enabled)
    {
        return false;
    }

    for (usize i = 0; i < self->profile.keyboardBindingsLength; ++i)
    {
        if (strcmp(self->profile.keyboardBindings[i].name, binding) == 0)
        {
            if (KeyboardBindingPressed(&self->profile.keyboardBindings[i]))
            {
                return true;
            }
        }
    }

    for (usize i = 0; i < self->profile.gamepadBindingsLength; ++i)
    {
        if (strcmp(self->profile.gamepadBindings[i].name, binding) == 0)
        {
            if (GamepadBindingPressed(&self->profile.gamepadBindings[i], self->gamepad))
            {
                return true;
            }
        }
    }

    for (usize i = 0; i < self->profile.mouseBindingsLength; ++i)
    {
        if (strcmp(self->profile.mouseBindings[i].name, binding) == 0)
        {
            if (MouseBindingPressed(&self->profile.mouseBindings[i]))
            {
                return true;
            }
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

    for (usize i = 0; i < self->profile.keyboardBindingsLength; ++i)
    {
        if (strcmp(self->profile.keyboardBindings[i].name, binding) == 0)
        {
            if (KeyboardBindingPressing(&self->profile.keyboardBindings[i]))
            {
                return true;
            }
        }
    }

    for (usize i = 0; i < self->profile.gamepadBindingsLength; ++i)
    {
        if (strcmp(self->profile.gamepadBindings[i].name, binding) == 0)
        {
            if (GamepadBindingPressing(&self->profile.gamepadBindings[i], self->gamepad))
            {
                return true;
            }
        }
    }

    for (usize i = 0; i < self->profile.mouseBindingsLength; ++i)
    {
        if (strcmp(self->profile.mouseBindings[i].name, binding) == 0)
        {
            if (MouseBindingPressing(&self->profile.mouseBindings[i]))
            {
                return true;
            }
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

    for (usize i = 0; i < self->profile.keyboardBindingsLength; ++i)
    {
        if (strcmp(self->profile.keyboardBindings[i].name, binding) == 0)
        {
            KeyboardBindingConsume(&self->profile.keyboardBindings[i]);

            break;
        }
    }

    for (usize i = 0; i < self->profile.gamepadBindingsLength; ++i)
    {
        if (strcmp(self->profile.gamepadBindings[i].name, binding) == 0)
        {
            GamepadBindingConsume(&self->profile.gamepadBindings[i], self->gamepad);

            break;
        }
    }

    for (usize i = 0; i < self->profile.mouseBindingsLength; ++i)
    {
        if (strcmp(self->profile.mouseBindings[i].name, binding) == 0)
        {
            MouseBindingConsume(&self->profile.mouseBindings[i]);

            break;
        }
    }
}
