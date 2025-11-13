/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "render_statistics.h"

#include "ngl.h"
#include "kcl_base.h"
#include "ng/log.h"

#include "common/gfxb_shot_handler.h"

#include <set>
#include <unordered_map>
#include <unordered_set>
#include <fstream>

using namespace GLB;

static const char SEPARATOR = ',';

#ifndef UINT64_MAX
#define UINT64_MAX       18446744073709551615ULL
#endif

template <typename T>
std::string to_string(T tmp)
{
	std::ostringstream out;
	out << tmp;
	return out.str();
}


RenderStatistics::RenderStatistics(KCL::uint32 test_width, KCL::uint32 test_height)
{
	m_test_width = test_width;
	m_test_height = test_height;
}


RenderStatistics::~RenderStatistics()
{

}


void RenderStatistics::AddFrame(const NGLStatistic& frame)
{
	if (frame.IsEnabled())
	{
		m_frames_stats.push_back(frame);
	}
}

uint32_t prev_shot = 0;
void TimedRenderStatistics::AddFrame(const NGLStatistic& frame, uint32_t anim_time, GFXB::ShotHandler *shot_handler)
{
	if (frame.IsEnabled())
	{
		uint32_t current_shot = shot_handler != nullptr ? shot_handler->GetCurrentCameraShotIndex() : 0;

		if (current_shot != prev_shot)

		//if (current_time >= /*(1.0 / 2)*/ 40.0 / 1000.0)
		{
			prev_shot = current_shot;

			//CalculatePartialStatistics(m_global_time.elapsed().wall - m_start_time, shot_index->GetCurrentCameraShotIndex());
			CalculatePartialStatistics(anim_time, shot_handler == nullptr ? 0 : current_shot - 1);
			m_save_timer.start();
		}
		m_frames_stats.push_back(frame);
	}
}


void GLB::TimedRenderStatistics::CalculatePartialStatistics(double time, uint32_t shot_index)
{
	KCL::uint32 frame_count = KCL::uint32(m_frames_stats.size());
	if (!frame_count)
	{
		return;
	}

	for (size_t i = 0; i < frame_count; ++i)
	{
		CalculateFrame(m_frames_stats[i]);
	}

	//calculate AVG of stat's elements
	m_draw_calls.SetFrames(frame_count);
	for (size_t i = 0; i < NGL_STATISTIC_COUNT; ++i)
	{
		m_query_values[i].SetFrames(frame_count);
	}
	m_overdraw.SetFrames(frame_count);

	shot_indexes[time] = shot_index;
	partial_overdraw[time] = m_overdraw;
	partial_drawcalls[time] = m_draw_calls;
	for (int k = 0; k < NGL_STATISTIC_COUNT; ++k)
	{
		partial_query_values[k][time] = m_query_values[k];
	}


	m_draw_calls.Clear();
	m_frames_stats.clear();
	m_texture_count.Clear();
	m_overdraw.Clear();
	for (size_t i = 0; i < NGL_STATISTIC_COUNT; ++i)
	{
		m_query_values[i].Clear();
	}
}


void GLB::TimedRenderStatistics::ExportStatistics(const char *filename)
{
#define HORIZONTAL_MODE 1
#if HORIZONTAL_MODE
	NGLOG_INFO("TimedRenderStatistics - Exporting statistics to %s", filename);

	std::fstream csv(filename, std::ios::out);
	if (!csv)
	{
		NGLOG_ERROR("TimedRenderStatistics: Can not create file: %s", filename);
		return;
	}

	csv.precision(2);
	csv.setf(std::ios::fixed, std::ios::floatfield);

	const double target_draw_count_low = 400;
	const double target_poly_count_low = 270 * 1000;

	const double target_draw_count_normal = 1000;
	const double target_poly_count_normal = 540 * 1000;

	const double target_draw_count_high = 2000;
	const double target_poly_count_high = 1000 * 1000;

	std::string avg_draw_col = "F";
	std::string avg_poly_col = "L";

	const int NUM_OF_HEADERS = 11;
	const char* header[NUM_OF_HEADERS] = {
		"Primitives submitted",//0
		"Vertices submitted",//1
		"VS invocations",//2
		"TCS patches",//3
		"TES invocations",//4
		"GS invocations",//5
		"GS primitives emitted",//6
		"FS invocations",//7
		"Clipping input primitives",//8
		"Clipping output primitives",//9
		"Samples passed"//10
	};


	csv << "Frametime" << SEPARATOR << "Frame" << SEPARATOR << "Shot" << SEPARATOR << "Draw calls - min" << SEPARATOR << "Draw calls - max" << SEPARATOR << "Draw calls - avg" << SEPARATOR;
	csv << "Overdraw - min" << SEPARATOR << "Overdraw - max" << SEPARATOR << "Overdraw - avg" << SEPARATOR;
	for (int k = 0; k < NUM_OF_HEADERS; ++k)
	{
		csv << header[k] << " - min" << SEPARATOR << header[k] << " - max" << SEPARATOR << header[k] << " - avg" << SEPARATOR;
	}

	csv << "Draw low" << SEPARATOR << "Poly low" << SEPARATOR;
	csv << "Draw normal" << SEPARATOR << "Poly normal" << SEPARATOR;
	csv << "Draw high" << SEPARATOR << "Poly high" << SEPARATOR;

	csv << std::endl;

	std::map<double, Value>::iterator it2[NUM_OF_HEADERS];
	for (int k = 0; k < NUM_OF_HEADERS; ++k)
	{
		it2[k] = partial_query_values[k].begin();
	}

	int row_counter = 2;
	std::map<double, Value>::iterator it_overdraw = partial_overdraw.begin();
	std::map<double, uint32_t>::iterator it_shot = shot_indexes.begin();
	for (std::map<double, Value>::iterator it = partial_drawcalls.begin(); it != partial_drawcalls.end(); ++it, ++it_overdraw, ++it_shot)
	{
		csv << it->first << SEPARATOR;//<--write frametime
		csv << it->first / (1000.0 / 24.0) << SEPARATOR;//<--write frame
		csv << it_shot->second + 1 << SEPARATOR;//<--write shot index
		csv << it->second.Min() << SEPARATOR << it->second.Max() << SEPARATOR << it->second.Avg() << SEPARATOR;//<--write draw calls
		csv << it_overdraw->second.Min() << SEPARATOR << it_overdraw->second.Max() << SEPARATOR << it_overdraw->second.Avg() << SEPARATOR;//<--write overdraw
		for (int k = 0; k < NUM_OF_HEADERS; ++k)//<--write query statistics
		{
			{
				Value value(it2[k]->second);
				csv << value.Min() << SEPARATOR << value.Max() << SEPARATOR << value.Avg() << SEPARATOR;
			}
			++it2[k];
		}

		csv << "=" << avg_draw_col << "-" << target_draw_count_low << row_counter << SEPARATOR;
		csv << "=" << avg_poly_col << "-" << target_poly_count_low << row_counter << SEPARATOR;

		csv << "=" << avg_draw_col << "-" << target_draw_count_normal << row_counter << SEPARATOR;
		csv << "=" << avg_poly_col << "-" << target_poly_count_normal << row_counter << SEPARATOR;

		csv << "=" << avg_draw_col << "-" << target_draw_count_high << row_counter << SEPARATOR;
		csv << "=" << avg_poly_col << "-" << target_poly_count_high << row_counter << SEPARATOR;

		csv << std::endl;

		row_counter++;
	}

	csv.close();
#else
	NGLOG_INFO("TimedRenderStatistics - Exporting statistics to %s", filename);

	std::fstream csv(filename, std::ios::out);
	if (!csv)
	{
		NGLOG_ERROR("TimedRenderStatistics: Can not create file: %s", filename);
		return;
	}

	csv.precision(16);

	csv << "Frametime" << SEPARATOR << "Draw calls - min" << SEPARATOR << "Draw calls - max" << SEPARATOR << "Draw calls - avg" << std::endl;
	for (std::map<double, Value>::iterator it = partial_drawcalls.begin(); it != partial_drawcalls.end(); it++)
	{
		Write(csv, it->first, it->second);
	}

	const int NUM_OF_HEADERS = 11;
	const char* header[NUM_OF_HEADERS] = {
		"Primitives submitted",//0
		"Vertices submitted",//1
		"VS invocations",//2
		"TCS patches",//3
		"TES invocations",//4
		"GS invocations",//5
		"GS primitives emitted",//6
		"FS invocations",//7
		"Clipping input primitives",//8
		"Clipping output primitives",//9
		"Samples passed"//10
	};

	for (int k = 0; k < NUM_OF_HEADERS; ++k)
	{
		WriteHeader(csv, header[k]);
		for (std::map<double, Value>::iterator it = partial_query_values[k].begin(); it != partial_query_values[k].end(); it++)
		{
			Write(csv, it->first, it->second);
		}
	}
	csv.close();
#endif
}


void GLB::TimedRenderStatistics::WriteHeader(std::fstream &csv, const char* title)
{
	csv << "" << SEPARATOR << title << " - min" << SEPARATOR << title << " - max" << SEPARATOR << title << " - avg" << std::endl;
}


void RenderStatistics::CalculateStatistics()
{
	KCL::uint32 frame_count = KCL::uint32(m_frames_stats.size());
	if (!frame_count)
	{
		return;
	}

	NGLOG_INFO("RenderStatistics - Calculating statistics for %s frames...", frame_count);

	for (size_t i = 0; i < frame_count; ++i)
	{
		CalculateFrame(m_frames_stats[i]);
	}

	//calculate AVG of stat's elements
	m_draw_calls.SetFrames(frame_count);
	for (size_t i = 0; i < NGL_STATISTIC_COUNT; ++i)
	{
		m_query_values[i].SetFrames(frame_count);
	}

	m_tess_draw_calls.SetFrames(frame_count);
	m_gs_draw_calls.SetFrames(frame_count);

	m_compute_dispatch_count.SetFrames(frame_count);

	m_texture_count.SetFrames(frame_count);

	m_overdraw.SetFrames(frame_count);
}


void RenderStatistics::CalculateFrame(const NGLStatistic &frame)
{
	double draw_calls = 0.0;
	double compute_per_frame = 0.0;
	KCL::uint64 query_values[NGL_STATISTIC_COUNT] = { 0 };

	for (std::vector<job_statistics>::const_iterator it = frame.jobs.begin(); it != frame.jobs.end(); ++it)
	{
		compute_per_frame += (double)it->m_dispatch_count;
		for (size_t j = 0; j < it->m_sub_pass.size(); ++j)
		{
			draw_calls += it->m_sub_pass[j].m_draw_calls.size();
			for (size_t h = 0; h < it->m_sub_pass[j].m_draw_calls.size(); ++h)
			{
				for (int k = 0; k < NGL_STATISTIC_COUNT; ++k)
				{
					query_values[k] += it->m_sub_pass[j].m_draw_calls[h].m_query_results[k];
				}
			}
		}
	}
	m_compute_dispatch_count.MinMaxAvg(compute_per_frame);
	m_texture_count.MinMaxAvg((double)frame.m_used_textures.size());

	//const double pixels = double(m_test_width * m_test_height);
	const double pixels = 1920.0 * 1080.0;
	double overdraw = query_values[NGL_STATISTIC_SAMPLES_PASSED] / pixels;
	m_overdraw.MinMaxAvg(overdraw);

	m_draw_calls.MinMaxAvg(draw_calls);
	for (size_t i = 0; i < NGL_STATISTIC_COUNT; ++i)
	{
		m_query_values[i].MinMaxAvg((double)query_values[i]);
	}
}


/*
void RenderStatistics::CalculateFrame(const std::vector<pass_statistics> &frame)
{
	KCL::uint64 draw_calls = 0;
	KCL::uint64 tess_draw_calls = 0;
	KCL::uint64 gs_draw_calls = 0;

	KCL::uint64 dispatch_count = 0;

	std::set<KCL::uint64> textures;

	KCL::uint64 query_results[draw_call_statistics::QUERY_COUNT];
	memset(query_results, 0, sizeof(query_results));

	for (size_t i = 0; i < frame.size(); i++)
	{
		const pass_statistics &pass = frame[i];

		draw_calls += pass.m_draw_calls.size();

		if (pass.m_type == pass_statistics::TYPE_COMPUTE_PASS)
		{
			dispatch_count += pass.m_invocation_count;
		}

		for (size_t j = 0; j < pass.m_draw_calls.size(); j++)
		{
			const draw_call_statistics &draw = pass.m_draw_calls[j];

			if (draw.m_has_tessellation_shader)
			{
				tess_draw_calls++;
			}

			if (draw.m_has_geometry_shader)
			{
				gs_draw_calls++;
			}

			textures.insert(draw.m_textures.begin(), draw.m_textures.end());

			for (size_t k = 0; k < draw_call_statistics::QUERY_COUNT; k++)
			{
				query_results[k] += draw.m_query_results[k];
			}
		}
	}

	m_draw_calls.MinMaxAvg(draw_calls);
	m_tess_draw_calls.MinMaxAvg(tess_draw_calls);
	m_gs_draw_calls.MinMaxAvg(gs_draw_calls);

	m_texture_count.MinMaxAvg(textures.size());

	m_compute_dispatch_count.MinMaxAvg(dispatch_count);

	for (size_t i = 0; i < draw_call_statistics::QUERY_COUNT; i++)
	{
		m_query_values[i].MinMaxAvg(query_results[i]);
	}

	double overdraw = query_results[draw_call_statistics::SAMPLES_PASSED] / double(m_test_width * m_test_height);
	m_overdraw.MinMaxAvg(overdraw);
}
*/


void RenderStatistics::ExportStatistics(const char *filename)
{
	NGLOG_INFO("RenderStatistics - Exporting statistics to %s", filename);

	std::fstream csv(filename, std::ios::out);
	if (!csv)
	{
		NGLOG_ERROR("RenderStatistics: Can not create file: %s", filename);
		return;
	}

	csv.precision(16);
	//csv << std::fixed;

	Write(csv, "Draw calls", m_draw_calls);

	Write(csv, "Primitives submitted", m_query_values[NGL_STATISTIC_PRIMITIVES_SUBMITED]);

	Write(csv, "Vertices submitted", m_query_values[NGL_STATISTIC_VERTICES_SUBMITED]);
	Write(csv, "VS invocations", m_query_values[NGL_STATISTIC_VS_INVOCATIONS]);
	Write(csv, "TCS patches", m_query_values[NGL_STATISTIC_TCS_PATCHES]);
	Write(csv, "TES invocations", m_query_values[NGL_STATISTIC_TES_INVOCATIONS]);

	Write(csv, "GS invocations", m_query_values[NGL_STATISTIC_GS_INVOCATIONS]);
	Write(csv, "GS primitives emitted", m_query_values[NGL_STATISTIC_GS_PRIMITIVES_EMITTED]);
	Write(csv, "FS invocations", m_query_values[NGL_STATISTIC_FS_INVOCATIONS]);
	Write(csv, "Clipping input primitives", m_query_values[NGL_STATISTIC_CLIPPING_INPUT_PRIMITIVES]);
	Write(csv, "Clipping output primitives", m_query_values[NGL_STATISTIC_CLIPPING_OUTPUT_PRIMITIVES]);

	Write(csv, "Texture count", m_texture_count);
	Write(csv, "Samples passed", m_query_values[NGL_STATISTIC_SAMPLES_PASSED]);

	Write(csv, "Overdraw rate", m_overdraw);
	Write(csv, "Compute dispatch count", m_compute_dispatch_count);

	//Additional details:
	Write(csv, "Draw calls - tessellated", m_tess_draw_calls);//TODO
	Write(csv, "Draw calls - using GS", m_gs_draw_calls);//TODO
	csv.close();
}


void RenderStatistics::Write(std::fstream &stream, const char *key, const Value &value)
{
	NGLOG_INFO("%s - min: %s", key, KCL::uint32(value.Min()));
	NGLOG_INFO("%s - max: %s", key, KCL::uint32(value.Max()));
	NGLOG_INFO("%s - avg: %s", key, KCL::uint32(value.Avg()));

	stream << key << " - min" << SEPARATOR << value.Min() << std::endl;
	stream << key << " - max" << SEPARATOR << value.Max() << std::endl;
	stream << key << " - avg" << SEPARATOR << value.Avg() << std::endl;
}


void GLB::RenderStatistics::Write(std::fstream &csv, double key, const Value &value)
{
	csv << key << SEPARATOR << value.Min() << SEPARATOR << value.Max() << SEPARATOR << value.Avg() << std::endl;
}


RenderStatistics::Value::Value()
{
	m_inited = false;

	m_min_value = 0.0;
	m_max_value = 0.0;
	m_avg_value = 0.0;
}


double RenderStatistics::Value::Min() const
{
	return m_inited ? m_min_value : 0.0;
}


double RenderStatistics::Value::Max() const
{
	return m_inited ? m_max_value : 0.0;
}


double RenderStatistics::Value::Avg() const
{
	return m_inited ? m_avg_value : 0.0;
}


void RenderStatistics::Value::SetFrames(KCL::uint32 frame_count)
{
	m_avg_value /= double(frame_count);
}


void RenderStatistics::Value::MinMaxAvg(double value)
{
	if (m_inited)
	{
		m_min_value = value > m_min_value ? m_min_value : value;
		m_max_value = value < m_max_value ? m_max_value : value;

		m_avg_value += value;
	}
	else
	{
		m_min_value = value;
		m_max_value = value;
		m_avg_value = value;
		m_inited = true;
	}
}


void GLB::RenderStatistics::Value::Clear()
{
	m_inited = false;

	m_min_value = 0.0;
	m_max_value = 0.0;
	m_avg_value = 0.0;
}


//
//	PerFrameRenderTimeStatistics
//

void PerFrameRenderTimeStatistics::AddFrame(const NGLStatistic& frame, uint32_t anim_time)
{
	if (frame.IsEnabled())
	{
		m_frames_stats.push_back(frame);
		m_frame_times.push_back(anim_time);
	}
}


void PerFrameRenderTimeStatistics::CalculateStatistics()
{
	m_job_time_stat.resize(m_frames_stats.size());
	m_jobs.clear();

	std::set<std::string> jobs;
	for (uint32_t i = 0; i < m_frames_stats.size(); i++)
	{
		const NGLStatistic &frame = m_frames_stats[i];

		for (std::vector<job_statistics>::const_iterator j_it = frame.jobs.begin(); j_it != frame.jobs.end(); ++j_it)
		{
			if (!j_it->m_is_active)
			{
				continue;
			}

			for (size_t j = 0; j < j_it->m_sub_pass.size(); ++j)
			{
				const job_statistics::sub_pass_stats &subpass = j_it->m_sub_pass[j];

				jobs.insert(subpass.m_name);

				if (m_job_time_stat[i].find(subpass.m_name) == m_job_time_stat[i].end())
				{
					m_job_time_stat[i][subpass.m_name] = 0;
				}

				for (size_t h = 0; h < subpass.m_draw_calls.size(); h++)
				{
					m_job_time_stat[i][subpass.m_name] += subpass.m_draw_calls[h].m_query_results[NGL_STATISTIC_TIME_ELAPSED];
				}
			}
		}
	}

	// fill the empty fields with zero
	for (std::set<std::string>::const_iterator j_it = jobs.begin(); j_it != jobs.end(); ++j_it)
	{
		m_jobs.push_back(*j_it);
		for (uint32_t i = 0; i < m_frames_stats.size(); i++)
		{
			if (m_job_time_stat[i].find(*j_it) == m_job_time_stat[i].end())
			{
				m_job_time_stat[i][*j_it] = 0;
			}
		}
	}
}


void PerFrameRenderTimeStatistics::ExportStatistics(const char *filename)
{
	std::fstream csv(filename, std::ios::out);
	if (!csv)
	{
		NGLOG_ERROR("PerFrameRenderTimeStatistics: Can not create file: %s", filename);
		return;
	}

	csv.precision(3);

	// print header
	csv << "Frame time" << SEPARATOR;
	csv << "User data" << SEPARATOR;
	for (uint32_t i = 0; i < m_jobs.size(); ++i)
	{
		csv << ((m_jobs[i] != "") ? m_jobs[i] : std::string("UNNAMED_JOBS")) << SEPARATOR;
	}
	csv << std::endl;

	for (uint32_t i = 0; i < m_job_time_stat.size(); i++)
	{
		csv << m_frame_times[i] << SEPARATOR;
		csv << m_frames_stats[i].m_user_data << SEPARATOR;
		for (uint32_t j = 0; j < m_jobs.size(); j++)
		{
			csv << (double)m_job_time_stat[i].at(m_jobs[j]) / 1000000.0 << SEPARATOR;
		}
		csv << std::endl;
	}

	csv.close();
}


GLB::PerFrameRenderTimeStatistics::PerFrameRenderTimeStatistics()
{

}


GLB::PerFrameRenderTimeStatistics::~PerFrameRenderTimeStatistics()
{

}


void GLB::RenderJobStatistics::CalculateStatistics()
{
	m_job_time_stat.resize(m_frames_stats.size());
	m_jobs.clear();

	std::set<std::string> jobs;
	for (uint32_t i = 0; i < m_frames_stats.size(); i++)
	{
		const NGLStatistic &frame = m_frames_stats[i];

		for (std::vector<job_statistics>::const_iterator j_it = frame.jobs.begin(); j_it != frame.jobs.end(); ++j_it)
		{
			if (!j_it->m_is_active)
			{
				continue;
			}

			for (size_t j = 0; j < j_it->m_sub_pass.size(); ++j)
			{
				jobs.insert(j_it->m_sub_pass[j].m_name);

				if (j_it->m_dispatch_count > 0)
				{
					m_job_time_stat[i][j_it->m_sub_pass[j].m_name] += j_it->m_dispatch_elapsed_time;
				}
			}

			for (size_t j = 0; j < j_it->m_sub_pass.size(); ++j)
			{
				for (size_t h = 0; h < j_it->m_sub_pass[j].m_draw_calls.size(); ++h)
				{
					std::string pass_name = j_it->m_sub_pass[j].m_name;

					m_job_time_stat[i][pass_name] += j_it->m_sub_pass[j].m_draw_calls[h].m_query_results[NGL_STATISTIC_TIME_ELAPSED];
				}
			}
		}
	}

	// fill the empty fields with uint64_t::max
	for (std::set<std::string>::const_iterator j_it = jobs.begin(); j_it != jobs.end(); ++j_it)
	{
		m_jobs.push_back(*j_it);

		for (uint32_t i = 0; i < m_frames_stats.size(); i++)
		{
			if (m_job_time_stat[i].find(*j_it) == m_job_time_stat[i].end())
			{
				m_job_time_stat[i][*j_it] = UINT64_MAX;
			}
		}
	}
}


void GLB::RenderJobStatistics::ExportStatistics(const char *filename)
{
	std::fstream csv(filename, std::ios::out);
	if (!csv)
	{
		NGLOG_ERROR("PerFrameRenderTimeStatistics: Can not create file: %s", filename);
		return;
	}

	csv.precision(3);
	csv.setf(std::ios::fixed, std::ios::floatfield);

	std::unordered_set<std::string> group_names;
	std::unordered_set<std::string>::iterator group_names_it;

	std::unordered_map<std::string, double> maps;
	std::unordered_map<std::string, double>::iterator maps_it;

	//double sum_time = double();
	for (uint32_t i = 0; i < m_jobs.size(); ++i)
	{
		size_t pos = m_jobs[i].find("::");
		std::string s;
		if (pos != std::string::npos)
		{
			s = m_jobs[i].substr(0, pos);
			group_names.insert(s);
		}
		else
		{
			s = m_jobs[i];
			group_names.insert(s);
		}

		for (uint32_t j = 0; j < m_job_time_stat.size(); j++)
		{
			if (m_job_time_stat[j].at(m_jobs[i]) != UINT64_MAX)
			{
				maps[s] += (double)m_job_time_stat[j].at(m_jobs[i]) / 1000000.0;
				//sum_time += maps[s];
			}
		}
	}
	csv << SEPARATOR;
	for (group_names_it = group_names.begin(); group_names_it != group_names.end(); ++group_names_it)
	{
		csv << *group_names_it << SEPARATOR;
	}
	csv << std::endl;
	csv << "time / frames" << SEPARATOR;

	for (maps_it = maps.begin();
	maps_it != maps.end();
		++maps_it)
	{
		csv << (*maps_it).second / m_frames_stats.size() << SEPARATOR;
	}

	csv << std::endl;
	csv << "time" << SEPARATOR;

	for (maps_it = maps.begin();
	maps_it != maps.end();
		++maps_it)
	{
		csv << (*maps_it).second << SEPARATOR;
	}

	csv.close();
}
