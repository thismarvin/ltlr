#pragma once

#include "common.h"

typedef enum
{
    ORD_LESS,
    ORD_GREATER,
} Ordering;

typedef struct
{
    char* name;
    KeyboardKey* keys;
    usize keysCapacity;
    usize keysLength;
    f32 bufferDuration;
    f32 bufferTimer;
} KeyboardBinding;

typedef struct
{
    char* name;
    GamepadButton* buttons;
    usize buttonsCapacity;
    usize buttonsLength;
    f32 bufferDuration;
    f32 bufferTimer;
} GamepadBinding;

typedef struct
{
    char* name;
    MouseButton* buttons;
    usize buttonsCapacity;
    usize buttonsLength;
    f32 bufferDuration;
    f32 bufferTimer;
} MouseBinding;

typedef struct
{
    char* name;
    GamepadAxis* axes;
    usize axesCapacity;
    usize axesLength;
    Ordering ordering;
    f32 target;
} AxisBinding;

typedef struct
{
    usize bindingsCapacity;
    KeyboardBinding* keyboardBindings;
    usize keyboardBindingsLength;
    GamepadBinding* gamepadBindings;
    usize gamepadBindingsLength;
    MouseBinding* mouseBindings;
    usize mouseBindingsLength;
    AxisBinding* axisBindings;
    usize axisBindingsLength;
} InputProfile;

typedef struct
{
    usize gamepad;
    bool enabled;
    InputProfile profile;
} InputHandler;

KeyboardBinding KeyboardBindingCreate(const char* name, const usize keysCapacity);
void KeyboardBindingSetBuffer(KeyboardBinding* self, const f32 bufferDuration);
void KeyboardBindingAddKey(KeyboardBinding* self, const KeyboardKey key);

GamepadBinding GamepadBindingCreate(const char* name, const usize buttonsCapacity);
void GamepadBindingSetBuffer(GamepadBinding* self, const f32 bufferDuration);
void GamepadBindingAddButton(GamepadBinding* self, const GamepadButton button);

MouseBinding MouseBindingCreate(const char* name, const usize buttonsCapacity);
void MouseBindingSetBuffer(MouseBinding* self, const f32 bufferDuration);
void MouseBindingAddButton(MouseBinding* self, const MouseButton button);

AxisBinding AxisBindingCreate
(
    const char* name,
    const usize axesCapacity,
    const Ordering ordering,
    const f32 target
);
void AxisBindingAddAxis(AxisBinding* self, const GamepadAxis axis);

InputProfile InputProfileCreate(const usize bindingsCapacity);
void InputProfileAddKeyboardBinding(InputProfile* self, const KeyboardBinding binding);
void InputProfileAddGamepadBinding(InputProfile* self, const GamepadBinding binding);
void InputProfileAddMouseBinding(InputProfile* self, const MouseBinding binding);
void InputProfileAddAxisBinding(InputProfile* self, const AxisBinding binding);

InputHandler InputHandlerCreate();
void InputHandlerSetProfile(InputHandler* self, const InputProfile profile);
void InputHandlerUpdate(InputHandler* self);
// TODO(thismarvin): Return the active bindings somehow...
bool InputHandlerPressed(const InputHandler* self, const char* binding);
bool InputHandlerPressing(const InputHandler* self, const char* binding);
bool InputHandlerReleased(const InputHandler* self, const char* binding);
void InputHandlerConsume(InputHandler* self, const char* binding);
