/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "driver_base.h"


CPUOverheadTest_Base::CPUOverheadTest_Base(const GlobalTestEnvironment * const gte) : TestBase(gte)
{
    m_score = 0 ;
}


CPUOverheadTest_Base::~CPUOverheadTest_Base()
{

}


bool CPUOverheadTest_Base::animate(const int time)
{
	SetAnimationTime(time);

	int score = m_frames;
	if (score > m_score)
	{
		m_score = score;
	}

	return time < m_settings->m_play_time;
}
