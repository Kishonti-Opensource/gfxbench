/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_ENVIRONTMENT_H
#define GFXB_ENVIRONTMENT_H

#include "gfxb_hdr.h"

#include <kcl_serializable.h>
#include <kcl_math3d.h>
#include <vector>

namespace GFXB
{

/*
class Color
{
	KCL::Vector3D m_color;
public:

	static const std::string GetColorString(KCL::Vector3D &color)
	{
		std::string hexa_color;
		KCL::Object::SerializeHexaColor(color.x, color.y, color.z, hexa_color);
		return hexa_color;
	}
};
*/

enum SkyFormat
{
	SKY_RGB = 0,
	SKY_RGBM,
	SKY_RGBE
};


struct SkyValues
{
	SkyFormat m_sky_format;
	std::string m_sky_texture;

	SkyValues()
	{
		m_sky_format = SKY_RGBE;
	}
	const char *GetSkyFormatString() const;
};


struct GIValues
{
	KCL::Vector3D m_sky_color;
	float m_sky_intensity;
	float m_indirect_lighting_factor;
	float m_ibl_diffuse_intensity;
	float m_ibl_reflection_intensity;

	GIValues()
	{
		m_sky_color = KCL::Vector3D(148.0f, 164.0f, 192.0f) / 255.0f;
		m_sky_intensity = 0.001f;
		m_indirect_lighting_factor = 1.0;
		m_ibl_diffuse_intensity = 1.0;
		m_ibl_reflection_intensity = 1.0;
	}
};


struct ColorCorrectionValues
{
	float m_contrast;
	float m_contrast_center;
	float m_saturation;

	ColorCorrectionValues()
	{
		m_contrast = 1.0f;
		m_contrast_center = 0.5f;
		m_saturation = 1.0f;
	}
};


struct SharpenValues
{
	float m_strength;
	float m_limit;

	SharpenValues()
	{
		m_strength = 0.0f;
		m_limit = 0.23f;
	}
};


struct FogValues
{
	KCL::Vector4D m_fog_color;
	KCL::Vector4D m_fog_parameters1;
	KCL::Vector4D m_fog_parameters2;

	FogValues()
	{
		m_fog_color.set(0.4f, 0.5f, 0.7f, 1.0f);

		// Distance fog
		m_fog_parameters1.x = 1.0f; // amount
		m_fog_parameters1.y = 0.0f; // density
		m_fog_parameters1.z = 0.0f; // start distance;

		// Vertical fog
		m_fog_parameters2.x = 1.0f; // amount
		m_fog_parameters2.y = 0.0f; // density
		m_fog_parameters2.z = 0.0f; // max height
	}
};


struct DOFValues
{
	float m_range;
	float m_focus_range;
	float m_function;
	KCL::uint32 m_strength;

	DOFValues()
	{
		m_range = 0.0f;
		m_focus_range = 0.0f;
		m_function = 1.0f;
		m_strength = 1;
	}
};


struct LODValues
{
	float m_lod1_distance;
	float m_lod2_distance;

	LODValues()
	{
		m_lod1_distance = 50.0f;
		m_lod2_distance = 100.0f;
	}
};


class Environment : public KCL::Serializable
{
	static const KCL::uint32 JSON_VERSION = 3;

public:
	Environment(KCL::uint32 shot_count);
	virtual ~Environment();

	virtual void Serialize(JsonSerializer& s) override;
	virtual std::string GetParameterFilename() const override;

	struct Values//read only, always be overwrited by Update()
	{
		bool m_is_per_shot_mode;
		SkyValues m_sky_values;
		GIValues m_gi_values;
		HDRValues m_hdr_values;
		ColorCorrectionValues m_color_correction_values;
		SharpenValues m_sharpen_values;
		FogValues m_fog_values;
		DOFValues m_dof_values;
		LODValues m_lod_values;
	};

	void Update(KCL::uint32 shot_index);

	const Values &GetValues() const;

	// Global values
	const SkyValues &GetSkyValues() const;
	const LODValues &GetLODValues() const;

	// Per shot values
	const bool &GetPerShotMode() const;
	const GIValues &GetGIValues() const;
	const HDRValues &GetHDRValues() const;
	const ColorCorrectionValues &GetColorCorrectionValues() const;
	const SharpenValues &GetSharpenValues() const;
	const FogValues &GetFogValues() const;
	const DOFValues &GetDOFValues() const;

	void GetValues(Values &values, KCL::uint32 shot_index);

	// Global values
	void SetSkyValues(const SkyValues &sky_values);

	// Per shot values
	void SetPerShotMode(const bool per_shot_mode);
	void SetLODValues(const LODValues &lod_values, KCL::uint32 shot_index = ~0u);
	void SetGIValues(const GIValues& gi_Values, KCL::uint32 shot_index = ~0u);
	void SetHDRValues(const HDRValues &hdr_values, KCL::uint32 shot_index = ~0u);
	void SetColorCorrectionValues(const ColorCorrectionValues &color_correction_values, KCL::uint32 shot_index = ~0u);
	void SetSharpenValues(const SharpenValues &sharpen_values, KCL::uint32 shot_index = ~0u);
	void SetDOFValues(const DOFValues &dof_values, KCL::uint32 shot_index = ~0u);
	void SetFogValues(const FogValues &fog_values, KCL::uint32 shot_index = ~0u);

public:
	struct GlobalValues
	{
		bool m_is_per_shot_mode;
		SkyValues m_sky_values;
	};
	struct ShotValues
	{
		KCL::uint32 m_shot_index;
		LODValues m_lod_values;
		HDRValues m_hdr_values;
		GIValues m_gi_values;
		ColorCorrectionValues m_color_correction_values;
		SharpenValues m_sharpen_values;
		FogValues m_fog_values;
		DOFValues m_dof_values;
	};

protected:
	virtual void OnParameterFileNotFound(const std::string &filename) override;

private:
	const KCL::uint32 m_shot_count;

	std::vector<ShotValues> m_shot_values;
	GlobalValues m_global_values;
	Values m_current_values;

	KCL::uint32 m_current_shot_index;

	void AllocateShotValues();
	void CollectValues(KCL::uint32 shot_index, Values &values);
};

}

#endif
