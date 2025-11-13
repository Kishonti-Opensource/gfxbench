/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_lightshaft.h"

using namespace GFXB;

Lightshaft::Lightshaft(const std::string &name)
{
	m_name = name;
}


Lightshaft::~Lightshaft()
{

}


void Lightshaft::Animate(KCL::uint32 shot_index)
{
	m_current_values = GetValues(shot_index);
}


Lightshaft::Values &Lightshaft::GetValues()
{
	return m_current_values;
}


Lightshaft::Values &Lightshaft::GetValues(KCL::uint32 shot_index)
{
	EnsureValues(shot_index);
	return m_values[shot_index];
}


void Lightshaft::SetValues(const Values &values)
{
	for (size_t i = 0; i < m_values.size(); i++)
	{
		m_values[i] = values;
		m_values[i].shot_index = (KCL::uint32) i;
	}
	m_current_values = values;
}


void Lightshaft::SetValues(const Values &values, KCL::uint32 shot_index)
{
	GetValues(shot_index) = values;
}


void Lightshaft::SetShotCount(KCL::uint32 shot_count)
{
	if (shot_count > 0)
	{
		EnsureValues(shot_count - 1);
	}
}


std::string Lightshaft::GetParameterFilename() const
{
	return "lights/" + m_name + ".json";
}


void Lightshaft::Serialize(JsonSerializer& s)
{
	s.Serialize("name", m_name);
	s.Serialize("values", m_values);
}


void Lightshaft::EnsureValues(KCL::uint32 shot_index)
{
	if (m_values.size() > shot_index)
	{
		return;
	}

	while (m_values.size() <= shot_index)
	{
		Values values;
		values.shot_index = (KCL::uint32)m_values.size();
		m_values.push_back(values);
	}
}


void Serialize(Lightshaft::Values &v, JsonSerializer& s)
{
	s.Serialize("shot_index", v.shot_index);
	s.Serialize("samples", v.m_sample_count);
	s.Serialize("intensity", v.m_intensity);
}
