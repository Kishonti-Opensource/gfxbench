/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "alu_base.h"


ALUTest_Base::ALUTest_Base(const GlobalTestEnvironment * const gte) : TestBase(gte),
	m_score(0)
{

}


ALUTest_Base::~ALUTest_Base()
{

}


bool ALUTest_Base::animate(const int time)
{
	SetAnimationTime(time);
	if (m_frames > m_score)
	{
		m_score = m_frames;
	}

	return time < m_settings->m_play_time;
}