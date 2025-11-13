/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef INPUT_COMPONENT_H
#define INPUT_COMPONENT_H

#include "test_component.h"

namespace tfw
{
	class MessageQueue;
}

namespace GLB
{
	class InputComponent : public TestComponent
	{
	public:
		static const char *NAME;

		InputComponent(TestBaseGFX *test);
		virtual ~InputComponent();

		virtual void BeginFrame() override;

		void SetQueue(tfw::MessageQueue *queue);
		void SetUserInputEnabled(bool enabled);
		bool IsUserInputEnabled() const;

		bool IsKeyDown(int key_code) const;
		bool IsKeyUp(int key_code) const;
		bool IsKeyPressed(int key_code) const;
		bool IsKeyReleased(int key_code) const;
		int GetMouseX() const;
		int GetMouseY() const;
		int GetMouseOldX() const;
		int GetMouseOldY() const;
		int IsMouseButtonDown(int button) const;
		int IsMouseButtonUp(int button) const;
		int IsMouseButtonPressed(int button) const;
		int IsMouseButtonReleased(int button) const;

	private:
		static const int MAX_KEYS = 512;
		static const int MAX_MOUSE_BUTTONS = 3;

		struct InputState
		{
			bool m_keys[MAX_KEYS];
			bool m_mouse_buttons[MAX_MOUSE_BUTTONS];
			int m_mouse_pos_x;
			int m_mouse_pos_y;
		};

		InputState *m_current_input_state;
		InputState *m_prev_input_state;

		bool m_user_input_enabled;
		tfw::MessageQueue *m_msg_queue;
	};
}

#endif
