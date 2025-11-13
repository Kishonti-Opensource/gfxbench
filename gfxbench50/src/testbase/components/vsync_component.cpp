/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "vsync_component.h"
#include "time_component.h"
#include "test_base_gfx.h"

// a frame is considered vsynced if fps is between these.
#define VSYNC_LIMIT_MAX 65
#define VSYNC_LIMIT_MIN 55
// vsync checks are made on frames accumulated during this time period
#define VSYNC_CHECK_INTERVAL_MS 1000
// the test is considered vsynced if this portion of the frames are vsynced during a check
#define VSYNC_LIMIT_TRIGGER_TRESHOLD 0.5f
// the test is maxed when this portion of the checks marks the test as vsynced
#define VSYNC_MAXED_TRIGGER_TRESHOLD 0.8f

using namespace GLB;

const char *VSyncComponent::NAME = "VSyncComponent";

VSyncComponent::VSyncComponent(TestBaseGFX *test) : TestComponent(test, NAME)
{
	m_time_component = nullptr;

	m_vsync_frames_accum_vsync = 0;
	m_vsync_time_accum = 0;
	m_vsync_frames_accum = 0;
	m_vsync_limit_count = 0;
}

VSyncComponent::~VSyncComponent()
{
}

bool VSyncComponent::Init()
{
	m_time_component = dynamic_cast<TimeComponent*>(m_test->GetTestComponent(TimeComponent::NAME));
	return m_time_component != nullptr;
}

void VSyncComponent::EndFrame()
{
	KCL::uint32 frame_time = static_cast<uint32_t>(m_time_component->GetFrameRenderTime());

	if (frame_time > (1000 / VSYNC_LIMIT_MAX) && frame_time < (1000 / VSYNC_LIMIT_MIN))
	{
		m_vsync_frames_accum_vsync++;
	}
	m_vsync_time_accum += frame_time;
	m_vsync_frames_accum++;

	if (m_vsync_time_accum > VSYNC_CHECK_INTERVAL_MS)
	{
		if (((float)m_vsync_frames_accum_vsync) / m_vsync_frames_accum > VSYNC_LIMIT_TRIGGER_TRESHOLD)
		{
			m_vsync_limit_count++;
		}
		m_vsync_time_accum = 0;
		m_vsync_frames_accum = 0;
		m_vsync_frames_accum_vsync = 0;
	}
}

void GLB::VSyncComponent::CreateResult(tfw::ResultGroup &result_group)
{
    if (m_vsync_limit_count > 0)
    {
        result_group.flags().push_back("VSYNC_MAXED");
    }
    else
    {
        result_group.flags().push_back("VSYNC_LIMITED");
    }
}
