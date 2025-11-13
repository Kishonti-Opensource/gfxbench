/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxresult.h"

using namespace tfw;

void GfxResult::fromJsonValue(const ng::JsonValue &jvalue)
{
	if (jvalue.isNull())
	{
		return;
	}

	setFps(jvalue["fps"].numberD(-1));
	setFrameCount((int)jvalue["frame_count"].numberD(-1));
	setEglConfigId((int)jvalue["egl_config_id"].numberD(-1));
	setVendor(jvalue["vendor"].stringD(""));
	setRenderer(jvalue["renderer"].stringD(""));
	setGraphicsVersion(jvalue["graphics_version"].stringD(""));
	setSurfaceWidth((int)jvalue["surface_width"].numberD(-1));
	setSurfaceHeight((int)jvalue["surface_height"].numberD(-1));
	setIsVSyncLimited(jvalue["is_vsync_limited"].booleanD(false));

	const ng::JsonValue& jextraData = jvalue["extra_data"];
	if (!jextraData.isNull() && jextraData.isArray())
	{
		std::vector<std::string> &extradata = extraData();
		for (size_t i(0); i < jextraData.size(); ++i)
		{
			extradata.push_back(jextraData[i].stringD(""));
		}
	}
	const ng::JsonValue& jframeTimes = jvalue["frametimes"];
	if (!jframeTimes.isNull() && jframeTimes.isArray())
	{
		std::vector<int> &frametimes = frameTimes();
		for (size_t i(0); i< jframeTimes.size(); ++i)
		{
			frametimes.push_back((int)jframeTimes[i].numberD(0));
		}
	}
	const ng::JsonValue& jCPUfreqs = jvalue["cpufreqs"];
	if (!jCPUfreqs.isNull() && jCPUfreqs.isArray())
	{
		std::vector<std::vector<int> > cpufreqs = CPUfreqs();
		for (size_t i(0); i < jCPUfreqs.size(); ++i)
		{
			ng::JsonValue jFrameCPUfreqs = jCPUfreqs[i];
			cpufreqs.push_back(std::vector<int>());
			std::vector<int>& freqs = cpufreqs[cpufreqs.size() - 1];
			for (size_t j(0); j < jFrameCPUfreqs.size(); ++j)
			{
				freqs.push_back((int)jFrameCPUfreqs[i].numberD(0));
			}
		}
	}
}

ng::JsonValue GfxResult::toJsonValue() const
{
	ng::JsonValue jvalue;
	if (isEmpty())
	{
		return jvalue;
	}

	jvalue["fps"] = fps();
	jvalue["frame_count"] = frameCount();
	jvalue["egl_config_id"] = eglConfigId();
	jvalue["vendor"] = vendor();
	jvalue["renderer"] = renderer();
	jvalue["graphics_version"] = graphicsVersion();
	jvalue["surface_width"] = surfaceWidth();
	jvalue["surface_height"] = surfaceHeight();
	jvalue["is_vsync_limited"] = isVSyncLimited();
	const std::vector<std::string> &extradata = extraData();
	if (extradata.size() != 0)
	{
		jvalue["extra_data"] = ng::JsonValue();
		ng::JsonValue &jextraData = jvalue["extra_data"];
		jextraData.resize(extradata.size());
		for (size_t i(0); i < extradata.size(); ++i)
		{
			jextraData[i] = extradata[i];
		}
	}
	const std::vector<int> &frameTimes = getFrameTimes();
	if (frameTimes.size() != 0)
	{
		jvalue["frametimes"] = ng::JsonValue();
		ng::JsonValue &jframeTimes = jvalue["frametimes"];
		jframeTimes.resize(frameTimes.size());
		size_t ctr = 0;
		for (std::vector<int>::const_iterator i = frameTimes.begin(); i != frameTimes.end(); ++i, ++ctr)
		{
			jframeTimes[ctr] = *i;
		}
	}
	const std::vector<std::vector<int> > &CPUFreqs = getCPUfreqs();
	if (CPUFreqs.size() != 0)
	{
		jvalue["cpufreqs"] = ng::JsonValue();
		ng::JsonValue &jCPUfreqs = jvalue["cpufreqs"];
		jCPUfreqs.resize(CPUFreqs.size());
		size_t ctr = 0;
		for (std::vector<std::vector<int> >::const_iterator i = CPUFreqs.begin(); i != CPUFreqs.end(); ++i, ++ctr)
		{
			ng::JsonValue &jFrameCPUfreqs = jCPUfreqs[ctr];
			jFrameCPUfreqs.resize((*i).size());
			size_t ctr2 = 0;
			for (std::vector<int>::const_iterator j = (*i).begin(); j != (*i).end(); ++j, ++ctr2)
			{
				jFrameCPUfreqs[ctr2] = *j;
			}
		}
	}

	return jvalue;
}

