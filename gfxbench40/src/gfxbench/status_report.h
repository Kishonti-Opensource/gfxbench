/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef STATUS_REPORT_H
#define STATUS_REPORT_H

#include <string>
#include <vector>

class StatusReport
{
public:
	StatusReport();

	void SetFilename(const std::string &filename);
	void SetInterval(int interval);
	void SetLoopCount(int loop_count);

	void OnFrame(int time, int frame_time);

	void WriteReport();

private:
	std::string m_filename;
	double m_interval;

	bool m_first_report;

	int m_cpu_cores;

	int m_min_frame_time;
	int m_max_frame_time;
	int m_sum_frame_time;

	int m_loop_count;
	int m_fps_counter;
	int m_last_report_time;

	struct Item
	{
		int m_time; // ms
		int m_min_frame_time;
		int m_max_frame_time;
		float m_avg_frame_time;
		float m_fps;
		float m_battery;
		float m_temp;
		float m_gpu;
		std::vector<float> m_cores;
	};
	std::vector<Item> m_items;

};

#endif
