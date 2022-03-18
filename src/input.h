#pragma once

#include "common.h"

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
    usize bindingsCapacity;
    KeyboardBinding* keyboardBindings;
    usize keyboardBindingsLength;
    GamepadBinding* gamepadBindings;
    usize gamepadBindingsLength;
    MouseBinding* mouseBindings;
    usize mouseBindingsLength;
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

InputProfile InputProfileCreate(usize bindingsCapacity);
void InputProfileAddKeyboardBinding(InputProfile* self, KeyboardBinding binding);
void InputProfileAddGamepadBinding(InputProfile* self, GamepadBinding binding);
void InputProfileAddMouseBinding(InputProfile* self, MouseBinding binding);

InputHandler InputHandlerCreate();
void InputHandlerSetProfile(InputHandler* self, InputProfile profile);
void InputHandlerUpdate(InputHandler* self);
bool InputHandlerPressed(const InputHandler* self, char* binding);
bool InputHandlerPressing(const InputHandler* self, char* binding);
void InputHandlerConsume(InputHandler* self, char* binding);
