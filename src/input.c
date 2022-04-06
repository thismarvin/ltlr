#include "context.h"
#include "input.h"
#include <string.h>

static void KeyboardBindingConsume(KeyboardBinding* binding)
{
    binding->bufferTimer = binding->bufferDuration;
}

static void GamepadBindingConsume(GamepadBinding* binding, const usize gamepad)
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

KeyboardBinding KeyboardBindingCreate(const char* name, const usize keysCapacity)
{
    return (KeyboardBinding)
    {
        .name = (char*)name,
        .keys = calloc(keysCapacity, sizeof(KeyboardKey)),
        .keysCapacity = keysCapacity,
        .keysLength = 0,
        .bufferDuration = 0,
        .bufferTimer = 0
    };
}

void KeyboardBindingSetBuffer(KeyboardBinding* self, const f32 bufferDuration)
{
    self->bufferDuration = bufferDuration;
    self->bufferTimer = bufferDuration;
}

void KeyboardBindingAddKey(KeyboardBinding* self, const KeyboardKey key)
{
    if (self->keysLength >= self->keysCapacity)
    {
        return;
    }

    self->keys[self->keysLength] = key;

    self->keysLength += 1;
}

GamepadBinding GamepadBindingCreate(const char* name, const usize buttonsCapacity)
{
    return (GamepadBinding)
    {
        .name = (char*)name,
        .buttons = calloc(buttonsCapacity, sizeof(GamepadButton)),
        .buttonsCapacity = buttonsCapacity,
        .buttonsLength = 0,
        .bufferDuration = 0,
        .bufferTimer = 0,
    };
}

void GamepadBindingSetBuffer(GamepadBinding* self, const f32 bufferDuration)
{
    self->bufferDuration = bufferDuration;
    self->bufferTimer = bufferDuration;
}

void GamepadBindingAddButton(GamepadBinding* self, const GamepadButton button)
{
    if (self->buttonsLength >= self->buttonsCapacity)
    {
        return;
    }

    self->buttons[self->buttonsLength] = button;

    self->buttonsLength += 1;
}

MouseBinding MouseBindingCreate(const char* name, const usize buttonsCapacity)
{
    return (MouseBinding)
    {
        .name = (char*)name,
        .buttons = calloc(buttonsCapacity, sizeof(MouseButton)),
        .buttonsCapacity = buttonsCapacity,
        .buttonsLength = 0,
        .bufferDuration = 0,
        .bufferTimer = 0,
    };
}

void MouseBindingSetBuffer(MouseBinding* self, const f32 bufferDuration)
{
    self->bufferDuration = bufferDuration;
    self->bufferTimer = bufferDuration;
}

void MouseBindingAddButton(MouseBinding* self, const MouseButton button)
{
    if (self->buttonsLength >= self->buttonsCapacity)
    {
        return;
    }

    self->buttons[self->buttonsLength] = button;

    self->buttonsLength += 1;
}

AxisBinding AxisBindingCreate
(
    const char* name,
    const usize axesCapacity,
    const Ordering ordering,
    const f32 target
)
{
    return (AxisBinding)
    {
        .name = (char*)name,
        .axes = calloc(axesCapacity, sizeof(GamepadAxis)),
        .axesCapacity = axesCapacity,
        .axesLength = 0,
        .ordering = ordering,
        .target = target,
    };
}

void AxisBindingAddAxis(AxisBinding* self, const GamepadAxis axis)
{
    if (self->axesLength >= self->axesCapacity)
    {
        return;
    }

    self->axes[self->axesLength] = axis;

    self->axesLength += 1;
}

static void KeyboardBindingUpdate(KeyboardBinding* binding)
{
    binding->bufferTimer += CTX_DT;

    bool inactive = true;

    for (usize i = 0; i < binding->keysLength; ++i)
    {
        if (IsKeyPressed(binding->keys[i]))
        {
            binding->bufferTimer = 0;
        }

        if (IsKeyReleased(binding->keys[i]))
        {
            binding->bufferTimer = 0;
        }

        if (!IsKeyUp(binding->keys[i]))
        {
            inactive = false;
        }
    }

    if (inactive)
    {
        KeyboardBindingConsume(binding);
    }
}

static void GamepadBindingUpdate(GamepadBinding* binding, const usize gamepad)
{
    if (!IsGamepadAvailable(gamepad))
    {
        return;
    }

    binding->bufferTimer += CTX_DT;

    bool inactive = true;

    for (usize i = 0; i < binding->buttonsLength; ++i)
    {
        if (IsGamepadButtonPressed(gamepad, binding->buttons[i]))
        {
            binding->bufferTimer = 0;
        }

        if (IsGamepadButtonReleased(gamepad, binding->buttons[i]))
        {
            binding->bufferTimer = 0;
        }

        if (!IsGamepadButtonUp(gamepad, binding->buttons[i]))
        {
            inactive = false;
        }
    }

    if (inactive)
    {
        GamepadBindingConsume(binding, gamepad);
    }
}

static void MouseBindingUpdate(MouseBinding* binding)
{
    binding->bufferTimer += CTX_DT;

    bool inactive = true;

    for (usize i = 0; i < binding->buttonsLength; ++i)
    {
        if (IsMouseButtonPressed(binding->buttons[i]))
        {
            binding->bufferTimer = 0;
        }

        if (IsMouseButtonReleased(binding->buttons[i]))
        {
            binding->bufferTimer = 0;
        }

        if (!IsMouseButtonUp(binding->buttons[i]))
        {
            inactive = false;
        }
    }

    if (inactive)
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

static bool GamepadBindingPressed(const GamepadBinding* binding, const usize gamepad)
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

static bool GamepadBindingPressing(const GamepadBinding* binding, const usize gamepad)
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

static bool AxisBindingPressing(const AxisBinding* binding, const usize gamepad)
{
    if (!IsGamepadAvailable(gamepad))
    {
        return false;
    }

    for (usize i = 0; i < binding->axesLength; ++i)
    {
        f32 value = GetGamepadAxisMovement(gamepad, binding->axes[i]);

        if (binding->ordering == ORD_LESS && value < binding->target)
        {
            return true;
        }

        if (binding->ordering == ORD_GREATER && value > binding->target)
        {
            return true;
        }
    }

    return false;
}


static bool KeyboardBindingReleased(const KeyboardBinding* binding)
{
    if (binding->bufferDuration != 0 && binding->bufferTimer < binding->bufferDuration)
    {
        return true;
    }

    for (usize i = 0; i < binding->keysLength; ++i)
    {
        if (IsKeyReleased(binding->keys[i]))
        {
            return true;
        }
    }

    return false;
}

static bool GamepadBindingReleased(const GamepadBinding* binding, const usize gamepad)
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
        if (IsGamepadButtonReleased(gamepad, binding->buttons[i]))
        {
            return true;
        }
    }

    return false;
}

static bool MouseBindingReleased(const MouseBinding* binding)
{
    if (binding->bufferDuration != 0 && binding->bufferTimer < binding->bufferDuration)
    {
        return true;
    }

    for (usize i = 0; i < binding->buttonsLength; ++i)
    {
        if (IsMouseButtonReleased(binding->buttons[i]))
        {
            return true;
        }
    }

    return false;
}

InputProfile InputProfileCreate(const usize bindingsCapactity)
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
        .axisBindings = calloc(bindingsCapactity, sizeof(AxisBinding)),
        .axisBindingsLength = 0,
    };
}

void InputProfileAddKeyboardBinding(InputProfile* self, const KeyboardBinding binding)
{
    if (self->keyboardBindingsLength >= self->bindingsCapacity)
    {
        return;
    }

    self->keyboardBindings[self->keyboardBindingsLength] = binding;

    self->keyboardBindingsLength += 1;
}

void InputProfileAddGamepadBinding(InputProfile* self, const GamepadBinding binding)
{
    if (self->gamepadBindingsLength >= self->bindingsCapacity)
    {
        return;
    }

    self->gamepadBindings[self->gamepadBindingsLength] = binding;

    self->gamepadBindingsLength += 1;
}

void InputProfileAddMouseBinding(InputProfile* self, const MouseBinding binding)
{
    if (self->gamepadBindingsLength >= self->bindingsCapacity)
    {
        return;
    }

    self->mouseBindings[self->mouseBindingsLength] = binding;

    self->mouseBindingsLength += 1;
}

void InputProfileAddAxisBinding(InputProfile* self, const AxisBinding binding)
{
    if (self->axisBindingsLength >= self->bindingsCapacity)
    {
        return;
    }

    self->axisBindings[self->axisBindingsLength] = binding;

    self->axisBindingsLength += 1;
}

InputHandler InputHandlerCreate(const usize gamepad)
{
    return (InputHandler)
    {
        .gamepad = gamepad,
        .enabled = false,
    };
}

void InputHandlerSetProfile(InputHandler* self, const InputProfile profile)
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

bool InputHandlerPressed(const InputHandler* self, const char* binding)
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

bool InputHandlerPressing(const InputHandler* self, const char* binding)
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

    for (usize i = 0; i < self->profile.axisBindingsLength; ++i)
    {
        if (strcmp(self->profile.axisBindings[i].name, binding) == 0)
        {
            if (AxisBindingPressing(&self->profile.axisBindings[i], self->gamepad))
            {
                return true;
            }
        }
    }

    return false;
}

bool InputHandlerReleased(const InputHandler* self, const char* binding)
{
    if (!self->enabled)
    {
        return false;
    }

    for (usize i = 0; i < self->profile.keyboardBindingsLength; ++i)
    {
        if (strcmp(self->profile.keyboardBindings[i].name, binding) == 0)
        {
            if (KeyboardBindingReleased(&self->profile.keyboardBindings[i]))
            {
                return true;
            }
        }
    }

    for (usize i = 0; i < self->profile.gamepadBindingsLength; ++i)
    {
        if (strcmp(self->profile.gamepadBindings[i].name, binding) == 0)
        {
            if (GamepadBindingReleased(&self->profile.gamepadBindings[i], self->gamepad))
            {
                return true;
            }
        }
    }

    for (usize i = 0; i < self->profile.mouseBindingsLength; ++i)
    {
        if (strcmp(self->profile.mouseBindings[i].name, binding) == 0)
        {
            if (MouseBindingReleased(&self->profile.mouseBindings[i]))
            {
                return true;
            }
        }
    }

    return false;
}

void InputHandlerConsume(InputHandler* self, const char* binding)
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
