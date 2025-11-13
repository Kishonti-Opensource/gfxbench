/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef RENDER_STATISTICS_H
#define RENDER_STATISTICS_H

#include "ngl.h"
#include "kcl_base.h"
#include "ng/timer.h"

#include <vector>
#include <set>


namespace GFXB
{
	class ShotHandler;
}

namespace GLB
{
class RenderStatistics
{
public:
	RenderStatistics(KCL::uint32 test_width, KCL::uint32 test_height);
	virtual ~RenderStatistics();

	void AddFrame(const NGLStatistic& frame);

	void CalculateStatistics();

	virtual void ExportStatistics(const char *filename);

protected:

	KCL::uint32 m_test_width;
	KCL::uint32 m_test_height;

	std::vector<NGLStatistic> m_frames_stats;

	class Value
	{
	public:
		Value();

		double Min() const;
		double Max() const;
		double Avg() const;

		void SetFrames(KCL::uint32 frame_count);

		void MinMaxAvg(double value);

		void Clear();
	private:
		bool m_inited;

		double m_min_value;
		double m_max_value;
		double m_avg_value;
	};


	Value m_draw_calls;
	Value m_tess_draw_calls;
	Value m_gs_draw_calls;

	Value m_texture_count;

	Value m_compute_dispatch_count;

	Value m_overdraw;

	Value m_query_values[NGL_STATISTIC_COUNT];

	void CalculateFrame(const NGLStatistic &frame);

	static void Write(std::fstream &stream, const char *key, const Value &value);
	static void Write(std::fstream &csv, double key, const Value &value);
};


class TimedRenderStatistics : public RenderStatistics
{
public:
	TimedRenderStatistics(uint32_t w, uint32_t h) : RenderStatistics(w, h)
	{
		m_save_timer.start();
		m_start_time = m_save_timer.elapsed().wall;
		m_global_time.stop();
	}

	void StartGlobalTimer()
	{
		if (m_global_time.is_stopped())
		{
			m_global_time.start();
		}
	}

	std::map<double, Value> partial_drawcalls;
	std::map<double, Value> partial_overdraw;
	std::map<double, uint32_t> shot_indexes;
	std::map<double, Value> partial_query_values[NGL_STATISTIC_COUNT];

	void CalculatePartialStatistics(double time, uint32_t shot_index);

	void ExportStatistics(const char *filename);

	void AddFrame(const NGLStatistic& frame, uint32_t anim_time, GFXB::ShotHandler *shot_index);
	void WriteHeader(std::fstream &stream, const char* title);
protected:
	ng::cpu_timer m_global_time;
	ng::cpu_timer m_save_timer;
	double m_start_time;
};


class PerFrameRenderTimeStatistics
{
public:

	PerFrameRenderTimeStatistics();
	virtual ~PerFrameRenderTimeStatistics();

	void AddFrame(const NGLStatistic& frame, uint32_t anim_time);
	virtual void CalculateStatistics();
	virtual void ExportStatistics(const char *filename);

protected:
	std::vector<NGLStatistic> m_frames_stats;
	std::vector<uint32_t> m_frame_times;
	std::vector<std::map<std::string,uint64_t>> m_job_time_stat;
	std::vector<std::string> m_jobs;
};


class RenderJobStatistics : public PerFrameRenderTimeStatistics
{
public:
	virtual void CalculateStatistics() override;
	virtual void ExportStatistics(const char *filename) override;
};


}//!namespace GLB

#endif //!RENDER_STATISTICS_H
