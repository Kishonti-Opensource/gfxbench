/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __FILL2_TEST_BASE__
#define __FILL2_TEST_BASE__


#include "test_base.h"


class CompressedFillTest2_Base : public GLB::TestBase
{
public:
	CompressedFillTest2_Base(const GlobalTestEnvironment* const gte);
	virtual ~CompressedFillTest2_Base();

protected:
	virtual const char* getUom() const { return "MTexels/s"; }
	virtual bool isWarmup() const { return false; }
	virtual KCL::uint32 indexCount() const { return 0; }
	virtual float getScore() const { return m_score; }

	virtual bool animate(const int time);
	virtual void finishTest();
	virtual void renderApiFinish() = 0;

	KCL::int32 m_score;

	double m_transferred_texels;
	int m_displayed_element_count;
	int m_element_count_step;
	int m_test_stage;
	KCL::uint32 m_start_frame;
	double m_start_time;
};


#endif // __FILL2_TEST_BASE__

