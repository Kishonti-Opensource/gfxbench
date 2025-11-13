/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_environment.h"

#include <sstream>

using namespace GFXB;


Environment::Environment(KCL::uint32 shot_count) : m_shot_count(shot_count)
{
	m_global_values.m_is_per_shot_mode = true;
	m_current_shot_index = 0;
	AllocateShotValues();
}


Environment::~Environment()
{
}


void Environment::Update(KCL::uint32 shot_index)
{
	m_current_shot_index = shot_index;
	CollectValues(shot_index, m_current_values);
}


const Environment::Values &Environment::GetValues() const
{
	return m_current_values;
}


const GIValues &Environment::GetGIValues() const
{
	return m_current_values.m_gi_values;
}


const HDRValues &Environment::GetHDRValues() const
{
	return m_current_values.m_hdr_values;
}


const SkyValues &Environment::GetSkyValues() const
{
	return m_current_values.m_sky_values;
}


const LODValues &Environment::GetLODValues() const
{
	return m_current_values.m_lod_values;
}


const bool &Environment::GetPerShotMode() const
{
	return m_current_values.m_is_per_shot_mode;
}


const ColorCorrectionValues &Environment::GetColorCorrectionValues() const
{
	return m_current_values.m_color_correction_values;
}


const SharpenValues &Environment::GetSharpenValues() const
{
	return m_current_values.m_sharpen_values;
}


const FogValues &Environment::GetFogValues() const
{
	return m_current_values.m_fog_values;
}


const DOFValues &Environment::GetDOFValues() const
{
	return m_current_values.m_dof_values;
}


void Environment::GetValues(Values &values, KCL::uint32 shot_index)
{
	CollectValues(shot_index, values);
}


void Environment::SetSkyValues(const SkyValues &sky_values)
{
	m_global_values.m_sky_values = sky_values;

	CollectValues(m_current_shot_index, m_current_values);
}


void Environment::SetPerShotMode(const bool per_shot_mode)
{
	m_global_values.m_is_per_shot_mode= per_shot_mode;

	CollectValues(m_current_shot_index, m_current_values);
}


void Environment::SetLODValues(const LODValues &lod_values, KCL::uint32 shot_index)
{
	if (shot_index == ~0u)
	{
		for (size_t i = 0; i < m_shot_values.size(); i++)
		{
			m_shot_values[i].m_lod_values = lod_values;
		}
	}
	else
	{
		m_shot_values[shot_index].m_lod_values = lod_values;
	}

	CollectValues(m_current_shot_index, m_current_values);
}


void Environment::SetGIValues(const GIValues& gi_Values, KCL::uint32 shot_index)
{
	if (shot_index == ~0u)
	{
		for (size_t i = 0; i < m_shot_values.size(); i++)
		{
			m_shot_values[i].m_gi_values = gi_Values;
		}
	}
	else
	{
		m_shot_values[shot_index].m_gi_values = gi_Values;
	}

	CollectValues(m_current_shot_index, m_current_values);
}


void Environment::SetHDRValues(const HDRValues &hdr_values, KCL::uint32 shot_index)
{
	if (shot_index == ~0u)
	{
		for (size_t i = 0; i < m_shot_values.size(); i++)
		{
			m_shot_values[i].m_hdr_values = hdr_values;
		}
	}
	else
	{
		m_shot_values[shot_index].m_hdr_values = hdr_values;
	}

	CollectValues(m_current_shot_index, m_current_values);
}


void Environment::SetColorCorrectionValues(const ColorCorrectionValues &color_correction, KCL::uint32 shot_index)
{
	if (shot_index == ~0u)
	{
		for (size_t i = 0; i < m_shot_values.size(); i++)
		{
			m_shot_values[i].m_color_correction_values = color_correction;
		}
	}
	else
	{
		m_shot_values[shot_index].m_color_correction_values = color_correction;
	}

	CollectValues(m_current_shot_index, m_current_values);
}


void Environment::SetSharpenValues(const SharpenValues &sharpen_values, KCL::uint32 shot_index)
{
	if (shot_index == ~0u)
	{
		for (size_t i = 0; i < m_shot_values.size(); i++)
		{
			m_shot_values[i].m_sharpen_values = sharpen_values;
		}
	}
	else
	{
		m_shot_values[shot_index].m_sharpen_values = sharpen_values;
	}

	CollectValues(m_current_shot_index, m_current_values);
}


void Environment::SetDOFValues(const DOFValues &dof_values, KCL::uint32 shot_index)
{
	if (shot_index == ~0u)
	{
		for (size_t i = 0; i < m_shot_values.size(); i++)
		{
			m_shot_values[i].m_dof_values = dof_values;
		}
	}
	else
	{
		m_shot_values[shot_index].m_dof_values = dof_values;
	}

	CollectValues(m_current_shot_index, m_current_values);
}


void Environment::SetFogValues(const FogValues &fog_values, KCL::uint32 shot_index)
{
	if (shot_index == ~0u)
	{
		for (size_t i = 0; i < m_shot_values.size(); i++)
		{
			m_shot_values[i].m_fog_values = fog_values;
		}
	}
	else
	{
		m_shot_values[shot_index].m_fog_values = fog_values;
	}

	CollectValues(m_current_shot_index, m_current_values);
}


void Environment::AllocateShotValues()
{
	if (m_shot_values.size() == (size_t)m_shot_count)
	{
		return;
	}

	m_shot_values.resize(m_shot_count);
	for (KCL::uint32 i = 0; i < m_shot_count; i++)
	{
		m_shot_values[i].m_shot_index = i;
	}
}


void Environment::CollectValues(KCL::uint32 shot_index, Values &values)
{
	if (size_t(shot_index) >= m_shot_values.size())
	{
		INFO("Environment::CollectValues - Illegal shot index: %d", shot_index);
		assert(0);
		return;
	}

	// Global values
	values.m_sky_values = m_global_values.m_sky_values;
	values.m_is_per_shot_mode = m_global_values.m_is_per_shot_mode;

	// Shot values
	values.m_lod_values = m_shot_values[shot_index].m_lod_values;
	values.m_hdr_values = m_shot_values[shot_index].m_hdr_values;
	values.m_color_correction_values = m_shot_values[shot_index].m_color_correction_values;
	values.m_sharpen_values = m_shot_values[shot_index].m_sharpen_values;
	values.m_gi_values = m_shot_values[shot_index].m_gi_values;
	values.m_fog_values = m_shot_values[shot_index].m_fog_values;
	values.m_dof_values = m_shot_values[shot_index].m_dof_values;
}


void Environment::OnParameterFileNotFound(const std::string &filename)
{
	SaveParameters();
}


std::string Environment::GetParameterFilename() const
{
	return "environment.json";
}


const char *SkyValues::GetSkyFormatString() const
{
	switch (m_sky_format)
	{
	case SKY_RGB:
		return "SKY_FORMAT_RGB";

	case SKY_RGBM:
		return "SKY_FORMAT_RGBM";

	case SKY_RGBE:
	default:
		return "SKY_FORMAT_RGBE";
	}
}


void Environment::Serialize(JsonSerializer& s)
{
	s.Serialize("global_values", m_global_values);
	s.Serialize("shot_values", m_shot_values);

	if (s.IsWriter == false)
	{
		// Ensure all the shot values are allocated
		AllocateShotValues();

		Update(m_current_shot_index);
	}
}



void Serialize(Environment::GlobalValues &v, JsonSerializer& s)
{
	s.Serialize("sky_values", v.m_sky_values);
	s.Serialize("is_per_shot_mode", v.m_is_per_shot_mode);
}


void Serialize(Environment::ShotValues &v, JsonSerializer& s)
{
	s.Serialize("lod_values", v.m_lod_values);
	s.Serialize("_shot_index", v.m_shot_index);
	s.Serialize("hdr_values", v.m_hdr_values);
	s.Serialize("color_correction_values", v.m_color_correction_values);
	s.Serialize("sharpen_values", v.m_sharpen_values);
	s.Serialize("gi_values", v.m_gi_values);
	s.Serialize("fog_values", v.m_fog_values);
	s.Serialize("dof_values", v.m_dof_values);
}


void Serialize(GIValues &gi, JsonSerializer& s)
{
	s.Serialize("gi_indirect_lighting_factor", gi.m_indirect_lighting_factor);
	s.Serialize("gi_sky_color", gi.m_sky_color);
	s.Serialize("gi_sky_intensity", gi.m_sky_intensity);
	s.Serialize("gi_ibl_diffuse_intensity", gi.m_ibl_diffuse_intensity);
	s.Serialize("gi_ibl_reflection_intensity", gi.m_ibl_reflection_intensity);
}


void Serialize(SkyValues &v, JsonSerializer& s)
{
	s.Serialize("sky_format", (KCL::uint32&)v.m_sky_format);
	s.Serialize("sky_texture", v.m_sky_texture);
}


void Serialize(LODValues &v, JsonSerializer& s)
{
	s.Serialize("lod1_distance", v.m_lod1_distance);
	s.Serialize("lod2_distance", v.m_lod2_distance);
}


void Serialize(ColorCorrectionValues &v, JsonSerializer& s)
{
	s.Serialize("contrast", v.m_contrast);
	s.Serialize("contrast_center", v.m_contrast_center);
	s.Serialize("saturation", v.m_saturation);
}


void Serialize(SharpenValues &v, JsonSerializer& s)
{
	s.Serialize("sharpen_strength", v.m_strength);
	s.Serialize("sharpen_limit", v.m_limit);
}


void Serialize(DOFValues &v, JsonSerializer& s)
{
	s.Serialize("dof_strength", v.m_strength);
	s.Serialize("dof_range", v.m_range);
	s.Serialize("dof_focus_range", v.m_focus_range);
	s.Serialize("dof_function", v.m_function);
}


void Serialize(FogValues &v, JsonSerializer& s)
{
	s.Serialize("fog_parameters1", v.m_fog_parameters1);
	s.Serialize("fog_parameters2", v.m_fog_parameters2);
	s.Serialize("fog_color", v.m_fog_color);
}
