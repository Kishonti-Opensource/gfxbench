/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "input_component.h"
#include "test_base_gfx.h"

#include <messagequeue.h>

#include "ngl.h"

using namespace GLB;

const char* InputComponent::NAME = "InputComponent";

InputComponent::InputComponent(TestBaseGFX *test) : TestComponent(test, NAME)
{
	m_user_input_enabled = false;
	m_current_input_state = nullptr;
	m_prev_input_state = nullptr;
	m_msg_queue = nullptr;

	SetUserInputEnabled(true);
}


InputComponent::~InputComponent()
{
	delete m_current_input_state;
	delete m_prev_input_state;
}


void InputComponent::BeginFrame()
{
	if (m_user_input_enabled == false)
	{
		return;
	}

	memcpy(m_prev_input_state, m_current_input_state, sizeof(InputState));

	if (m_msg_queue)
	{
		while (m_msg_queue->has_next())
		{
			tfw::Message msg = m_msg_queue->pop_front();

			switch (msg.type)
			{
				// Key press
			case 1:
				if (msg.arg1 >= 0 && msg.arg1 < MAX_KEYS) // Special keys like Volume up/down have negative arg1
				{
					m_current_input_state->m_keys[msg.arg1] = msg.arg2 != 0;
				}
				break;

				// Mouse movement
			case 2:
				m_current_input_state->m_mouse_pos_x = msg.arg1;
				m_current_input_state->m_mouse_pos_y = msg.arg2;
				break;

				// Mouse buttons
			case 3:
				if (msg.arg1 >= 0 && msg.arg1 < MAX_MOUSE_BUTTONS)
				{
					m_current_input_state->m_mouse_buttons[msg.arg1] = msg.arg2 != 0;
				}
				break;

			case 4: // MSG_TYPE_RESIZE
				nglCustomAction(0, 235173);
				break;
			}
		}
	}
}


void InputComponent::SetQueue(tfw::MessageQueue *queue)
{
	m_msg_queue = queue;
}


void InputComponent::SetUserInputEnabled(bool enabled)
{
	if (enabled)
	{
		if (m_current_input_state == nullptr)
		{
			m_current_input_state = new InputState();
		}
		if (m_prev_input_state == nullptr)
		{
			m_prev_input_state = new InputState();
		}

		if (m_user_input_enabled == false)
		{
			memset(m_current_input_state, 0, sizeof(InputState));
			memset(m_prev_input_state, 0, sizeof(InputState));
		}
	}

	m_user_input_enabled = enabled;
}


bool InputComponent::IsUserInputEnabled() const
{
	return m_user_input_enabled;
}


bool InputComponent::IsKeyDown(int key_code) const
{
	if (m_user_input_enabled)
	{
		return m_current_input_state->m_keys[key_code];
	}
	return false;
}


bool InputComponent::IsKeyUp(int key_code) const
{
	if (m_user_input_enabled)
	{
		return !m_current_input_state->m_keys[key_code];
	}
	return false;
}


bool InputComponent::IsKeyPressed(int key_code) const
{
	if (m_user_input_enabled)
	{
		return !m_prev_input_state->m_keys[key_code] && m_current_input_state->m_keys[key_code];
	}
	return false;
}


bool InputComponent::IsKeyReleased(int key_code) const
{
	if (m_user_input_enabled)
	{
		return m_prev_input_state->m_keys[key_code] && !m_current_input_state->m_keys[key_code];
	}
	return false;
}


int InputComponent::GetMouseX() const
{
	if (m_user_input_enabled)
	{
		return m_current_input_state->m_mouse_pos_x;
	}
	return 0;
}


int InputComponent::GetMouseY() const
{
	if (m_user_input_enabled)
	{
		return m_current_input_state->m_mouse_pos_y;
	}
	return 0;
}


int InputComponent::GetMouseOldX() const
{
	if (m_user_input_enabled)
	{
		return m_prev_input_state->m_mouse_pos_x;
	}
	return 0;
}


int InputComponent::GetMouseOldY() const
{
	if (m_user_input_enabled)
	{
		return m_prev_input_state->m_mouse_pos_y;
	}
	return 0;
}


int InputComponent::IsMouseButtonDown(int button) const
{
	if (m_user_input_enabled && button < MAX_MOUSE_BUTTONS)
	{
		return m_current_input_state->m_mouse_buttons[button];
	}
	return false;
}


int InputComponent::IsMouseButtonUp(int button) const
{
	if (m_user_input_enabled && button < MAX_MOUSE_BUTTONS)
	{
		return !m_current_input_state->m_mouse_buttons[button];
	}
	return false;
}


int InputComponent::IsMouseButtonPressed(int button) const
{
	if (m_user_input_enabled && button < MAX_MOUSE_BUTTONS)
	{
		return !m_prev_input_state->m_mouse_buttons[button] && m_current_input_state->m_mouse_buttons[button];
	}
	return false;
}


int InputComponent::IsMouseButtonReleased(int button) const
{
	if (m_user_input_enabled && button < MAX_MOUSE_BUTTONS)
	{
		return m_prev_input_state->m_mouse_buttons[button] && !m_current_input_state->m_mouse_buttons[button];
	}
	return false;
}
