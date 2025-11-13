/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "AndroidVulkanGraphicsContext.h"
#include "ng/log.h"
#include <android/native_window.h>


bool AndroidVulkanGraphicsContext::initWindowSurface(ANativeWindow *window)
{
	NGLOG_INFO("Initializing Vulkan context");
	window_ = window;

	return true;
}
