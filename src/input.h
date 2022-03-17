#pragma once

#include "common.h"

typedef struct
{
    char* name;
    KeyboardKey* keys;
    usize keysCapacity;
    usize keysLength;
    GamepadButton* buttons;
    usize buttonsCapacity;
    usize buttonsLength;
    f32 bufferDuration;
    f32 bufferTimer;
} ButtonBinding;

typedef struct
{
    ButtonBinding* bindings;
    usize bindingsCapacity;
    usize bindingsLength;
} InputProfile;

typedef struct
{
    usize gamepad;
    bool enabled;
    InputProfile profile;
} InputHandler;

ButtonBinding ButtonBindingCreate(char* name, usize keysCapacity, usize buttonsCapacity);
void ButtonBindingSetBuffer(ButtonBinding* self, f32 bufferDuration);
void ButtonBindingAddKey(ButtonBinding* self, KeyboardKey key);
void ButtonBindingAddButton(ButtonBinding* self, GamepadButton button);

InputProfile InputProfileCreate(usize totalBindings);
void InputProfileAddBinding(InputProfile* self, ButtonBinding binding);

InputHandler InputHandlerCreate(usize gamepad);
void InputHandlerSetProfile(InputHandler* self, InputProfile profile);
void InputHandlerUpdate(InputHandler* self);
bool InputHandlerPressed(const InputHandler* self, char* binding);
bool InputHandlerPressing(const InputHandler* self, char* binding);
void InputHandlerConsume(InputHandler* self, char* binding);
