/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "charts_component.h"
#include "time_component.h"
#include "test_base_gfx.h"

#include "chart.h"
#include "result.h"

#include <kcl_os.h>
#include <kcl_math3d.h>
#include <sstream>

using namespace GLB;

const char *ChartsComponent::NAME = "ChartsComponent";

ChartsComponent::ChartsComponent(TestBaseGFX *test) : TestComponent(test, NAME)
{
	m_query_interval_ms = 0;
	m_last_query_time = 0;

	m_core_count = 0;

	m_render_frames_count = 0;
	m_render_time_max = 0;
	m_render_time_min = 0;
}


ChartsComponent::~ChartsComponent()
{
}


bool ChartsComponent::Init()
{
	m_time_component = dynamic_cast<TimeComponent*>(m_test->GetTestComponent(TimeComponent::NAME));
	if (m_time_component == nullptr)
	{
		return false;
	}

	m_query_interval_ms = GetTestDescriptor().m_charts_query_frequency;

	m_is_temp_logging_enabled = KCL::g_os->GetCoreTemperature() > 0;

	m_is_gpu_logging_enabled = KCL::g_os->GetCurrentGPUFrequency() >= 0;

	int core_count = KCL::g_os->GetNumOfCPUCores();
	m_core_count = core_count >= 0 ? KCL::uint32(core_count) : 0u;

	if (m_core_count > 0)
	{
		// TODO: When number of CPU cores returned, but no frequency data
		if (KCL::g_os->GetCurrentCPUFrequency(0) <= 0)
		{
			m_core_count = 0;
		}
	}

	return true;
}


void ChartsComponent::EndFrame()
{
	m_render_frames_count++;

	KCL::uint32 render_time = static_cast<uint32_t>(m_time_component->GetFrameRenderTime());
	m_render_times_accum += render_time;

	m_render_time_max = KCL::Max(render_time, m_render_time_max);

	if (m_render_time_min == 0)
	{
		m_render_time_min = render_time;
	}
	else
	{
		m_render_time_min = KCL::Min(render_time, m_render_time_min);
	}

	KCL::uint32 now = static_cast<uint32_t>(m_time_component->GetElapsedTime());
	if (now - m_last_query_time > m_query_interval_ms)
	{
		m_last_query_time = now;

		if (m_is_temp_logging_enabled)
		{
			m_cpu_temps.push_back(KCL::g_os->GetCoreTemperature());
		}

		if (m_is_gpu_logging_enabled)
		{
			m_gpu_freqs.push_back(KCL::g_os->GetCurrentGPUFrequency());
		}

		for (KCL::uint32 i = 0; i < m_core_count; i++)
		{
			m_cpu_freqs.push_back( KCL::g_os->GetCurrentCPUFrequency(i));
		}

		m_render_times_min.push_back(m_render_time_min);
		m_render_times_max.push_back(m_render_time_max);
		m_render_times_avg.push_back(m_render_times_accum / m_render_frames_count);

		m_render_frames_count = 0;
		m_render_times_accum = 0;

		m_render_time_max = 0;
		m_render_time_min = 0;

		m_query_times.push_back(now);
	}
}


void ChartsComponent::CreateResult(tfw::ResultGroup &result_group)
{
	//
	if (m_query_times.empty() == false)
	{
		m_query_times.pop_front();
		m_render_times_avg.pop_front();
		m_render_times_max.pop_front();
		m_render_times_min.pop_front();
	}
	//

	tfw::Samples time_domain;
	time_domain.setName("Time (s)");
	for (size_t i = 0; i < m_query_times.size(); i++)
	{
		time_domain.addValue(double(m_query_times[i]) / 1000.0);
	}

	{
		tfw::Chart chart;
		chart.setChartID("Frametimes");
		chart.setDomainAxis("");
		chart.setSampleAxis("Frametimes (ms)");

		chart.setDomain(time_domain);

		chart.values().resize(3);
		chart.values()[0].setName("Minimum");
		chart.values()[0].values().resize(m_render_times_min.size());
		chart.values()[0].values().assign(m_render_times_min.begin(), m_render_times_min.end());
		chart.values()[1].setName("Maximum");
		chart.values()[1].values().resize(m_render_times_max.size());
		chart.values()[1].values().assign(m_render_times_max.begin(), m_render_times_max.end());
		chart.values()[2].setName("Average");
		chart.values()[2].values().resize(m_render_times_avg.size());
		chart.values()[2].values().assign(m_render_times_avg.begin(), m_render_times_avg.end());

		result_group.addChart(chart);
	}

	if (m_cpu_temps.size())
	{
		tfw::Chart chart;

		chart.setChartID("Temperature");
		chart.setDomainAxis("");
		chart.setSampleAxis("Temperature (C)");

		chart.setDomain(time_domain);

		chart.values().resize(1);
		chart.values()[0].setName("Temperature (C)");
		chart.values()[0].values().resize(m_cpu_temps.size());
		chart.values()[0].values().assign(m_cpu_temps.begin(), m_cpu_temps.end());

		result_group.addChart(chart);
	}

	if (m_cpu_freqs.size() > 0 || m_gpu_freqs.size() > 0)
	{
		tfw::Chart chart;

		chart.setChartID("Performance");
		chart.setDomainAxis("");
		chart.setSampleAxis("Clock speed (MHz)");

		chart.setDomain(time_domain);

		chart.values().resize(m_core_count + (m_gpu_freqs.size() > 0 ? 1 : 0));

		if (m_cpu_freqs.size() > 0)
		{
			for (unsigned int core = 0; core < m_core_count; ++core)
			{
				std::stringstream sstream;
				sstream << "CPU" << (core + 1);

				chart.values()[core].setName(sstream.str());
				size_t CPUfreqsSize = m_cpu_freqs.size() / m_core_count;
				chart.values()[core].values().resize(CPUfreqsSize);
				for (size_t i = 0; i < CPUfreqsSize; ++i)
				{
					chart.values()[core].values()[i] = m_cpu_freqs[i * m_core_count + core];
				}
			}
		}

		if (m_gpu_freqs.size() > 0)
		{
			chart.values()[m_core_count].setName("GPU");
			chart.values()[m_core_count].values().assign(m_gpu_freqs.begin(), m_gpu_freqs.end());
		}

		result_group.addChart(chart);
	}
}
