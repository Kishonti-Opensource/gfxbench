/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GL_WRAPPER_H
#define GL_WRAPPER_H

#include <map>
#include <set>
#include <vector>
#include "kcl_base.h"

#define GL_WRAPPER_ASSERT_ON_ERROR 1
#define ENABLE_GL_ERROR_CHECKING 0


struct MeasureFlags
{
	enum Enum
	{
		DrawCalls = 1,
		TextureCount = 2,
		Samples = 4,
		VertexBuffer = 8,
        PipeStats = 16,
	};
};

struct DrawCallStatistics
{
	int program;

	unsigned int num_active_textures;
	unsigned int num_draw_triangles;
	unsigned int num_draw_vertices;

    GLenum mode; //POINTS, TRIS, etc.
    bool uses_TES;
    bool uses_GS;

	std::set<KCL::uint32> texture_ids;

	unsigned int num_samples_passed;
	unsigned int generated_primitives;

    static const unsigned int query_stats_count = 11;
    unsigned int num_query_stats[query_stats_count];

	DrawCallStatistics()
	{
		program = 0;
		num_active_textures = 0;
		num_draw_triangles = 0;
		num_draw_vertices = 0;
		num_samples_passed = 0;
		generated_primitives = 0;

        mode = 0;

        uses_TES = false;
        uses_GS = false;

        for(int i=0; i<query_stats_count; ++i)
            num_query_stats[i] = 0;
	}
};


typedef std::map<int, int>::const_iterator buffer_it;
class MeasureResults
{
public:
	MeasureResults()
	{
		m_dispatch_count = 0;
		draw_calls.clear();
		m_bufferObjects.clear();
		measure_flags = 0;
		m_frame_time = 0;
	}

	KCL::uint32 m_dispatch_count;
	std::map<int, int> m_bufferObjects; //buffer id and buffer size
	std::vector<DrawCallStatistics> draw_calls;
	unsigned int measure_flags;
	KCL::uint32 m_frame_time;
};


typedef std::map<KCL::uint32, MeasureResults>::iterator session_it;
class GLWrapper
{
public:
	virtual const KCL::uint32 BeginMeasure(const unsigned int flags) = 0;
	virtual MeasureResults EndMeasure(const KCL::uint32 id) = 0;
    virtual void PauseMeasurement() = 0;
    virtual void ResumeMeasurement() = 0;
    virtual bool IsMeasurementPaused() = 0;

    virtual void BeginMeasureQuick() = 0;
    virtual void EndMeasureQuick(KCL::uint32& o_qp, KCL::uint32& o_qd, KCL::uint32& o_qpq) = 0;

	virtual void CheckLeaks() = 0;

protected:
	std::map<KCL::uint32, MeasureResults> m_session; //measure id and measure result

    KCL::uint32 m_quick_prims;
    KCL::uint32 m_quick_prims_query;
    KCL::uint32 m_quick_draws;
};

GLWrapper* GetGLWrapper();


namespace StatFunc
{
	inline bool compare_draw_calls(const MeasureResults &a, const MeasureResults &b)
	{
		if (a.draw_calls.size() >= b.draw_calls.size())
		{
			return false;
		}
		return true;
	}

	inline KCL::uint32 GetTotalBufferObjectSize(const MeasureResults& result)
	{
		long total = 0;
		buffer_it it = result.m_bufferObjects.begin();
		for (; it != result.m_bufferObjects.end(); it++)
		{
			total += it->second;
		}
		return total;
	}

	inline KCL::uint32 GetTotalDrawCalls(const MeasureResults& result)
	{
		return result.draw_calls.size();
	}

	inline KCL::uint32 GetTotalDrawPrimitives(const MeasureResults& result)
	{
		int total = 0;
		for (int i = 0; i < result.draw_calls.size(); i++)
		{
			total += result.draw_calls[i].num_draw_triangles;
		}
		return total;
	}
	inline KCL::uint32 GetTotalNumberTextures(const MeasureResults& result)
	{
		std::set<int> textures;
		for (int i = 0; i < result.draw_calls.size(); i++)
		{
			for (int h = 0; h < result.draw_calls[i].num_active_textures; h++)
			{
				textures.insert(result.draw_calls[i].texture_ids.begin(), result.draw_calls[i].texture_ids.end());
			}
		}
		return textures.size();
	}
	/*
	virtual KCL::uint32 GetMemoryAllocatedTexture()
	{
	return 0;
	}

	virtual KCL::uint32 GetMemoryAllocatedVertex()
	{
	return 0;
	}

	virtual KCL::uint32 GetMemoryAllocatedTotal()
	{
	return 0;
	}
	*/
}


#endif
