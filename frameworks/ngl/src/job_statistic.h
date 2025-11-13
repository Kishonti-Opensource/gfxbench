/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#pragma once
#include <cstdint>
#include <map>
#include <vector>


enum NGL_Statistic_Queries
{
	NGL_STATISTIC_PRIMITIVES_SUBMITED = 0,
	NGL_STATISTIC_VERTICES_SUBMITED,
	NGL_STATISTIC_VS_INVOCATIONS,
	NGL_STATISTIC_TCS_PATCHES,
	NGL_STATISTIC_TES_INVOCATIONS,
	NGL_STATISTIC_GS_INVOCATIONS,
	NGL_STATISTIC_GS_PRIMITIVES_EMITTED,
	NGL_STATISTIC_FS_INVOCATIONS,
	NGL_STATISTIC_CLIPPING_INPUT_PRIMITIVES,
	NGL_STATISTIC_CLIPPING_OUTPUT_PRIMITIVES,
	NGL_STATISTIC_PRIMITIVES_GENERATED,
	NGL_STATISTIC_SAMPLES_PASSED,
	NGL_STATISTIC_TIME_ELAPSED,

	NGL_STATISTIC_COUNT
};


struct NGLMemoryItem
{
	std::string m_name;
	uint64_t m_size;

	NGLMemoryItem()
	{
		m_size = 0;
	}

	NGLMemoryItem(const std::string &name, uint64_t size)
	{
		m_name = name;
		m_size = size;
	}
};


struct NGLMemoryStatistics
{
	std::vector<NGLMemoryItem> m_render_targets;
	std::vector<NGLMemoryItem> m_textures;
	std::vector<NGLMemoryItem> m_images;

	std::vector<NGLMemoryItem> m_vertex_buffers;
	std::vector<NGLMemoryItem> m_index_buffers;
	std::vector<NGLMemoryItem> m_storage_buffers;
};


struct draw_call_statistics
{
	//public
	uint64_t m_query_results[NGL_STATISTIC_COUNT];

	//internal
	uint32_t m_query_objects[NGL_STATISTIC_COUNT];

	//internal
	uint32_t m_query_targets[NGL_STATISTIC_COUNT];

	draw_call_statistics()
	{
		memset(m_query_results, 0, sizeof(m_query_results));
		memset(m_query_objects, 0, sizeof(m_query_objects));
		memset(m_query_targets, 0, sizeof(m_query_targets));
	}
};


struct job_statistics
{
	struct sub_pass_stats
	{
		std::string m_name;
		std::vector<draw_call_statistics> m_draw_calls;
	};
	std::vector<sub_pass_stats> m_sub_pass;
	size_t m_dispatch_count;
	uint32_t m_timer_query;

	// Dispatch time of a job stored in nanosec
	uint64_t m_dispatch_elapsed_time;

	bool m_is_active;

	job_statistics()
	{
		Clear();
	}

	void Clear()
	{
		m_dispatch_count = 0;
		m_timer_query = 0;
		m_dispatch_elapsed_time = 0;
		m_is_active = false;
		for (size_t i = 0; i < m_sub_pass.size();i++)
		{
			m_sub_pass[i].m_draw_calls.clear();
			m_sub_pass[i].m_draw_calls.reserve(128);
		}
	}
};


/*
Hold a per frame statistic
*/
struct NGLStatistic
{
	NGLStatistic()
	{
		m_user_data = 0;
		memset(queries_enabled, 0, sizeof(queries_enabled));
	}

	bool IsEnabled() const
	{
		bool ret_val = false;
		for (int i = 0; i < NGL_STATISTIC_COUNT; ++i)
		{
			if(queries_enabled[i] == true)
			{
				ret_val = true;
				break;
			}
		}
		return ret_val;
	}

	uint32_t m_user_data;
	std::map<uint32_t, std::string> m_used_textures;

	NGLMemoryStatistics m_memory_statistics;

	std::vector<job_statistics> jobs;
	bool queries_enabled[NGL_STATISTIC_COUNT];
};
