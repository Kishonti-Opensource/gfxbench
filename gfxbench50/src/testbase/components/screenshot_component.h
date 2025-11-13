/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef SCREENSHOT_COMPONENT_H
#define SCREENSHOT_COMPONENT_H

#include "test_component.h"
#include <set>
#include <kcl_base.h>

namespace GLB
{

class TimeComponent;

class ScreenshotComponent : public TestComponent
{
	static const bool VIDEO_CAPTURE_MODE = false;

public:
	static const char *NAME;

	ScreenshotComponent(TestBaseGFX *test);
	virtual ~ScreenshotComponent();

	virtual bool Init();
	virtual void AfterRender();

	bool IsScreenshotMode() const;

private:
	TimeComponent *m_time_component;

	std::set<KCL::uint32> m_screenshot_frames;

	static void GetScreenshotNameForFrame(std::string &out, KCL::uint32 frame);
	static void GetScreenshotNameForTime(std::string &out, const std::string &test_name, KCL::uint32 time);
};

}

#endif
