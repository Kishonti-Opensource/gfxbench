/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef STATISTICS_COMPONENT_H
#define STATISTICS_COMPONENT_H

#include <ng/timer.h>
#include "test_component.h"

namespace GLB
{

enum StatisticMode
{
	STATISTICS_MODE_NONE = 0,
	STATISTICS_MODE_RENDER_STATISTIC = 1 << 0,
	STATISTICS_MODE_PER_FRAME_RENDER_TIME = 1 << 1,
	STATISTICS_MODE_RENDER_JOB = 1 << 2
};

class RenderStatistics;
class TimedRenderStatistics;
class PerFrameRenderTimeStatistics;
class RenderJobStatistics;

class StatisticsComponent : public TestComponent
{
public:
	static const int STATISTICS_MODE = STATISTICS_MODE_NONE; //STATISTICS_MODE_PER_FRAME_RENDER_TIME | STATISTICS_MODE_RENDER_STATISTIC;
	static const char *NAME;

	StatisticsComponent(TestBaseGFX *test, int statistic_mode);
	virtual ~StatisticsComponent();

	virtual bool Init();

	virtual void BeginFrame();
	virtual void EndFrame();

	virtual void CreateResult(tfw::ResultGroup &result_group);

private:
	RenderStatistics *m_render_statistics;
	TimedRenderStatistics *m_timed_render_statistics;

	PerFrameRenderTimeStatistics *m_per_frame_render_time_statistics;
	RenderJobStatistics *m_render_job_statistics;

	int m_statistic_mode;
};

}//!GLB

#endif//!STATISTICS_COMPONENT_H
