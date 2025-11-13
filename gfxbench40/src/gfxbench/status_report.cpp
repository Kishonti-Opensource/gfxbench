/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "status_report.h"

#include <kcl_os.h>
#include <kcl_io.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <ng/log.h>
#include <limits.h>

StatusReport::StatusReport()
{
	m_first_report = true;

	m_loop_count = 0;
	m_min_frame_time = INT_MAX;
	m_max_frame_time = -1;
	m_sum_frame_time = 0;

	m_interval = 0.0;
	m_last_report_time = 0.0;
	m_fps_counter = 0;

	m_cpu_cores = KCL::g_os->GetNumOfCPUCores();
}


void StatusReport::SetFilename(const std::string &filename)
{
	m_filename = filename;
}


void StatusReport::SetInterval(int interval)
{
	m_interval = interval;
}


void StatusReport::SetLoopCount(int loop_count)
{
	m_loop_count = loop_count;
}


void StatusReport::OnFrame(int time, int frame_time)
{
	m_fps_counter++;

	m_sum_frame_time += frame_time;
	m_min_frame_time = frame_time < m_min_frame_time ? frame_time : m_min_frame_time;
	m_max_frame_time = frame_time > m_max_frame_time ? frame_time : m_max_frame_time;

	int dt = time - m_last_report_time;
	if (dt < m_interval)
	{
		return;
	}

	Item item;
	item.m_time = float(time);
	item.m_min_frame_time = m_min_frame_time;
	item.m_max_frame_time = m_max_frame_time;
	item.m_avg_frame_time = float(m_sum_frame_time) / float(m_fps_counter);
	item.m_fps = float( float(m_fps_counter) / float(dt) * 1000.0f);
	item.m_battery = float( KCL::g_os->GetBatteryLevel());
	item.m_temp = KCL::g_os->GetCoreTemperature();
	item.m_gpu = float(KCL::g_os->GetCurrentGPUFrequency());
	for (int i = 0; i < m_cpu_cores; i++)
	{
		item.m_cores.push_back(KCL::g_os->GetCurrentCPUFrequency(i));
	}

	m_items.push_back(item);

	// Reset the interval
	m_last_report_time = time;
	m_fps_counter = 0;
	m_min_frame_time = INT_MAX;
	m_max_frame_time = -1;
	m_sum_frame_time = 0;
}


void StatusReport::WriteReport()
{
	const char sep = ',';

	std::stringstream sstream;

	sstream << std::fixed << std::setprecision(2);

	if (m_first_report)
	{
		sstream << "Time (ms)" << sep;
		sstream << "Loop" << sep;
		sstream << "Min frame time" << sep;
		sstream << "Max frame time" << sep;
		sstream << "Avg frame time" << sep;
		sstream << "Avg FPS" << sep;
		sstream << "Battery" << sep;
		sstream << "Temp" << sep;
		sstream << "GPU Frequency" << sep;

		for (size_t i = 0; i < m_cpu_cores; i++)
		{
			sstream << "CPU " << i + 1 << " Frequency";
			if (i < m_cpu_cores - 1)
			{
				sstream << sep;
			}
		}

		sstream << std::endl;
	}

	for (size_t i = 0; i < m_items.size(); i++)
	{
		const Item &item = m_items[i];
		sstream << item.m_time << sep;
		sstream << m_loop_count << sep;
		sstream << item.m_min_frame_time << sep;
		sstream << item.m_max_frame_time << sep;
		sstream << item.m_avg_frame_time << sep;
		sstream << item.m_fps << sep;
		sstream << item.m_battery << sep;
		sstream << item.m_temp << sep;
		sstream << item.m_gpu << sep;
		for (size_t j = 0; j < item.m_cores.size(); j++)
		{
			if (item.m_cores[j] > 0.0f)
			{
				sstream << int(item.m_cores[j] / 1000.0f);
			}
			else
			{
				sstream << int(item.m_cores[j]);
			}

			if (j < m_cpu_cores - 1)
			{
				sstream << sep;
			}
		}

		sstream << std::endl;
	}

	KCL::File file(m_filename, m_first_report ? KCL::Write : KCL::Append, KCL::RWDir, true);
	if (file.Opened())
	{
		file.Write(sstream.str());
		file.Close();

		NGLOG_INFO("Log appended to: %s", file.GetFileLocation());
	}
	else
	{
		NGLOG_ERROR("Can not open file: %s", m_filename);
	}

	m_items.clear();

	m_first_report = false;
}
