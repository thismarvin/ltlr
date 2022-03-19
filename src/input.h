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

KeyboardBinding KeyboardBindingCreate(char* name, usize keysCapacity);
void KeyboardBindingSetBuffer(KeyboardBinding* self, f32 bufferDuration);
void KeyboardBindingAddKey(KeyboardBinding* self, KeyboardKey key);

GamepadBinding GamepadBindingCreate(char* name, usize buttonsCapacity);
void GamepadBindingSetBuffer(GamepadBinding* self, f32 bufferDuration);
void GamepadBindingAddButton(GamepadBinding* self, GamepadButton button);

MouseBinding MouseBindingCreate(char* name, usize buttonsCapacity);
void MouseBindingSetBuffer(MouseBinding* self, f32 bufferDuration);
void MouseBindingAddButton(MouseBinding* self, MouseButton button);

AxisBinding AxisBindingCreate(char* name, usize axesCapacity, Ordering ordering, f32 target);
void AxisBindingAddAxis(AxisBinding* self, GamepadAxis axis);

InputProfile InputProfileCreate(usize bindingsCapacity);
void InputProfileAddKeyboardBinding(InputProfile* self, KeyboardBinding binding);
void InputProfileAddGamepadBinding(InputProfile* self, GamepadBinding binding);
void InputProfileAddMouseBinding(InputProfile* self, MouseBinding binding);
void InputProfileAddAxisBinding(InputProfile* self, AxisBinding binding);

InputHandler InputHandlerCreate();
void InputHandlerSetProfile(InputHandler* self, InputProfile profile);
void InputHandlerUpdate(InputHandler* self);
// TODO(thismarvin): Return the active bindings somehow...
bool InputHandlerPressed(const InputHandler* self, char* binding);
bool InputHandlerPressing(const InputHandler* self, char* binding);
bool InputHandlerReleased(const InputHandler* self, char* binding);
void InputHandlerConsume(InputHandler* self, char* binding);
