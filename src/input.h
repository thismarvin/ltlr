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
} ButtonBinding;

typedef struct
{
    ButtonBinding* bindings;
    usize bindingsCapacity;
    usize bindingsLength;
} InputProfile;

typedef struct
{
    usize playerIndex;
    bool enabled;
    InputProfile profile;
} InputHandler;

ButtonBinding ButtonBindingCreate(char* name, usize keysCapacity);
void ButtonBindingSetBuffer(ButtonBinding* self, f32 bufferDuration);
void ButtonBindingAddKey(ButtonBinding* self, KeyboardKey key);
void ButtonBindingUpdate(ButtonBinding* self);
bool ButtonBindingPressed(const ButtonBinding* self);
bool ButtonBindingPressing(const ButtonBinding* self);
void ButtonBindingConsume(ButtonBinding* self);

InputProfile InputProfileCreate(usize totalBindings);
void InputProfileAddBinding(InputProfile* self, ButtonBinding binding);

InputHandler InputHandlerCreate(usize playerIndex);
void InputHandlerSetProfile(InputHandler* self, InputProfile profile);
void InputHandlerUpdate(InputHandler* self);
bool InputHandlerPressed(const InputHandler* self, char* binding);
bool InputHandlerPressing(const InputHandler* self, char* binding);
void InputHandlerConsume(InputHandler* self, char* binding);
