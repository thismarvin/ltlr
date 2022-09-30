#pragma once

#include "common.h"

typedef struct
{
    char* m_name;
    KeyboardKey* m_keys;
    usize m_keysCapacity;
    usize m_keysLength;
    f32 m_bufferDuration;
    f32 m_bufferTimer;
} KeyboardBinding;

typedef struct
{
    char* m_name;
    GamepadButton* m_buttons;
    usize m_buttonsCapacity;
    usize m_buttonsLength;
    f32 m_bufferDuration;
    f32 m_bufferTimer;
} GamepadBinding;

typedef struct
{
    char* m_name;
    MouseButton* m_buttons;
    usize m_buttonsCapacity;
    usize m_buttonsLength;
    f32 m_bufferDuration;
    f32 m_bufferTimer;
} MouseBinding;

typedef struct
{
    char* m_name;
    GamepadAxis* m_axes;
    usize m_axesCapacity;
    usize m_axesLength;
    Ordering m_ordering;
    f32 m_target;
} AxisBinding;

typedef struct
{
    usize m_bindingsCapacity;
    KeyboardBinding* m_keyboardBindings;
    usize m_keyboardBindingsLength;
    GamepadBinding* m_gamepadBindings;
    usize m_gamepadBindingsLength;
    MouseBinding* m_mouseBindings;
    usize m_mouseBindingsLength;
    AxisBinding* m_axisBindings;
    usize m_axisBindingsLength;
} InputProfile;

typedef struct
{
    usize m_gamepad;
    InputProfile* m_profile;
} InputHandler;

KeyboardBinding KeyboardBindingCreate(const char* name, usize keysCapacity);
void KeyboardBindingSetBuffer(KeyboardBinding* self, f32 bufferDuration);
void KeyboardBindingAddKey(KeyboardBinding* self, KeyboardKey key);
void KeyboardBindingDestroy(KeyboardBinding* self);

GamepadBinding GamepadBindingCreate(const char* name, usize buttonsCapacity);
void GamepadBindingSetBuffer(GamepadBinding* self, f32 bufferDuration);
void GamepadBindingAddButton(GamepadBinding* self, GamepadButton button);
void GamepadBindingDestroy(GamepadBinding* self);

MouseBinding MouseBindingCreate(const char* name, usize buttonsCapacity);
void MouseBindingSetBuffer(MouseBinding* self, f32 bufferDuration);
void MouseBindingAddButton(MouseBinding* self, MouseButton button);
void MouseBindingDestroy(MouseBinding* self);

AxisBinding AxisBindingCreate(const char* name, usize axesCapacity, Ordering ordering, f32 target);
void AxisBindingAddAxis(AxisBinding* self, GamepadAxis axis);
void AxisBindingDestroy(AxisBinding* self);

InputProfile InputProfileCreate(usize bindingsCapacity);
void InputProfileAddKeyboardBinding(InputProfile* self, KeyboardBinding binding);
void InputProfileAddGamepadBinding(InputProfile* self, GamepadBinding binding);
void InputProfileAddMouseBinding(InputProfile* self, MouseBinding binding);
void InputProfileAddAxisBinding(InputProfile* self, AxisBinding binding);
void InputProfileDestroy(InputProfile* self);

InputHandler InputHandlerCreate(usize gamepad);
void InputHandlerSetProfile(InputHandler* self, const InputProfile* profile);
void InputHandlerUpdate(InputHandler* self);
// TODO(thismarvin): Return the active bindings somehow...
bool InputHandlerPressed(const InputHandler* self, const char* binding);
bool InputHandlerPressing(const InputHandler* self, const char* binding);
bool InputHandlerReleased(const InputHandler* self, const char* binding);
void InputHandlerConsume(InputHandler* self, const char* binding);
