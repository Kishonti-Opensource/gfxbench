/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#if GLB_USE_SYSTEM_MESSAGER

#include <Windows.h>

namespace GLB
{
	
class SystemMessager
{
	static const char* m_test_ids[17];

public:
	enum MessageEvents
	{
		UNSPECIFIED = 0,
		LOAD_BEGIN = 1,
		LOAD_END = 2,
		TEST_BEGIN = 3,
		TEST_END = 4
	};

private:
	UINT m_message_handle;
	int m_test_index;

public:
	SystemMessager(const char *test_id)
	{
		m_message_handle = 0;
		m_test_index = -1;

		const int num_tests = sizeof(m_test_ids) / sizeof(m_test_ids[0]);
		for (int i = 0; i < num_tests; i++)
		{
			if (strcmp(test_id, m_test_ids[i]) == 0)
			{
				m_test_index = i;
			}
		}

		if (m_test_index < 0)
		{
			m_test_index = 0;
		}

		m_message_handle = RegisterWindowMessage("GFXBench5Message");
	}

	void Send(MessageEvents message_event)
	{
		SendMessage(HWND_BROADCAST, m_message_handle, m_test_index, (int)message_event);
	}
};


const char* SystemMessager::m_test_ids[17] = {
	"unknown",
	"dx_5_high",
	"dx_5_high_off",
	"dx_5_normal",
	"dx_5_normal_off",
	"dx12_5_high",
	"dx12_5_high_off",
	"dx12_5_normal",
	"dx12_5_normal_off",
	"gl_5_high",
	"gl_5_high_off",
	"gl_5_normal",
	"gl_5_normal_off",
	"vulkan_5_high",
	"vulkan_5_high_off",
	"vulkan_5_normal",
	"vulkan_5_normal_off"
};

}

#else
	
namespace GLB
{
	class SystemMessager
	{
	};
}

#endif
