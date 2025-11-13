/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "result.h"
#include "time_component.h"
#include "test_base_gfx.h"
#include "kcl_os.h"
#include "kcl_io.h"
#ifdef DEMO_MODE
#include <Windows.h>
#endif

using namespace GLB;

const char *TimeComponent::NAME = "TimeComponent";

TimeComponent::TimeComponent(TestBaseGFX *test) : TestComponent(test, NAME), m_timer(false)
{
	m_frame_start_time = 0;
	m_frame_time = 0;

	m_animation_time = 0;

	m_start_animation_time = 0;

	m_min_frame_time = 0;
	m_frame_time_limit_enabled = false;

	m_fps = -1.0f,
	m_fps_time = 0;
	m_fps_frames = 0;
}


TimeComponent::~TimeComponent()
{
}


bool TimeComponent::Init()
{
	const TestDescriptor &desc = GetTestDescriptor();

	if (desc.m_fps_limit > 0.0f)
	{
		m_min_frame_time = desc.m_fps_limit > 0.0f ? KCL::uint32(1000.0f / desc.m_fps_limit) : 1u;
		m_frame_time_limit_enabled = m_min_frame_time > 1u;
	}
	else
	{
		m_min_frame_time = 1u;
		m_frame_time_limit_enabled = false;
	}

	if (desc.IsSingleFrame())
	{
		m_start_animation_time = desc.m_single_frame;
	}
	else
	{
		m_start_animation_time = desc.m_start_animation_time;
	}

	m_animation_time = m_start_animation_time;

	return true;
}


void TimeComponent::BeginFrame()
{
	m_test->SetAnimationTime((uint32_t)m_animation_time);
	m_frame_start_time = GetElapsedTime();
}


void TimeComponent::EndFrame()
{
	int64_t now = GetElapsedTime();
	m_frame_time = now - m_frame_start_time;

	if (m_frame_time_limit_enabled && m_frame_time < m_min_frame_time)
	{
		KCL::g_os->Sleep(uint32_t (m_min_frame_time - m_frame_time));

		now = GetElapsedTime();
		m_frame_time = now - m_frame_start_time;
	}


	KCL::uint32 m_old_animation_time = m_test->GetAnimationTime();


	const TestDescriptor &desc = GetTestDescriptor();
	if (desc.IsFixedTime())
	{
		m_animation_time = m_old_animation_time + desc.m_frame_step_time;
	}
	else if (desc.IsSingleFrame())
	{
		m_animation_time = desc.m_single_frame;
	}
	else
	{
		m_animation_time = m_old_animation_time + m_frame_time;
	}


	// Check if we jumped over a timestamp
	if (!m_time_events.empty() && !desc.IsFixedTime())
	{
		for (size_t i = 0; i < m_time_events.size(); i++)
		{
			if (m_animation_time > m_time_events[i] && m_old_animation_time < m_time_events[i])
			{
				m_animation_time = m_time_events[i];
				break;
			}
		}
	}

	if (desc.m_max_rendered_frames >= 0 && (int)m_test->GetFrameCount() >= desc.m_max_rendered_frames)
	{
		m_test->FinishTest();
	}

	if (m_animation_time > (uint32_t)desc.m_play_time)
	{
		if (desc.m_is_endless)
		{
			m_animation_time = m_start_animation_time;
#ifdef DEMO_MODE
			KCL::AssetFile sndFile("gfxb5_dummy.wav");
			PlaySound(TEXT(sndFile.GetFileLocation().c_str()), NULL, SND_FILENAME | SND_ASYNC);

#endif
		}
		else
		{
			m_test->FinishTest();
		}
	}


#ifndef DISABLE_RESULTS
	if (desc.m_fps_window > 0 && m_test->GetNoResultFlag() == false)
	{
		m_fps_frames++;
		m_fps_time += m_frame_time;
		if (m_fps_time >= desc.m_fps_window)
		{
			m_fps = m_fps_frames * (1000.0f / m_fps_time);
			INFO("FPS: %.2f", m_fps);
			m_fps_frames = 0;
			m_fps_time = 0;
		}
	}
#endif
}


void TimeComponent::BeginTest()
{
	m_timer.start();
}


void TimeComponent::AddTimeEvent(int64_t time)
{
	for (size_t i = 0; i < m_time_events.size(); i++)
	{
		if (m_time_events[i] < time)
		{
			m_time_events.insert(m_time_events.begin() + i, time);
			return;
		}
	}

	m_time_events.push_back(time);
}


void TimeComponent::StopTimer()
{
	m_timer.stop();
}


int64_t TimeComponent::GetFrameRenderTime() const
{
	return m_frame_time;
}


int64_t TimeComponent::GetElapsedTime() const
{
	return int64_t(m_timer.elapsed().wall * 1000.0);
}


float TimeComponent::GetFPS() const
{
	return m_fps;
}


void TimeComponent::CreateResult(tfw::ResultGroup &result_group)
{
	assert( !(result_group.results().size() < 1));
	tfw::Result &test_result = result_group.results().back();

	uint32_t elapsed_time = (uint32_t)GetElapsedTime();
	test_result.setElapsedTime(int(elapsed_time));
	test_result.setMeasuredTime(int(elapsed_time)); // for backwards compatibility measured time equals elapsed time
}
