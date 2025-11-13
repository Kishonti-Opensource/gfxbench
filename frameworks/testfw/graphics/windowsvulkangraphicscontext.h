/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef WINDOWSVULKANGRAPHICSCONTEXT_H
#define WINDOWSVULKANGRAPHICSCONTEXT_H

#include "vulkangraphicscontext.h"
#include <Windows.h>

class WindowsVulkanGraphicsContext : public VulkanGraphicsContext
{
public:
	WindowsVulkanGraphicsContext(HWND hwnd) : hwnd_(hwnd) {}
	~WindowsVulkanGraphicsContext() {};

	HWND hwnd()
	{
		return hwnd_;
	}

protected:
	HWND hwnd_;
};

#endif //!WINDOWSVULKANGRAPHICSCONTEXT_H
