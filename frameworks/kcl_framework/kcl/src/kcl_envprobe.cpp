/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "kcl_envprobe.h"

using namespace KCL;

EnvProbe::EnvProbe()
{
	m_frame_counter = 0;
	m_index = 0;
}


EnvProbe::~EnvProbe()
{

}


void EnvProbe::Serialize(JsonSerializer& s)
{
	s.Serialize("pos", m_pos);
	s.Serialize("sampling_pos", m_sampling_pos);
	s.Serialize("half_extent", m_half_extent);
}


std::string EnvProbe::GetParameterFilename() const
{
	return "probe.json";
}
