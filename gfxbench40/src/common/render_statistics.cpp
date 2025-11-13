/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "kcl_os.h"
#include "kcl_io.h"

#include "render_statistics.h"
#include "test_descriptor.h"

#include <cmath>
#include <float.h>
#include <sstream>
#include <fstream>
#include <cassert>

using namespace StatisticsHelper;

void saveMinMaxAvg(std::stringstream &outFile, const StatisticsArray &statisticsArray, int w, int h)
{
	if (statisticsArray.m_data.size() < 2)
	{
		return;
	}
	OverdrawFromSamplesPassed overdrawFromSamplesPassed(w, h);
	MemBWFromSamplesPassed memBWFromSamplesPassed;

	double      min_frame_time = statisticsArray.m_data[0].GetFrameTime();
	float       min_avg_fps = statisticsArray.m_data[0].GetAvgFps();
	KCL::uint32 min_vertex_count = statisticsArray.m_data[0].GetVertexCount();
	KCL::uint32 min_primitive_count = statisticsArray.m_data[0].GetPrimitiveCount();
	KCL::uint32 min_draw_calls = statisticsArray.m_data[0].GetDrawCalls();
	KCL::int32	min_texture_count = statisticsArray.m_data[0].GetUsedTextureCount();
	KCL::int32	min_samples_passed = statisticsArray.m_data[0].GetSamplesPassed();
	float   	min_pixel_coverage = statisticsArray.m_data[0].GetPixelCoverage();
	double      min_overdraw_rate = overdrawFromSamplesPassed(statisticsArray.m_data[0].GetSamplesPassed());
	double      min_memory_bandwidth = memBWFromSamplesPassed(statisticsArray.m_data[0].GetSamplesPassed());
	KCL::int32	min_instruction_count = statisticsArray.m_data[0].GetInstructionCount();

	KCL::int32 min_frame_time_at = 0;
	KCL::int32 min_avg_fps_at = 0;
	KCL::int32 min_vertex_count_at = 0;
	KCL::int32 min_primitive_count_at = 0;
	KCL::int32 min_draw_calls_at = 0;
	KCL::int32 min_texture_count_at = 0;
	KCL::int32 min_samples_passed_at = 0;
	KCL::int32 min_pixel_coverage_at = 0;
	KCL::int32 min_overdraw_rate_at = 0;
	KCL::int32 min_memory_bandwidth_at = 0;
	KCL::int32 min_instruction_count_at = 0;


	double      max_frame_time = statisticsArray.m_data[0].GetFrameTime();
	float       max_avg_fps = statisticsArray.m_data[0].GetAvgFps();
	KCL::uint32 max_vertex_count = statisticsArray.m_data[0].GetVertexCount();
	KCL::uint32 max_primitive_count = statisticsArray.m_data[0].GetPrimitiveCount();
	KCL::uint32 max_draw_calls = statisticsArray.m_data[0].GetDrawCalls();
	KCL::int32	max_texture_count = statisticsArray.m_data[0].GetUsedTextureCount();
	KCL::int32	max_samples_passed = statisticsArray.m_data[0].GetSamplesPassed();
	float   	max_pixel_coverage = statisticsArray.m_data[0].GetPixelCoverage();
	double      max_overdraw_rate = overdrawFromSamplesPassed(statisticsArray.m_data[0].GetSamplesPassed());
	double      max_memory_bandwidth = memBWFromSamplesPassed(statisticsArray.m_data[0].GetSamplesPassed());
	KCL::int32	max_instruction_count = statisticsArray.m_data[0].GetInstructionCount();

	KCL::int32 max_frame_time_at = 0;
	KCL::int32 max_avg_fps_at = 0;
	KCL::int32 max_vertex_count_at = 0;
	KCL::int32 max_primitive_count_at = 0;
	KCL::int32 max_draw_calls_at = 0;
	KCL::int32 max_texture_count_at = 0;
	KCL::int32 max_samples_passed_at = 0;
	KCL::int32 max_pixel_coverage_at = 0;
	KCL::int32 max_overdraw_rate_at = 0;
	KCL::int32 max_memory_bandwidth_at = 0;
	KCL::int32 max_instruction_count_at = 0;


	double avg_frame_time = 0;
	int avg_frame_time_1st_not_null_idx = -1;
	size_t avg_frame_time_divider = 0;

	double avg_avg_fps = 0;
	int avg_avg_fps_1st_not_null_idx = -1;
	size_t avg_avg_fps_divider = 0;

	double avg_vertex_count = ((double)(statisticsArray.m_data[0].GetVertexCount())) / ((double)(statisticsArray.m_data.size()));
	double avg_primitive_count = ((double)(statisticsArray.m_data[0].GetPrimitiveCount())) / ((double)(statisticsArray.m_data.size()));
	double avg_draw_calls = ((double)(statisticsArray.m_data[0].GetDrawCalls())) / ((double)(statisticsArray.m_data.size()));

	double avg_texture_count = statisticsArray.m_data[0].GetUsedTextureCount() >= 0.0 ? ((double)(statisticsArray.m_data[0].GetUsedTextureCount())) / ((double)(statisticsArray.m_data.size())) : -1.0;
	double avg_samples_passed = statisticsArray.m_data[0].GetSamplesPassed() >= 0.0 ? ((double)(statisticsArray.m_data[0].GetSamplesPassed())) / ((double)(statisticsArray.m_data.size())) : -1.0;
	double avg_pixel_coverage = statisticsArray.m_data[0].GetPixelCoverage() >= 0.0 ? ((double)(statisticsArray.m_data[0].GetPixelCoverage())) / ((double)(statisticsArray.m_data.size())) : -1.0;
	double avg_overdraw_rate = overdrawFromSamplesPassed(statisticsArray.m_data[0].GetSamplesPassed()) >= 0.0 ? overdrawFromSamplesPassed(statisticsArray.m_data[0].GetSamplesPassed()) / ((double)(statisticsArray.m_data.size())) : -1.0;
	double avg_memory_bandwidth = memBWFromSamplesPassed(statisticsArray.m_data[0].GetSamplesPassed()) >= 0.0 ? memBWFromSamplesPassed(statisticsArray.m_data[0].GetSamplesPassed()) / ((double)(statisticsArray.m_data.size())) : -1.0;
	double avg_instruction_count = statisticsArray.m_data[0].GetInstructionCount() >= 0.0 ? ((double)(statisticsArray.m_data[0].GetInstructionCount())) / ((double)(statisticsArray.m_data.size())) : -1.0;

	KCL::int32 avg_frame_time_at = -1;
	KCL::int32 avg_avg_fps_at = -1;
	KCL::int32 avg_vertex_count_at = -1;
	KCL::int32 avg_primitive_count_at = -1;
	KCL::int32 avg_draw_calls_at = -1;
	KCL::int32 avg_texture_count_at = -1;
	KCL::int32 avg_samples_passed_at = -1;
	KCL::int32 avg_pixel_coverage_at = -1;
	KCL::int32 avg_overdraw_rate_at = -1;
	KCL::int32 avg_memory_bandwidth_at = -1;
	KCL::int32 avg_instruction_count_at = -1;

	double avg_frame_time_at_distance = -1.0;
	double avg_avg_fps_at_distance = -1.0;
	double avg_vertex_count_at_distance = -1.0;
	double avg_primitive_count_at_distance = -1.0;
	double avg_draw_calls_at_distance = -1.0;
	double avg_texture_count_at_distance = -1.0;
	double avg_samples_passed_at_distance = -1.0;
	double avg_pixel_coverage_at_distance = -1.0;
	double avg_overdraw_rate_at_distance = -1.0;
	double avg_memory_bandwidth_at_distance = -1.0;
	double avg_instruction_count_at_distance = -1.0;


	for (size_t i = 1; i<statisticsArray.m_data.size(); ++i)
	{
		//frame_time
		if (min_frame_time <= 0.0 || min_frame_time > statisticsArray.m_data[i].GetFrameTime())
		{
			min_frame_time = statisticsArray.m_data[i].GetFrameTime();
			min_frame_time_at = i + 1;
		}
		if (max_frame_time < statisticsArray.m_data[i].GetFrameTime())
		{
			max_frame_time = statisticsArray.m_data[i].GetFrameTime();
			max_frame_time_at = i + 1;
		}
		if (avg_frame_time_1st_not_null_idx < 0 && statisticsArray.m_data[i].GetFrameTime() > 0.0)
		{
			avg_frame_time_1st_not_null_idx = i;
			avg_frame_time_divider = statisticsArray.m_data.size() - (size_t)avg_frame_time_1st_not_null_idx - 1;
		}
		if (avg_frame_time_divider)
		{
			avg_frame_time += statisticsArray.m_data[i].GetFrameTime() / (double)avg_frame_time_divider;
		}

		//avg_fps
		if (min_avg_fps <= 0.0 || min_avg_fps > statisticsArray.m_data[i].GetAvgFps())
		{
			min_avg_fps = statisticsArray.m_data[i].GetAvgFps();
			min_avg_fps_at = i + 1;
		}
		if (max_avg_fps < statisticsArray.m_data[i].GetAvgFps())
		{
			max_avg_fps = statisticsArray.m_data[i].GetAvgFps();
			max_avg_fps_at = i + 1;
		}
		if (avg_avg_fps_1st_not_null_idx < 0 && statisticsArray.m_data[i].GetAvgFps() > 0.0)
		{
			avg_avg_fps_1st_not_null_idx = i;
			avg_avg_fps_divider = statisticsArray.m_data.size() - (size_t)avg_avg_fps_1st_not_null_idx - 1;
		}
		if (avg_avg_fps_divider)
		{
			avg_avg_fps += statisticsArray.m_data[i].GetAvgFps() / (double)avg_avg_fps_divider;
		}

		//vertex_count
		if (min_vertex_count > statisticsArray.m_data[i].GetVertexCount())
		{
			min_vertex_count = statisticsArray.m_data[i].GetVertexCount();
			min_vertex_count_at = i + 1;
		}
		if (max_vertex_count < statisticsArray.m_data[i].GetVertexCount())
		{
			max_vertex_count = statisticsArray.m_data[i].GetVertexCount();
			max_vertex_count_at = i + 1;
		}
		avg_vertex_count += ((double)statisticsArray.m_data[i].GetVertexCount()) / ((double)statisticsArray.m_data.size());

		//primitive_count
		if (min_primitive_count > statisticsArray.m_data[i].GetPrimitiveCount())
		{
			min_primitive_count = statisticsArray.m_data[i].GetPrimitiveCount();
			min_primitive_count_at = i + 1;
		}
		if (max_primitive_count < statisticsArray.m_data[i].GetPrimitiveCount())
		{
			max_primitive_count = statisticsArray.m_data[i].GetPrimitiveCount();
			max_primitive_count_at = i + 1;
		}
		avg_primitive_count += ((double)statisticsArray.m_data[i].GetPrimitiveCount()) / ((double)statisticsArray.m_data.size());


		//draw_calls
		if (min_draw_calls > statisticsArray.m_data[i].GetDrawCalls())
		{
			min_draw_calls = statisticsArray.m_data[i].GetDrawCalls();
			min_draw_calls_at = i + 1;
		}
		if (max_draw_calls < statisticsArray.m_data[i].GetDrawCalls())
		{
			max_draw_calls = statisticsArray.m_data[i].GetDrawCalls();
			max_draw_calls_at = i + 1;
		}
		avg_draw_calls += ((double)statisticsArray.m_data[i].GetDrawCalls()) / ((double)statisticsArray.m_data.size());


		//texture_count
		if (min_texture_count > statisticsArray.m_data[i].GetUsedTextureCount())
		{
			min_texture_count = statisticsArray.m_data[i].GetUsedTextureCount();
			min_texture_count_at = i + 1;
		}
		if (max_texture_count < statisticsArray.m_data[i].GetUsedTextureCount())
		{
			max_texture_count = statisticsArray.m_data[i].GetUsedTextureCount();
			max_texture_count_at = i + 1;
		}
		if (statisticsArray.m_data[i].GetUsedTextureCount() >= 0.0)
			avg_texture_count += ((double)statisticsArray.m_data[i].GetUsedTextureCount()) / ((double)statisticsArray.m_data.size());


		//samples_passed
		if (min_samples_passed > statisticsArray.m_data[i].GetSamplesPassed())
		{
			min_samples_passed = statisticsArray.m_data[i].GetSamplesPassed();
			min_samples_passed_at = i + 1;
		}
		if (max_samples_passed < statisticsArray.m_data[i].GetSamplesPassed())
		{
			max_samples_passed = statisticsArray.m_data[i].GetSamplesPassed();
			max_samples_passed_at = i + 1;
		}
		if (statisticsArray.m_data[i].GetSamplesPassed() >= 0.0)
			avg_samples_passed += ((double)statisticsArray.m_data[i].GetSamplesPassed()) / ((double)statisticsArray.m_data.size());

		//pixel coverage
		if (min_pixel_coverage > statisticsArray.m_data[i].GetPixelCoverage())
		{
			min_pixel_coverage = statisticsArray.m_data[i].GetPixelCoverage();
			min_pixel_coverage_at = i + 1;
		}
		if (max_pixel_coverage < statisticsArray.m_data[i].GetPixelCoverage())
		{
			max_pixel_coverage = statisticsArray.m_data[i].GetPixelCoverage();
			max_pixel_coverage_at = i + 1;
		}
		if (statisticsArray.m_data[i].GetPixelCoverage() >= 0.0)
			avg_pixel_coverage += ((double)statisticsArray.m_data[i].GetPixelCoverage()) / ((double)statisticsArray.m_data.size());


		//overdraw_rate
		if (min_overdraw_rate > overdrawFromSamplesPassed(statisticsArray.m_data[i].GetSamplesPassed()))
		{
			min_overdraw_rate = overdrawFromSamplesPassed(statisticsArray.m_data[i].GetSamplesPassed());
			min_overdraw_rate_at = i + 1;
		}
		if (max_overdraw_rate < overdrawFromSamplesPassed(statisticsArray.m_data[i].GetSamplesPassed()))
		{
			max_overdraw_rate = overdrawFromSamplesPassed(statisticsArray.m_data[i].GetSamplesPassed());
			max_overdraw_rate_at = i + 1;
		}
		if (statisticsArray.m_data[i].GetSamplesPassed() >= 0.0)
			avg_overdraw_rate += ((double)overdrawFromSamplesPassed(statisticsArray.m_data[i].GetSamplesPassed())) / ((double)statisticsArray.m_data.size());


		//memory_bandwidth
		if (min_memory_bandwidth > memBWFromSamplesPassed(statisticsArray.m_data[i].GetSamplesPassed()))
		{
			min_memory_bandwidth = memBWFromSamplesPassed(statisticsArray.m_data[i].GetSamplesPassed());
			min_memory_bandwidth_at = i + 1;
		}
		if (max_memory_bandwidth < memBWFromSamplesPassed(statisticsArray.m_data[i].GetSamplesPassed()))
		{
			max_memory_bandwidth = memBWFromSamplesPassed(statisticsArray.m_data[i].GetSamplesPassed());
			max_memory_bandwidth_at = i + 1;
		}
		if (statisticsArray.m_data[i].GetSamplesPassed() >= 0.0)
			avg_memory_bandwidth += ((double)memBWFromSamplesPassed(statisticsArray.m_data[i].GetSamplesPassed())) / ((double)statisticsArray.m_data.size());


		//instruction_count
		if (min_instruction_count > statisticsArray.m_data[i].GetInstructionCount())
		{
			min_instruction_count = statisticsArray.m_data[i].GetInstructionCount();
			min_instruction_count_at = i + 1;
		}
		if (max_instruction_count < statisticsArray.m_data[i].GetInstructionCount())
		{
			max_instruction_count = statisticsArray.m_data[i].GetInstructionCount();
			max_instruction_count_at = i + 1;
		}
		if (statisticsArray.m_data[i].GetInstructionCount() >= 0.0)
			avg_instruction_count += ((double)statisticsArray.m_data[i].GetInstructionCount()) / ((double)statisticsArray.m_data.size());
	}


	//find closest to avg
	double delta = 0.0;
	for (size_t i = 0; i < statisticsArray.m_data.size(); ++i)
	{
		//frame_time		 avg_frame_time_at_distance = -1.0;		    avg_frame_time_at = -1;
		delta = fabs(avg_frame_time - statisticsArray.m_data[i].GetFrameTime());
		if (0.0 < statisticsArray.m_data[i].GetFrameTime() && (avg_frame_time_at_distance > delta || avg_frame_time_at_distance < 0.0))
		{
			avg_frame_time_at_distance = delta;
			avg_frame_time_at = i;
		}

		//avg_fps		     avg_avg_fps_at_distance = -1.0;		    avg_avg_fps_at = -1;
		delta = fabs(avg_avg_fps - statisticsArray.m_data[i].GetAvgFps());
		if (0.0 < statisticsArray.m_data[i].GetAvgFps() && (avg_avg_fps_at_distance > delta || avg_avg_fps_at_distance < 0.0))
		{
			avg_avg_fps_at_distance = delta;
			avg_avg_fps_at = i;
		}


		//vertex_count		 avg_vertex_count_at_distance = -1.0;	    avg_vertex_count_at = -1;
		delta = fabs(avg_vertex_count - statisticsArray.m_data[i].GetVertexCount());
		if (avg_vertex_count_at_distance > delta || avg_vertex_count_at_distance < 0.0)
		{
			avg_vertex_count_at_distance = delta;
			avg_vertex_count_at = i;
		}


		//primitive_count	 avg_primitive_count_at_distance = -1.0;	avg_primitive_count_at = -1;
		delta = fabs(avg_primitive_count - statisticsArray.m_data[i].GetPrimitiveCount());
		if (avg_primitive_count_at_distance > delta || avg_primitive_count_at_distance < 0.0)
		{
			avg_primitive_count_at_distance = delta;
			avg_primitive_count_at = i;
		}


		//draw_calls		 avg_draw_calls_at_distance = -1.0;		    avg_draw_calls_at = -1;
		delta = fabs(avg_draw_calls - statisticsArray.m_data[i].GetDrawCalls());
		if (avg_draw_calls_at_distance > delta || avg_draw_calls_at_distance < 0.0)
		{
			avg_draw_calls_at_distance = delta;
			avg_draw_calls_at = i;
		}


		//texture_count		 avg_texture_count_at_distance = -1.0;      avg_texture_count_at = -1;
		if (avg_texture_count >= 0.0)
		{
			delta = fabs(avg_texture_count - statisticsArray.m_data[i].GetUsedTextureCount());
			if (avg_texture_count_at_distance > delta || avg_texture_count_at_distance < 0.0)
			{
				avg_texture_count_at_distance = delta;
				avg_texture_count_at = i;
			}
		}


		//samples_passed	 avg_samples_passed_at_distance = -1.0;     avg_samples_passed_at = -1;
		if (avg_samples_passed >= 0.0)
		{
			delta = fabs(avg_samples_passed - statisticsArray.m_data[i].GetSamplesPassed());
			if (avg_samples_passed_at_distance > delta || avg_samples_passed_at_distance < 0.0)
			{
				avg_samples_passed_at_distance = delta;
				avg_samples_passed_at = i;
			}


			//overdraw_rate		 avg_overdraw_rate_at_distance = -1.0;	    avg_overdraw_rate_at = -1;
			delta = fabs(avg_overdraw_rate - overdrawFromSamplesPassed(statisticsArray.m_data[i].GetSamplesPassed()));
			if (avg_overdraw_rate_at_distance > delta || avg_overdraw_rate_at_distance < 0.0)
			{
				avg_overdraw_rate_at_distance = delta;
				avg_overdraw_rate_at = i;
			}


			//memory_bandwidth	 avg_memory_bandwidth_at_distance = -1.0; 	avg_memory_bandwidth_at = -1;
			delta = fabs(avg_memory_bandwidth - memBWFromSamplesPassed(statisticsArray.m_data[i].GetSamplesPassed()));
			if (avg_memory_bandwidth_at_distance > delta || avg_memory_bandwidth_at_distance < 0.0)
			{
				avg_memory_bandwidth_at_distance = delta;
				avg_memory_bandwidth_at = i;
			}
		}

		//instruction_count  avg_instruction_count_at_distance = -1.0;  avg_instruction_count_at = -1;
		if (avg_instruction_count >= 0.0)
		{
			delta = fabs(avg_instruction_count - statisticsArray.m_data[i].GetInstructionCount());
			if (avg_instruction_count_at_distance > delta || avg_instruction_count_at_distance < 0.0)
			{
				avg_instruction_count_at_distance = delta;
				avg_instruction_count_at = i;
			}
		}

		//pixel_coverage	avg_pixel_coverage_at_distance = -1.0;     avg_pixel_coverage_at = -1;
		if (avg_pixel_coverage >= 0.0)
		{
			delta = fabs(avg_pixel_coverage - statisticsArray.m_data[i].GetPixelCoverage());
			if (avg_pixel_coverage_at_distance > delta || avg_pixel_coverage_at_distance < 0.0)
			{
				avg_pixel_coverage_at_distance = delta;
				avg_pixel_coverage_at = i;
			}
		}
	}

	std::ios_base::fmtflags default_fmtflags = outFile.setf(std::ios_base::showpoint | std::ios_base::fixed, std::ios_base::floatfield);
	std::streamsize default_precision = outFile.precision(5);

	outFile << "Min frame time (ms) : " << min_frame_time << " at frame: " << min_frame_time_at << std::endl;
	outFile << "Max frame time (ms) : " << max_frame_time << " at frame: " << max_frame_time_at << std::endl;
	outFile << "Avg frame time (ms) : " << avg_frame_time << " closest frame to avg: " << avg_frame_time_at << std::endl << std::endl;

	outFile << "Min avg fps : " << min_avg_fps << " at frame: " << min_avg_fps_at << std::endl;
	outFile << "Max avg fps : " << max_avg_fps << " at frame: " << max_avg_fps_at << std::endl;
	outFile << "Avg avg fps : " << avg_avg_fps << " closest frame to avg: " << avg_avg_fps_at << std::endl << std::endl;

	outFile << "Min vertex count : " << min_vertex_count << " at frame: " << min_vertex_count_at << std::endl;
	outFile << "Max vertex count : " << max_vertex_count << " at frame: " << max_vertex_count_at << std::endl;
	outFile << "Avg vertex count : " << avg_vertex_count << " closest frame to avg: " << avg_vertex_count_at << std::endl << std::endl;

	outFile << "Min primitive count : " << min_primitive_count << " at frame: " << min_primitive_count_at << std::endl;
	outFile << "Max primitive count : " << max_primitive_count << " at frame: " << max_primitive_count_at << std::endl;
	outFile << "Avg primitive count : " << avg_primitive_count << " closest frame to avg: " << avg_primitive_count_at << std::endl << std::endl;

	outFile << "Min pixel coverage : " << min_pixel_coverage << " at frame: " << min_pixel_coverage_at << std::endl;
	outFile << "Max pixel coverage : " << max_pixel_coverage << " at frame: " << max_pixel_coverage_at << std::endl;
	outFile << "Avg pixel coverage : " << avg_pixel_coverage << " closest frame to avg: " << avg_pixel_coverage_at << std::endl << std::endl;

	outFile << "Min draw calls : " << min_draw_calls << " at frame: " << min_draw_calls_at << std::endl;
	outFile << "Max draw calls : " << max_draw_calls << " at frame: " << max_draw_calls_at << std::endl;
	outFile << "Avg draw calls : " << avg_draw_calls << " closest frame to avg: " << avg_draw_calls_at << std::endl << std::endl;

	outFile << "Min texture count : " << min_texture_count << " at frame: " << min_texture_count_at << std::endl;
	outFile << "Max texture count : " << max_texture_count << " at frame: " << max_texture_count_at << std::endl;
	outFile << "Avg texture count : " << avg_texture_count << " closest frame to avg: " << avg_texture_count_at << std::endl << std::endl;

	outFile << "Min samples passed : " << min_samples_passed << " at frame: " << min_samples_passed_at << std::endl;
	outFile << "Max samples passed : " << max_samples_passed << " at frame: " << max_samples_passed_at << std::endl;
	outFile << "Avg samples passed : " << avg_samples_passed << " closest frame to avg: " << avg_samples_passed_at << std::endl << std::endl;

	outFile << "Min overdraw rate : " << min_overdraw_rate << " at frame: " << min_overdraw_rate_at << std::endl;
	outFile << "Max overdraw rate : " << max_overdraw_rate << " at frame: " << max_overdraw_rate_at << std::endl;
	outFile << "Avg overdraw rate : " << avg_overdraw_rate << " closest frame to avg: " << avg_overdraw_rate_at << std::endl << std::endl;

	outFile << "Min texture read memory bandwidth (MiB - assume 4 bit/pixel textures) : " << min_memory_bandwidth << " at frame: " << min_memory_bandwidth_at << std::endl;
	outFile << "Max texture read memory bandwidth (MiB - assume 4 bit/pixel textures) : " << max_memory_bandwidth << " at frame: " << max_memory_bandwidth_at << std::endl;
	outFile << "Avg texture read memory bandwidth (MiB - assume 4 bit/pixel textures) : " << avg_memory_bandwidth << " closest frame to avg: " << avg_memory_bandwidth_at << std::endl << std::endl;

	outFile << "Min instruction count : " << min_instruction_count << " at frame: " << min_instruction_count_at << std::endl;
	outFile << "Max instruction count : " << max_instruction_count << " at frame: " << max_instruction_count_at << std::endl;
	outFile << "Avg instruction count : " << avg_instruction_count << " closest frame to avg: " << avg_instruction_count_at << std::endl << std::endl;

	outFile.setf(default_fmtflags, std::ios_base::floatfield);
	outFile.precision(default_precision);
}


void StatisticsArray::saveInTextFile(const std::string &path) const
{
	//TODO: save m_settings too
	KCL::File file(path, KCL::Write, KCL::RWDir);

	if (file.GetLastError())
	{
		INFO("error at saveInTextFile");
		return;
	}

	std::stringstream ss;
	ss << m_settings->m_name << " "
		<< m_settings->GetTextureType() << " ";

	int w = m_settings->m_viewport_width;
	int h = m_settings->m_viewport_height;
	std::string tail = "";

	assert(w != -1);
	assert(h != -1);

	if (SMode_Onscreen != m_settings->GetScreenMode())
	{
		if (SMode_Offscreen == m_settings->GetScreenMode())
			tail = " offscreen";
		else
			tail = " hybrid";
	}

	ss << w << "x" << h << tail;

	if (m_settings->m_frame_step_time > 0)
	{
		ss << " fixed frame step time (ms): " << m_settings->m_frame_step_time;
	}


	ss << std::endl << std::endl;

	saveMinMaxAvg(ss, *this, w, h);

	ss.fill(' ');
	ss.flags(std::ios::right);
	const std::streamsize columnWidth = 20;

	ss.width(columnWidth);
	ss << "Frame";


	ss.width(columnWidth);
	ss << "Frame time";

	ss.width(columnWidth);
	ss << "Avg FPS";

	ss.width(columnWidth);
	ss << "Vertex count";

	ss.width(columnWidth);
	ss << "Primitive count";

	ss.width(columnWidth);
	ss << "Pixel Coverage";

	ss.width(columnWidth);
	ss << "Draw calls";

	ss.width(columnWidth);
	ss << "Texture count";

	ss.width(columnWidth);
	ss << "Samples passed";

	ss.width(columnWidth);
	ss << "Overdraw rate";

	ss.width(columnWidth * 2);
	ss << "Texture memory BW in MiB";

	ss.width(columnWidth * 2);
	ss << "Shader instruction count";

	std::ios_base::fmtflags default_fmtflags = ss.setf(std::ios_base::showpoint | std::ios_base::fixed, std::ios_base::floatfield);
	std::streamsize default_precision = ss.precision(5);

	for (size_t i = 0; i < m_data.size(); ++i)
	{
		ss << std::endl;

		ss.width(columnWidth);
		ss << i;


		ss.width(columnWidth);
		ss << m_data[i].GetFrameTime();

		ss.width(columnWidth);
		ss << m_data[i].GetAvgFps();

		ss.width(columnWidth);
		ss << m_data[i].GetVertexCount();

		ss.width(columnWidth);
		ss << m_data[i].GetPrimitiveCount();

		ss.width(columnWidth);
		ss << m_data[i].GetPixelCoverage();

		ss.width(columnWidth);
		ss << m_data[i].GetDrawCalls();

		ss.width(columnWidth);
		ss << m_data[i].GetUsedTextureCount();

		ss.width(columnWidth);
		ss << m_data[i].GetSamplesPassed();

		double overdraw = ((double)m_data[i].GetSamplesPassed()) / ((double)(w*h));
		ss.width(columnWidth);
		ss << overdraw;

		double memBW = ((double)m_data[i].GetSamplesPassed()) / 2.0 / 1024.0 / 1024.0; // assume 4 bit/pixel textures
		ss.width(columnWidth * 2);
		ss << memBW;

		ss.width(columnWidth * 2);
		ss << m_data[i].GetInstructionCount();
	}

	ss.setf(default_fmtflags, std::ios_base::floatfield);
	ss.precision(default_precision);

	file.Printf("%s", ss.str().c_str());
	file.Close();
}


void StatisticsArray::loadFromTextFile(const std::string &path)
{
	//TODO: implement
	INFO("Error in StatisticsArray::loadFromTextFile: Not yet implemented.");
}


StatisticsArray* NewStatisticsArray(const TestDescriptor &settings)
{
	return new StatisticsArray(settings);
}


void DeleteStatisticsArray(StatisticsArray* &statisticsArray)
{
	delete statisticsArray;
	statisticsArray = 0;
}
