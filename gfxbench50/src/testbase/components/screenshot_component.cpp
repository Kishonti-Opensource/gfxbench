/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "screenshot_component.h"
#include "time_component.h"
#include "test_base_gfx.h"

#include <iomanip>
#include <sstream>

using namespace GLB;

const char *ScreenshotComponent::NAME = "ScreenshotComponent";

ScreenshotComponent::ScreenshotComponent(TestBaseGFX *test) : TestComponent(test, NAME)
{
	m_time_component = nullptr;
}


ScreenshotComponent::~ScreenshotComponent()
{
}


bool ScreenshotComponent::Init()
{
	m_time_component = dynamic_cast<TimeComponent*>(m_test->GetTestComponent(TimeComponent::NAME));
	if (m_time_component == nullptr)
	{
		return false;
	}

	const std::vector<int> frames = GetTestDescriptor().m_screenshot_frames;

	for (size_t i = 0; i < frames.size(); i++)
	{
		m_screenshot_frames.insert(KCL::uint32(frames[i]));
		
		if (GetTestDescriptor().IsFixedTime() == false)
		{
			m_time_component->AddTimeEvent(KCL::uint32(frames[i]));
		}
	}	
		
	return true;
}


void ScreenshotComponent::AfterRender()
{
	if (IsScreenshotMode())
	{
		const TestDescriptor &desc = GetTestDescriptor();
		KCL::uint32 frame_count = m_test->GetFrameCount();

		KCL::uint32 animation_time = m_test->GetAnimationTime();

		if (VIDEO_CAPTURE_MODE)
		{	
			std::string filename;
			GetScreenshotNameForFrame(filename, frame_count);

			m_test->CreateScreenshot(filename.c_str());
		}
		else if (desc.IsFixedTime() || desc.IsSingleFrame())
		{
			if (m_screenshot_frames.find(frame_count) != m_screenshot_frames.end())
			{
				std::string filename;
				GetScreenshotNameForTime(filename, desc.m_name, animation_time);
				
				m_test->CreateScreenshot(filename.c_str());
			}			 
		}
		else if (m_screenshot_frames.find(animation_time) != m_screenshot_frames.end())
		{
			std::string filename;
			GetScreenshotNameForTime(filename, desc.m_name, animation_time);

			m_test->CreateScreenshot(filename.c_str());					
		}
	}
}


bool ScreenshotComponent::IsScreenshotMode() const
{
	return !m_screenshot_frames.empty() || VIDEO_CAPTURE_MODE;
}


void ScreenshotComponent::GetScreenshotNameForFrame(std::string &out, KCL::uint32 frame)
{
	std::stringstream sstream;

	sstream << std::setw(8) << std::setfill('0') << frame;

	out = sstream.str();
}


void ScreenshotComponent::GetScreenshotNameForTime(std::string &out, const std::string &test_name, KCL::uint32 time)
{
	std::stringstream sstream;

	sstream << "scr_" << test_name << '_';
	sstream << std::setw(6) << std::setfill('0') << time;

	out = sstream.str();
}
