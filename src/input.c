#include "input.h"

#include "context.h"

#include <string.h>

static void KeyboardBindingConsume(KeyboardBinding* binding)
{
	binding->m_bufferTimer = binding->m_bufferDuration;
}

static void GamepadBindingConsume(GamepadBinding* binding, const usize gamepad)
{
	if (!IsGamepadAvailable(gamepad))
	{
		return;
	}

	binding->m_bufferTimer = binding->m_bufferDuration;
}

static void MouseBindingConsume(MouseBinding* binding)
{
	binding->m_bufferTimer = binding->m_bufferDuration;
}

KeyboardBinding KeyboardBindingCreate(const char* name, const usize keysCapacity)
{
	// clang-format off
	return (KeyboardBinding) {
		.m_name = (char*)name,
		.m_keys = calloc(keysCapacity, sizeof(KeyboardKey)),
		.m_keysCapacity = keysCapacity,
		.m_keysLength = 0,
		.m_bufferDuration = 0,
		.m_bufferTimer = 0
	};
	// clang-format on
}

void KeyboardBindingSetBuffer(KeyboardBinding* self, const f32 bufferDuration)
{
	self->m_bufferDuration = bufferDuration;
	self->m_bufferTimer = bufferDuration;
}

void KeyboardBindingAddKey(KeyboardBinding* self, const KeyboardKey key)
{
	if (self->m_keysLength >= self->m_keysCapacity)
	{
		return;
	}

	self->m_keys[self->m_keysLength] = key;

	self->m_keysLength += 1;
}

void KeyboardBindingDestroy(KeyboardBinding* self)
{
	free(self->m_keys);
}

GamepadBinding GamepadBindingCreate(const char* name, const usize buttonsCapacity)
{
	return (GamepadBinding) {
		.m_name = (char*)name,
		.m_buttons = calloc(buttonsCapacity, sizeof(GamepadButton)),
		.m_buttonsCapacity = buttonsCapacity,
		.m_buttonsLength = 0,
		.m_bufferDuration = 0,
		.m_bufferTimer = 0,
	};
}

void GamepadBindingSetBuffer(GamepadBinding* self, const f32 bufferDuration)
{
	self->m_bufferDuration = bufferDuration;
	self->m_bufferTimer = bufferDuration;
}

void GamepadBindingAddButton(GamepadBinding* self, const GamepadButton button)
{
	if (self->m_buttonsLength >= self->m_buttonsCapacity)
	{
		return;
	}

	self->m_buttons[self->m_buttonsLength] = button;

	self->m_buttonsLength += 1;
}

void GamepadBindingDestroy(GamepadBinding* self)
{
	free(self->m_buttons);
}

MouseBinding MouseBindingCreate(const char* name, const usize buttonsCapacity)
{
	return (MouseBinding) {
		.m_name = (char*)name,
		.m_buttons = calloc(buttonsCapacity, sizeof(MouseButton)),
		.m_buttonsCapacity = buttonsCapacity,
		.m_buttonsLength = 0,
		.m_bufferDuration = 0,
		.m_bufferTimer = 0,
	};
}

void MouseBindingSetBuffer(MouseBinding* self, const f32 bufferDuration)
{
	self->m_bufferDuration = bufferDuration;
	self->m_bufferTimer = bufferDuration;
}

void MouseBindingAddButton(MouseBinding* self, const MouseButton button)
{
	if (self->m_buttonsLength >= self->m_buttonsCapacity)
	{
		return;
	}

	self->m_buttons[self->m_buttonsLength] = button;

	self->m_buttonsLength += 1;
}

void MouseBindingDestroy(MouseBinding* self)
{
	free(self->m_buttons);
}

AxisBinding AxisBindingCreate(
	const char* name,
	const usize axesCapacity,
	const Ordering ordering,
	const f32 target
)
{
	return (AxisBinding) {
		.m_name = (char*)name,
		.m_axes = calloc(axesCapacity, sizeof(GamepadAxis)),
		.m_axesCapacity = axesCapacity,
		.m_axesLength = 0,
		.m_ordering = ordering,
		.m_target = target,
	};
}

void AxisBindingAddAxis(AxisBinding* self, const GamepadAxis axis)
{
	if (self->m_axesLength >= self->m_axesCapacity)
	{
		return;
	}

	self->m_axes[self->m_axesLength] = axis;

	self->m_axesLength += 1;
}

void AxisBindingDestroy(AxisBinding* self)
{
	free(self->m_axes);
}

static void KeyboardBindingUpdate(KeyboardBinding* binding)
{
	binding->m_bufferTimer += CTX_DT;

	bool inactive = true;

	for (usize i = 0; i < binding->m_keysLength; ++i)
	{
		if (IsKeyPressed(binding->m_keys[i]))
		{
			binding->m_bufferTimer = 0;
		}

		if (IsKeyReleased(binding->m_keys[i]))
		{
			binding->m_bufferTimer = 0;
		}

		if (!IsKeyUp(binding->m_keys[i]))
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

	binding->m_bufferTimer += CTX_DT;

	bool inactive = true;

	for (usize i = 0; i < binding->m_buttonsLength; ++i)
	{
		if (IsGamepadButtonPressed(gamepad, binding->m_buttons[i]))
		{
			binding->m_bufferTimer = 0;
		}

		if (IsGamepadButtonReleased(gamepad, binding->m_buttons[i]))
		{
			binding->m_bufferTimer = 0;
		}

		if (!IsGamepadButtonUp(gamepad, binding->m_buttons[i]))
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
	binding->m_bufferTimer += CTX_DT;

	bool inactive = true;

	for (usize i = 0; i < binding->m_buttonsLength; ++i)
	{
		if (IsMouseButtonPressed(binding->m_buttons[i]))
		{
			binding->m_bufferTimer = 0;
		}

		if (IsMouseButtonReleased(binding->m_buttons[i]))
		{
			binding->m_bufferTimer = 0;
		}

		if (!IsMouseButtonUp(binding->m_buttons[i]))
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
	if (binding->m_bufferDuration != 0 && binding->m_bufferTimer < binding->m_bufferDuration)
	{
		return true;
	}

	for (usize i = 0; i < binding->m_keysLength; ++i)
	{
		if (IsKeyPressed(binding->m_keys[i]))
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

	if (binding->m_bufferDuration != 0 && binding->m_bufferTimer < binding->m_bufferDuration)
	{
		return true;
	}

	for (usize i = 0; i < binding->m_buttonsLength; ++i)
	{
		if (IsGamepadButtonPressed(gamepad, binding->m_buttons[i]))
		{
			return true;
		}
	}

	return false;
}

static bool MouseBindingPressed(const MouseBinding* binding)
{
	if (binding->m_bufferDuration != 0 && binding->m_bufferTimer < binding->m_bufferDuration)
	{
		return true;
	}

	for (usize i = 0; i < binding->m_buttonsLength; ++i)
	{
		if (IsMouseButtonPressed(binding->m_buttons[i]))
		{
			return true;
		}
	}

	return false;
}

static bool KeyboardBindingPressing(const KeyboardBinding* binding)
{
	for (usize i = 0; i < binding->m_keysLength; ++i)
	{
		if (IsKeyDown(binding->m_keys[i]))
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

	for (usize i = 0; i < binding->m_buttonsLength; ++i)
	{
		if (IsGamepadButtonDown(gamepad, binding->m_buttons[i]))
		{
			return true;
		}
	}

	return false;
}

static bool MouseBindingPressing(const MouseBinding* binding)
{
	for (usize i = 0; i < binding->m_buttonsLength; ++i)
	{
		if (IsMouseButtonDown(binding->m_buttons[i]))
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

	for (usize i = 0; i < binding->m_axesLength; ++i)
	{
		const f32 value = GetGamepadAxisMovement(gamepad, binding->m_axes[i]);

		if (binding->m_ordering == ORD_LESS && value < binding->m_target)
		{
			return true;
		}

		if (binding->m_ordering == ORD_GREATER && value > binding->m_target)
		{
			return true;
		}
	}

	return false;
}

static bool KeyboardBindingReleased(const KeyboardBinding* binding)
{
	if (binding->m_bufferDuration != 0 && binding->m_bufferTimer < binding->m_bufferDuration)
	{
		return true;
	}

	for (usize i = 0; i < binding->m_keysLength; ++i)
	{
		if (IsKeyReleased(binding->m_keys[i]))
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

	if (binding->m_bufferDuration != 0 && binding->m_bufferTimer < binding->m_bufferDuration)
	{
		return true;
	}

	for (usize i = 0; i < binding->m_buttonsLength; ++i)
	{
		if (IsGamepadButtonReleased(gamepad, binding->m_buttons[i]))
		{
			return true;
		}
	}

	return false;
}

static bool MouseBindingReleased(const MouseBinding* binding)
{
	if (binding->m_bufferDuration != 0 && binding->m_bufferTimer < binding->m_bufferDuration)
	{
		return true;
	}

	for (usize i = 0; i < binding->m_buttonsLength; ++i)
	{
		if (IsMouseButtonReleased(binding->m_buttons[i]))
		{
			return true;
		}
	}

	return false;
}

InputProfile InputProfileCreate(const usize bindingsCapactity)
{
	return (InputProfile) {
		.m_bindingsCapacity = bindingsCapactity,
		.m_keyboardBindings = calloc(bindingsCapactity, sizeof(KeyboardBinding)),
		.m_keyboardBindingsLength = 0,
		.m_gamepadBindings = calloc(bindingsCapactity, sizeof(GamepadBinding)),
		.m_gamepadBindingsLength = 0,
		.m_mouseBindings = calloc(bindingsCapactity, sizeof(MouseBinding)),
		.m_mouseBindingsLength = 0,
		.m_axisBindings = calloc(bindingsCapactity, sizeof(AxisBinding)),
		.m_axisBindingsLength = 0,
	};
}

void InputProfileAddKeyboardBinding(InputProfile* self, const KeyboardBinding binding)
{
	if (self->m_keyboardBindingsLength >= self->m_bindingsCapacity)
	{
		return;
	}

	self->m_keyboardBindings[self->m_keyboardBindingsLength] = binding;

	self->m_keyboardBindingsLength += 1;
}

void InputProfileAddGamepadBinding(InputProfile* self, const GamepadBinding binding)
{
	if (self->m_gamepadBindingsLength >= self->m_bindingsCapacity)
	{
		return;
	}

	self->m_gamepadBindings[self->m_gamepadBindingsLength] = binding;

	self->m_gamepadBindingsLength += 1;
}

void InputProfileAddMouseBinding(InputProfile* self, const MouseBinding binding)
{
	if (self->m_gamepadBindingsLength >= self->m_bindingsCapacity)
	{
		return;
	}

	self->m_mouseBindings[self->m_mouseBindingsLength] = binding;

	self->m_mouseBindingsLength += 1;
}

void InputProfileAddAxisBinding(InputProfile* self, const AxisBinding binding)
{
	if (self->m_axisBindingsLength >= self->m_bindingsCapacity)
	{
		return;
	}

	self->m_axisBindings[self->m_axisBindingsLength] = binding;

	self->m_axisBindingsLength += 1;
}

void InputProfileDestroy(InputProfile* self)
{
	for (usize i = 0; i < self->m_keyboardBindingsLength; ++i)
	{
		KeyboardBindingDestroy(&self->m_keyboardBindings[i]);
	}

	for (usize i = 0; i < self->m_gamepadBindingsLength; ++i)
	{
		GamepadBindingDestroy(&self->m_gamepadBindings[i]);
	}

	for (usize i = 0; i < self->m_mouseBindingsLength; ++i)
	{
		MouseBindingDestroy(&self->m_mouseBindings[i]);
	}

	for (usize i = 0; i < self->m_axisBindingsLength; ++i)
	{
		AxisBindingDestroy(&self->m_axisBindings[i]);
	}

	free(self->m_keyboardBindings);
	free(self->m_gamepadBindings);
	free(self->m_mouseBindings);
	free(self->m_axisBindings);
}

InputHandler InputHandlerCreate(const usize gamepad)
{
	return (InputHandler) {
		.m_gamepad = gamepad,
		.m_profile = NULL,
	};
}

void InputHandlerSetProfile(InputHandler* self, const InputProfile* profile)
{
	self->m_profile = (InputProfile*)profile;
}

static bool InputHandlerEnabled(const InputHandler* self)
{
	return self->m_profile != NULL;
}

void InputHandlerUpdate(InputHandler* self)
{
	if (!InputHandlerEnabled(self))
	{
		return;
	}

	if (self->m_gamepad == 0)
	{
		for (usize i = 0; i < self->m_profile->m_keyboardBindingsLength; ++i)
		{
			KeyboardBindingUpdate(&self->m_profile->m_keyboardBindings[i]);
		}
	}

	for (usize i = 0; i < self->m_profile->m_gamepadBindingsLength; ++i)
	{
		GamepadBindingUpdate(&self->m_profile->m_gamepadBindings[i], self->m_gamepad);
	}

	for (usize i = 0; i < self->m_profile->m_mouseBindingsLength; ++i)
	{
		MouseBindingUpdate(&self->m_profile->m_mouseBindings[i]);
	}
}

bool InputHandlerPressed(const InputHandler* self, const char* binding)
{
	if (!InputHandlerEnabled(self))
	{
		return false;
	}

	if (self->m_gamepad == 0)
	{
		for (usize i = 0; i < self->m_profile->m_keyboardBindingsLength; ++i)
		{
			if (strcmp(self->m_profile->m_keyboardBindings[i].m_name, binding) == 0)
			{
				if (KeyboardBindingPressed(&self->m_profile->m_keyboardBindings[i]))
				{
					return true;
				}
			}
		}
	}

	for (usize i = 0; i < self->m_profile->m_gamepadBindingsLength; ++i)
	{
		if (strcmp(self->m_profile->m_gamepadBindings[i].m_name, binding) == 0)
		{
			if (GamepadBindingPressed(&self->m_profile->m_gamepadBindings[i], self->m_gamepad))
			{
				return true;
			}
		}
	}

	for (usize i = 0; i < self->m_profile->m_mouseBindingsLength; ++i)
	{
		if (strcmp(self->m_profile->m_mouseBindings[i].m_name, binding) == 0)
		{
			if (MouseBindingPressed(&self->m_profile->m_mouseBindings[i]))
			{
				return true;
			}
		}
	}

	return false;
}

bool InputHandlerPressing(const InputHandler* self, const char* binding)
{
	if (!InputHandlerEnabled(self))
	{
		return false;
	}

	if (self->m_gamepad == 0)
	{
		for (usize i = 0; i < self->m_profile->m_keyboardBindingsLength; ++i)
		{
			if (strcmp(self->m_profile->m_keyboardBindings[i].m_name, binding) == 0)
			{
				if (KeyboardBindingPressing(&self->m_profile->m_keyboardBindings[i]))
				{
					return true;
				}
			}
		}
	}

	for (usize i = 0; i < self->m_profile->m_gamepadBindingsLength; ++i)
	{
		if (strcmp(self->m_profile->m_gamepadBindings[i].m_name, binding) == 0)
		{
			if (GamepadBindingPressing(&self->m_profile->m_gamepadBindings[i], self->m_gamepad))
			{
				return true;
			}
		}
	}

	for (usize i = 0; i < self->m_profile->m_mouseBindingsLength; ++i)
	{
		if (strcmp(self->m_profile->m_mouseBindings[i].m_name, binding) == 0)
		{
			if (MouseBindingPressing(&self->m_profile->m_mouseBindings[i]))
			{
				return true;
			}
		}
	}

	for (usize i = 0; i < self->m_profile->m_axisBindingsLength; ++i)
	{
		if (strcmp(self->m_profile->m_axisBindings[i].m_name, binding) == 0)
		{
			if (AxisBindingPressing(&self->m_profile->m_axisBindings[i], self->m_gamepad))
			{
				return true;
			}
		}
	}

	return false;
}

bool InputHandlerReleased(const InputHandler* self, const char* binding)
{
	if (!InputHandlerEnabled(self))
	{
		return false;
	}

	if (self->m_gamepad == 0)
	{
		for (usize i = 0; i < self->m_profile->m_keyboardBindingsLength; ++i)
		{
			if (strcmp(self->m_profile->m_keyboardBindings[i].m_name, binding) == 0)
			{
				if (KeyboardBindingReleased(&self->m_profile->m_keyboardBindings[i]))
				{
					return true;
				}
			}
		}
	}

	for (usize i = 0; i < self->m_profile->m_gamepadBindingsLength; ++i)
	{
		if (strcmp(self->m_profile->m_gamepadBindings[i].m_name, binding) == 0)
		{
			if (GamepadBindingReleased(&self->m_profile->m_gamepadBindings[i], self->m_gamepad))
			{
				return true;
			}
		}
	}

	for (usize i = 0; i < self->m_profile->m_mouseBindingsLength; ++i)
	{
		if (strcmp(self->m_profile->m_mouseBindings[i].m_name, binding) == 0)
		{
			if (MouseBindingReleased(&self->m_profile->m_mouseBindings[i]))
			{
				return true;
			}
		}
	}

	return false;
}

void InputHandlerConsume(InputHandler* self, const char* binding)
{
	if (!InputHandlerEnabled(self))
	{
		return;
	}

	if (self->m_gamepad == 0)
	{
		for (usize i = 0; i < self->m_profile->m_keyboardBindingsLength; ++i)
		{
			if (strcmp(self->m_profile->m_keyboardBindings[i].m_name, binding) == 0)
			{
				KeyboardBindingConsume(&self->m_profile->m_keyboardBindings[i]);

				break;
			}
		}
	}

	for (usize i = 0; i < self->m_profile->m_gamepadBindingsLength; ++i)
	{
		if (strcmp(self->m_profile->m_gamepadBindings[i].m_name, binding) == 0)
		{
			GamepadBindingConsume(&self->m_profile->m_gamepadBindings[i], self->m_gamepad);

			break;
		}
	}

	for (usize i = 0; i < self->m_profile->m_mouseBindingsLength; ++i)
	{
		if (strcmp(self->m_profile->m_mouseBindings[i].m_name, binding) == 0)
		{
			MouseBindingConsume(&self->m_profile->m_mouseBindings[i]);

			break;
		}
	}
}
