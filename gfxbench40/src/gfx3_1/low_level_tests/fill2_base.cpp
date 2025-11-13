/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "fill2_base.h"


CompressedFillTest2_Base::CompressedFillTest2_Base(const GlobalTestEnvironment* const gte)
	: TestBase(gte)
{
	m_score = 0;

	m_transferred_texels = 0.0;
	m_displayed_element_count = 28;
	m_element_count_step = 4;
	m_test_stage = 0;

	m_start_frame = 0;
	m_start_time = 0;
}


CompressedFillTest2_Base::~CompressedFillTest2_Base()
{

}


bool CompressedFillTest2_Base::animate(const int time)
{
	if (WasKeyPressed(82)) // button R
	{
		INFO("Reload...\n");
		FreeResources();
		init();
	}

	SetAnimationTime(time);

	double elapsed_time = 0.001f *  (KCL::g_os->GetTimeMilliSec() - m_start_time);

	switch (m_test_stage)
	{
	case 0:
		m_start_time = KCL::g_os->GetTimeMilliSec();
		m_start_frame = m_frames;
		m_test_stage = 1;
		return true;

		// Adjust number of displayed items
	case 1:
	{
		if (elapsed_time < 0.2f)
		{
			return true;
		}

		renderApiFinish();
		double now = KCL::g_os->GetTimeMilliSec();

		int frame_count = m_frames - m_start_frame;
		double frame_time = now - m_start_time;
		float fps = float(1000.0 * double(frame_count) / frame_time);

		m_start_time = now;
		m_start_frame = m_frames;

		// Spend at most half the play time with adjustment			
		if (time > m_settings->m_play_time * 0.5f)
		{
			m_test_stage = 2;
			return true;
		}
		if (fps > 30)
		{
			m_element_count_step = m_element_count_step > 0 ? m_element_count_step : (-m_element_count_step / 2);
			if (!m_element_count_step)
			{
				m_element_count_step = 4;
			}

			m_displayed_element_count += m_element_count_step;
		}
		else if (fps < 25)
		{
			m_element_count_step = m_element_count_step < 0 ? (m_element_count_step / 2) : (-m_element_count_step / 2);
			if (!m_element_count_step)
			{
				m_element_count_step = -4;
			}

			m_displayed_element_count += m_element_count_step;
			if (m_displayed_element_count < 1)
			{
				m_displayed_element_count = 1;
			}
		}
		else
		{
			m_test_stage = 2;
		}
		return true;
	}

	// Number of items is set, wait for fps settle...
	case 2:
	{
		// Wait for frame rate to settle
		if (elapsed_time > 1.0)
		{
			renderApiFinish(); // Finish the warm up
			m_start_time = KCL::g_os->GetTimeMilliSec();
			m_start_frame = m_frames;
			m_transferred_texels = 0;
			m_test_stage = 3;
		}

		return true;
	}

	case 3:
	{
		return time < m_settings->m_play_time;
	}

	default:
		return time < m_settings->m_play_time;
	}
}


void CompressedFillTest2_Base::finishTest()
{
	TestBase::finishTest();
	// Perform measurement
	double elapsed_time = 0.001 * (KCL::g_os->GetTimeMilliSec() - m_start_time);
	m_score = m_transferred_texels / elapsed_time / (1024 * 1024);
}

