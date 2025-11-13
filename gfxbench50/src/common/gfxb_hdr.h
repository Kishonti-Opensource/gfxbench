/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_HDR_H
#define GFXB_HDR_H

#include <kcl_base.h>
#include <kcl_math3d.h>
#include <kcl_object.h>


#define DEBUG_HDR_UVS 0


namespace GFXB
{
	class SceneBase;

	enum ExposureMode
	{
		EXPOSURE_MANUAL = 0,	// Manually provided exposure value
		EXPOSURE_AUTO,			// Exposure defined automatically from frame's luminance
		EXPOSURE_ADAPTIVE,		// Exposure defined automatically from eye's adaptive luminance
		EXPOSURE_PREDEFINED,	// Exposure calculated form predefined luminance values
		EXPOSURE_MODE_COUNT
	};

	enum AdaptationMode
	{
		ADAPTATION_ENABLED = 0, // Default
		ADAPTATION_DISABLED = 1,
		ADAPTATION_PREDEFINED = 2 // Use the predefined luminance values
	};

	struct HDRValues
	{
		HDRValues();
		~HDRValues();

		void ResetTonemapper();

		// Exposure
		ExposureMode m_exposure_mode;
		float m_exposure;
		float m_auto_exposure_bias;
		float m_auto_min_exposure;
		float m_auto_max_exposure;
		float m_adaptation_speed;

		// Hejl / Burgess-Dawson (Filmic Tonemapping) constants
		// Default values are from John Hable (Uncharted 2)
		float m_linear_white;
		float m_shoulder_strength;
		float m_linear_strength;
		float m_linear_angle;
		float m_toe_strength;
		float m_toe_numerator;
		float m_toe_denominator;

		// Bloom
		float m_bloom_threshold;
		float m_bloom_intensity;
	};

	class ComputeHDR
	{
	private:
		static const KCL::uint32 PASS1_WORKGROUP_SIZE_X = 16;
		static const KCL::uint32 PASS1_WORKGROUP_SIZE_Y = 8;

		static const KCL::uint32 PASS1_DISPATCH_X = 8;
		static const KCL::uint32 PASS1_DISPATCH_Y = 8;

		static const KCL::uint32 PASS2_WORKGROUP_SIZE = 64;

	public:
		ComputeHDR();
		~ComputeHDR();

		void Init(SceneBase *scene, KCL::uint32 viewport_width, KCL::uint32 viewport_height, KCL::uint32 downsample);
		void SetInputTexture(KCL::uint32 input_texture);

		void UpdateViewport(KCL::uint32 width, KCL::uint32 height);
		void DeleteRenderers();

		void Animate(const HDRValues &hdr_values, KCL::uint32 animation_time);
		void Render(KCL::uint32 command_buffer);

		void Reset();

		void Warmup(KCL::uint32 command_buffer, KCL::uint32 input_texture);

		void LoadLuminanceValues(const std::string &filename);
		void SaveLuminanceValues(const std::string &filename);

		void LoadReductionPass1();
		void ReloadReductionPass1();

		KCL::uint32 GetReductionPass1() const;
		KCL::uint32 GetReductionPass2() const;

		KCL::Vector4D GetExposureValues() const;
		ExposureMode GetExposureMode() const;
		const HDRValues GetHDRValues() const;

		void *GetUniformTonemapperConstants1();
		void *GetUniformTonemapperConstants2();
		void *GetUniformExposure();
		void *GetUniformTonemapWhite();
		void *GetUniformBloomParameters();

		void SetDownsample( unsigned x )
		{
			m_downsample = x;
		}

		void SetSaveLuminanceValues(bool value)
		{
			m_hdr_save_luminance_values = value;
		}

		KCL::uint32 GetLuminanceBuffer() const;

		static void GetTonemapperCurve(std::vector<float> &sample_points, const HDRValues& constants);
		static float FilmicTonemap(float linear_color, const HDRValues& constants);
		static float FilmicTonemapFunc(float linear_color, const HDRValues& constants);

	private:
		GFXB::SceneBase *m_scene;

		KCL::uint32 m_downsample;

		KCL::uint32 m_reduction_pass1_shader;
		KCL::uint32 m_reduction_pass2_shader;
		KCL::uint32 m_reduction_pass2_auto_shader;
		KCL::uint32 m_reduction_pass2_adaptive_shader;

		KCL::uint32 m_reduction_pass1;
		KCL::uint32 m_reduction_pass2;

		KCL::uint32 m_input_texture;
		KCL::Vector2D m_texture_samples_inv;

		KCL::uint32 m_viewport_width;
		KCL::uint32 m_viewport_height;

		int m_sample_per_thread_x;
		int m_sample_per_thread_y;

		struct work_group_size
		{
			KCL::uint32 x, y, z, w;
		};
		work_group_size m_dispatch_pass1;
		work_group_size m_dispatch_pass2;

		KCL::uint32 m_pass1_output_buffer;
		KCL::uint32 m_pass2_output_buffer;

		float m_delta_time;

		KCL::int32 m_last_anim_time;

		HDRValues m_hdr_values;

		bool m_hdr_save_luminance_values;
		bool m_hdr_use_predefined_luminance;
		std::map<KCL::uint32, float> m_luminance_map;

		// Uniforms
		KCL::Vector4D m_hdr_abcd; 		// a - ShoulderStrength, b - LinearStrength, c - LinearAngle, d - ToeStrength
		KCL::Vector4D m_hdr_efw_tau;	// e - ToeNumerator, f - ToeDenominator, w - LinearWhite, tau - Adaptation Range
		KCL::Vector4D m_hdr_exposure;
		KCL::Vector4D m_hdr_auto_exposure_values;
		float m_hdr_tonemap_white;
		KCL::Vector4D m_bloom_parameters;
		KCL::Vector2D m_hdr_predefined_luminanace; // x - luminance value, y - enable/disable flag

		KCL::uint32 CreateBuffer(KCL::uint32 stride, KCL::uint32 num);
		static const char* GetExposureModeDefine(ExposureMode mode);

#if DEBUG_HDR_UVS
	public:
		uint32_t m_uv_test_texture;

	private:
		void InitDebug();

		uint32_t m_uv_test_texture_clear_job;
#endif

	};

	std::string GetPredefinedLuminanceFilename(const std::string tier_level_name);
}

#endif