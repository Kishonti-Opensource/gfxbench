/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef CHARTS_COMPONENT_H
#define CHARTS_COMPONENT_H

#include "test_component.h"

#include <kcl_base.h>

#include <deque>

namespace GLB
{

class TimeComponent;

class ChartsComponent : public TestComponent
{
public:
	static const char *NAME;

	ChartsComponent(TestBaseGFX *test);
	virtual ~ChartsComponent();

	virtual bool Init() override;

	virtual void EndFrame() override;

	virtual void CreateResult(tfw::ResultGroup &result_group) override;

private:
	TimeComponent *m_time_component;

	KCL::uint32 m_last_query_time;
	KCL::uint32 m_query_interval_ms;

	bool m_is_gpu_logging_enabled;
	std::deque<int> m_gpu_freqs;

	std::deque<float> m_cpu_temps;
	bool m_is_temp_logging_enabled;

	std::deque<int> m_cpu_freqs;
	KCL::uint32 m_core_count;

	std::deque<int> m_render_times_min;
	std::deque<int> m_render_times_max;
	std::deque<int> m_render_times_avg;

	KCL::uint32 m_render_time_min;
	KCL::uint32 m_render_time_max;
	KCL::uint32 m_render_times_accum;
	KCL::uint32 m_render_frames_count;

	std::deque<int> m_query_times;


	//ng::cpu_timer loadTimer;
	std::string result_nocharts_;
};

}

#endif
