/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef RENDER_STATISTICS_H
#define RENDER_STATISTICS_H

#include <kcl_base.h>
#include <string>
#include <vector>

namespace StatisticsHelper
{
	class OverdrawFromSamplesPassed
	{
	public:
		OverdrawFromSamplesPassed(int w, int h) : divider(w*h > 4 ? w*h : -1.0) {}
		double operator()(double samplesPassed)
		{
			if (samplesPassed < 0.0 || divider < 0.0) return -1.0;
			return samplesPassed / divider;
		}
	private:
		double divider;
	};

	class MemBWFromSamplesPassed
	{
	public:
		double operator()(double samplesPassed)
		{
			const double divider = 2.0 * 1024.0 * 1024.0; // 2.0 because of assumed 4 bit/pixel, 1024*1024 because MiB
			return samplesPassed / divider;
		}
	} ;

}//namespace statistics

class TestDescriptor;


class StatisticsSample
{
public:
	StatisticsSample(double frame_time = 0.0, float avg_fps = 0.0f, KCL::uint32 vertex_count = 0, KCL::uint32 primitive_count = 0, KCL::uint32 draw_calls = 0, KCL::int32 texture_count = 0, KCL::int32 samples_passed = 0, float pixel_coverage = 0, KCL::int32 instruction_count = 0):
		m_frame_time(frame_time),
		m_avg_fps(avg_fps),
		m_vertex_count(vertex_count),
		m_primitive_count(primitive_count),
		m_draw_calls(draw_calls),
		m_texture_count(texture_count),
		m_samples_passed(samples_passed),
		m_pixel_coverage(pixel_coverage),
		m_instruction_count(instruction_count)
	{
	}


	void SetFrameTime (double frame_time) { m_frame_time = frame_time; }
	void SetAvgFps (float avg_fps) { m_avg_fps = avg_fps; }
	void SetVertexCount (KCL::uint32 vertex_count) { m_vertex_count = vertex_count; }
	void SetPrimitiveCount (KCL::uint32 primitive_count) { m_primitive_count = primitive_count; }
	void SetDrawCalls (KCL::uint32 draw_calls) { m_draw_calls = draw_calls; }
	void SetUsedTextureCount (KCL::int32 tc) { m_texture_count = tc; }
	void SetSamplesPassed (KCL::int32 sp) { m_samples_passed = sp; }
	void SetPixelCoverage (float pc) { m_pixel_coverage = pc; }
	void SetInstructionCount (KCL::int32 ic) { m_instruction_count = ic; }
	
	double GetFrameTime () const { return m_frame_time; }
	float GetAvgFps () const { return m_avg_fps; }
	KCL::uint32 GetVertexCount () const { return m_vertex_count; }
	KCL::uint32 GetPrimitiveCount () const { return m_primitive_count; }
	KCL::uint32 GetDrawCalls () const { return m_draw_calls; }
	KCL::int32 GetUsedTextureCount () const { return m_texture_count; }
	KCL::int32 GetSamplesPassed () const { return m_samples_passed; }
	float GetPixelCoverage () const { return m_pixel_coverage; }
	KCL::int32 GetInstructionCount	() const { return m_instruction_count; }


private:
	double m_frame_time; // the time taken rendering the frame
	float m_avg_fps; // avg fps per second
	KCL::uint32 m_vertex_count;
	KCL::uint32 m_primitive_count;
	KCL::uint32 m_draw_calls;
	KCL::int32 m_texture_count;     // If you see only -1 in the result, it means that the measurement is not enabled.
	KCL::int32 m_samples_passed;    // If you see only -1 in the result, it means that the measurement is not enabled. Occlusion query is needed.
	float m_pixel_coverage;
	KCL::int32 m_instruction_count; // If you see only -1 in the result, it means that the measurement is not enabled. Occlusion query is needed. Tool called cgc.exe needed, and its path has to be set.
};


struct StatisticsArray
{
	friend StatisticsArray* NewStatisticsArray(const TestDescriptor &settings);
	friend void DeleteStatisticsArray(StatisticsArray* &statisticsArray);

	const TestDescriptor *m_settings;
	std::vector<StatisticsSample> m_data;

	void saveInTextFile(const std::string &path) const;
	void loadFromTextFile(const std::string &path);
private:
	StatisticsArray(const TestDescriptor &settings) : m_settings(&settings)
	{
	}
};


StatisticsArray* NewStatisticsArray(const TestDescriptor &settings);
void DeleteStatisticsArray(StatisticsArray* &statisticsArray);


#endif //RENDER_STATISTICS_H
