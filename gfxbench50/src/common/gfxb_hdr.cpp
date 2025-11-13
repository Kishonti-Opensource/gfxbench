/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_hdr.h"

#include <ngl.h>

#include <kcl_scene_handler.h>

#include "gfxb_scene_base.h"
#include "gfxb_shader.h"
#include "gfxb_shot_handler.h"
#include "gfxb_barrier.h"

using namespace GFXB;



ComputeHDR::ComputeHDR()
{
	m_scene = nullptr;

	m_downsample = 1;

	m_reduction_pass1_shader = 0;
	m_reduction_pass2_shader = 0;
	m_reduction_pass2_auto_shader = 0;
	m_reduction_pass2_adaptive_shader = 0;

	m_reduction_pass1 = 0;
	m_reduction_pass2 = 0;

	m_dispatch_pass1.x = PASS1_DISPATCH_X;
	m_dispatch_pass1.y = PASS1_DISPATCH_Y;
	m_dispatch_pass1.z = 1;
	m_dispatch_pass1.w = 1;

	m_dispatch_pass2.x = 1;
	m_dispatch_pass2.y = 1;
	m_dispatch_pass2.z = 1;
	m_dispatch_pass2.w = 1;

	m_last_anim_time = -1;

	m_hdr_tonemap_white = 0.0f;

	m_viewport_width = 0;
	m_viewport_height = 0;

	m_sample_per_thread_x = 0;
	m_sample_per_thread_y = 0;

#if DEBUG_HDR_UVS
	m_uv_test_texture = 0;
	m_uv_test_texture_clear_job = 0;
#endif

	m_hdr_save_luminance_values = false;
	m_hdr_use_predefined_luminance = false;
	m_hdr_predefined_luminanace = KCL::Vector2D(0.0f, 0.0f);
}


ComputeHDR::~ComputeHDR()
{
}


void ComputeHDR::Init(SceneBase *scene, KCL::uint32 viewport_width, KCL::uint32 viewport_height, KCL::uint32 downsample)
{
	m_scene = scene;

	m_downsample = downsample;

	UpdateViewport(viewport_width, viewport_height);

	m_pass1_output_buffer = CreateBuffer(sizeof(float), PASS2_WORKGROUP_SIZE);
	m_pass2_output_buffer = CreateBuffer(sizeof(KCL::Vector4D), 1);

	{
		NGL_job_descriptor cd;
		{
			NGL_subpass sp;
			sp.m_name = "hdr::reduction_pass1";
			cd.m_subpasses.push_back(sp);
		}
		cd.m_is_compute = true;
		cd.m_load_shader_callback = LoadShader;
		m_reduction_pass1 = nglGenJob(cd);

		ReloadReductionPass1();
	}

	{
		NGL_job_descriptor cd;
		{
			NGL_subpass sp;
			sp.m_name = "hdr::reduction_pass2";
			cd.m_subpasses.push_back(sp);
		}
		cd.m_is_compute = true;
		cd.m_load_shader_callback = LoadShader;
		m_reduction_pass2 = nglGenJob(cd);

		{
			ShaderDescriptor desc =
				ShaderDescriptor("hdr_reduction_pass2.shader").
				AddDefine(GetExposureModeDefine(EXPOSURE_AUTO)).
				SetWorkgroupSize(64, 1, 1);
			m_reduction_pass2_auto_shader = ShaderFactory::GetInstance()->AddDescriptor(desc);
		}

		{
			ShaderDescriptor desc =
				ShaderDescriptor("hdr_reduction_pass2.shader").
				AddDefine(GetExposureModeDefine(EXPOSURE_ADAPTIVE)).
				SetWorkgroupSize(64, 1, 1);
			m_reduction_pass2_adaptive_shader = ShaderFactory::GetInstance()->AddDescriptor(desc);
		}

		m_reduction_pass2_shader = m_reduction_pass2_auto_shader;
	}

#if DEBUG_HDR_UVS
	InitDebug();
#endif
}


void ComputeHDR::UpdateViewport(KCL::uint32 width, KCL::uint32 height)
{
	m_viewport_width = width;
	m_viewport_height = height;

	// The number of compute thread in each dimension
	const int threads_x = PASS1_DISPATCH_X * PASS1_WORKGROUP_SIZE_X;
	const int threads_y = PASS1_DISPATCH_Y * PASS1_WORKGROUP_SIZE_Y;

	// The number of samples in each dimension
	m_sample_per_thread_x = width / threads_x / m_downsample;
	m_sample_per_thread_y = height / threads_y / m_downsample;
	m_sample_per_thread_x = KCL::Max(m_sample_per_thread_x, 1);
	m_sample_per_thread_y = KCL::Max(m_sample_per_thread_y, 1);

	// Count of the all samples
	m_texture_samples_inv.x = 1.0f / (m_sample_per_thread_x * threads_x);
	m_texture_samples_inv.y = 1.0f / (m_sample_per_thread_y * threads_y);

	LoadReductionPass1();
}


void ComputeHDR::DeleteRenderers()
{
	nglDeletePipelines(m_reduction_pass1);
	nglDeletePipelines(m_reduction_pass2);
}


void ComputeHDR::SetInputTexture(KCL::uint32 input_texture)
{
	m_input_texture = input_texture;
}


void ComputeHDR::Animate(const HDRValues &hdr_values, KCL::uint32 animation_time)
{
	m_hdr_values = hdr_values;

	if (m_hdr_save_luminance_values && m_hdr_values.m_exposure_mode == EXPOSURE_PREDEFINED)
	{
		m_hdr_values.m_exposure_mode = EXPOSURE_ADAPTIVE;
	}

	// Here we check if we can enable the adaptive exposure mode if requested
	// Note: delta time is only calulcated when we use adaptive exposure. Other shaders do not use it
	if (m_hdr_values.m_exposure_mode == EXPOSURE_ADAPTIVE)
	{
		if (m_last_anim_time < 0 || m_scene->GetCameraShotHandler()->IsCameraShotChanged())
		{
			// This is the first frame or the shot is changed
			m_hdr_values.m_exposure_mode = EXPOSURE_AUTO;
		}
		else
		{
			// Calulcate the delta time
			m_delta_time = float(animation_time) - float(m_last_anim_time);
			m_delta_time *= 0.001f; // to seconds

			if (m_scene->IsEditorMode())
			{
				// Allow adaptation in editor when the scene is paused
				if (m_delta_time == 0.0f)
				{
					m_delta_time = 0.04f;
				}
			}
			else if (m_delta_time <= 0.0f)
			{
				// Single frame rendering or going back in time
				m_hdr_values.m_exposure_mode = EXPOSURE_AUTO;
			}
		}
	}

	m_last_anim_time = animation_time;

	if (m_hdr_save_luminance_values)
	{
		m_hdr_use_predefined_luminance = false;
	}
	else
	{
		m_hdr_use_predefined_luminance = (m_hdr_values.m_exposure_mode == EXPOSURE_PREDEFINED);
		m_hdr_predefined_luminanace.y = m_hdr_use_predefined_luminance ? 1.0f : 0.0f;
	}

	if (m_hdr_use_predefined_luminance)
	{
		if (m_luminance_map.find(animation_time) != m_luminance_map.end())
		{
			m_hdr_predefined_luminanace.x = m_luminance_map.at(animation_time);
		}
		else
		{
			if (m_luminance_map.empty())
			{
				INFO("Predefined luminance values missing for ALL frames");
			}
			else
			{
				INFO("Predefined luminance value missing for frame %d", animation_time);
			}
		}

		if (m_hdr_predefined_luminanace.x >= 0.0f)
		{
			m_hdr_values.m_exposure_mode = EXPOSURE_ADAPTIVE;
		}
		else
		{
			m_hdr_values.m_exposure_mode = EXPOSURE_AUTO;
		}
	}

	// Select the appropriate shader
	if (m_hdr_values.m_exposure_mode == EXPOSURE_AUTO)
	{
		m_reduction_pass2_shader = m_reduction_pass2_auto_shader;
	}
	else if (m_hdr_values.m_exposure_mode == EXPOSURE_ADAPTIVE)
	{
		m_reduction_pass2_shader = m_reduction_pass2_adaptive_shader;
	}

	// Select the appropriate exposure
	if (m_hdr_values.m_exposure_mode == EXPOSURE_MANUAL)
	{
		// Use the predefined exposure value
		m_hdr_exposure.x = hdr_values.m_exposure;
		m_hdr_exposure.y = (float) pow(2.0f, hdr_values.m_exposure); // Linear exposure
		m_hdr_auto_exposure_values.x = hdr_values.m_exposure;
		m_hdr_auto_exposure_values.y = (float)pow(2.0f, hdr_values.m_exposure);
	}
	else
	{
		// Auto or adaptive: use the exposure bias
		m_hdr_auto_exposure_values.x = hdr_values.m_auto_exposure_bias;
		m_hdr_auto_exposure_values.y = (float)pow(2.0f, hdr_values.m_auto_min_exposure); // Min linear exposure
		m_hdr_auto_exposure_values.z = (float)pow(2.0f, hdr_values.m_auto_max_exposure); // Max linear exposure

		m_hdr_auto_exposure_values.y = KCL::Max(m_hdr_auto_exposure_values.y, 0.001f);
	}

	m_hdr_abcd.x = hdr_values.m_shoulder_strength;
	m_hdr_abcd.y = hdr_values.m_linear_strength;
	m_hdr_abcd.z = hdr_values.m_linear_angle;
	m_hdr_abcd.w = hdr_values.m_toe_strength;

	m_hdr_efw_tau.x = hdr_values.m_toe_numerator;
	m_hdr_efw_tau.y = hdr_values.m_toe_denominator;
	m_hdr_efw_tau.z = hdr_values.m_linear_white;
	m_hdr_efw_tau.w = hdr_values.m_adaptation_speed;

	m_hdr_tonemap_white = 1.0f / FilmicTonemapFunc(hdr_values.m_linear_white, hdr_values); // White scale inverse

	// Bloom
	m_bloom_parameters.x = hdr_values.m_bloom_threshold;
	m_bloom_parameters.y = hdr_values.m_bloom_intensity;
}


void ComputeHDR::Render(KCL::uint32 command_buffer)
{
	if (m_hdr_values.m_exposure_mode == EXPOSURE_MANUAL)
	{
		return;
	}

	if (m_hdr_save_luminance_values)
	{
		if (m_hdr_values.m_exposure_mode == EXPOSURE_ADAPTIVE)
		{
			KCL::Vector4D save_values = GetExposureValues();
			m_luminance_map[m_last_anim_time] = save_values.z;
		}
		else
		{
			m_luminance_map[m_last_anim_time] = -1.0f;
		}
	}

	Transitions::Get()
		.TextureBarrier(m_input_texture, NGL_SHADER_RESOURCE_AND_NON_FRAGMENT_SHADER_RESOURCE)
		.BufferBarrier(m_pass1_output_buffer, NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS_AND_NON_FRAGMENT_SHADER_RESOURCE)
		.Execute(command_buffer);

	{
#if DEBUG_HDR_UVS
		nglBegin(m_uv_test_texture_clear_job,command_buffer);
		nglEnd(m_uv_test_texture_clear_job);
#endif

		nglBegin(m_reduction_pass1, command_buffer);

		// Dispatch reduction pass 1
		const void *p[UNIFORM_MAX];
		p[UNIFORM_INPUT_TEXTURE] = &m_input_texture;
		p[UNIFORM_HDR_REDUCTION_VALUES] = &m_pass1_output_buffer;

#if DEBUG_HDR_UVS
		p[UNIFORM_HDR_UV_TEST_TEXTURE] = &m_uv_test_texture;
#endif

		nglDispatch(m_reduction_pass1, m_reduction_pass1_shader, PASS1_DISPATCH_X, PASS1_DISPATCH_Y, 1, p);

		nglEnd(m_reduction_pass1);
	}

	Transitions::Get()
		.BufferBarrier(m_pass1_output_buffer, NGL_SHADER_RESOURCE_AND_NON_FRAGMENT_SHADER_RESOURCE)
		.BufferBarrier(m_pass2_output_buffer, NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS_AND_NON_FRAGMENT_SHADER_RESOURCE)
		.Execute(command_buffer);

	{
		nglBegin(m_reduction_pass2, command_buffer);

		// Dispatch reduction pass 2
		const void *p[UNIFORM_MAX];
		p[UNIFORM_HDR_REDUCTION_VALUES] = &m_pass1_output_buffer;
		p[UNIFORM_DELTA_TIME] = &m_delta_time;
		p[UNIFORM_TEXTURE_SAMPLES_INV] = m_texture_samples_inv.v;
		p[UNIFORM_HDR_ABCD] = m_hdr_abcd.v;
		p[UNIFORM_HDR_EFW_TAU] = m_hdr_efw_tau.v;
		p[UNIFORM_HDR_AUTO_EXPOSURE_VALUES] = &m_hdr_auto_exposure_values;
		p[UNIFORM_HDR_EXPOSURE] = &m_pass2_output_buffer;
		p[UNIFORM_NUM_WORK_GROUPS] = &m_dispatch_pass2;
		p[UNIFORM_HDR_PREDEFINED_LUMINANACE] = &m_hdr_predefined_luminanace;

		nglDispatch(m_reduction_pass2, m_reduction_pass2_shader, 1, 1, 1, p);

		nglEnd(m_reduction_pass2);
	}

#if 0
	KCL::Vector4D values = GetExposureValues();
	INFO("Exposure: %f, Linear exposure: %f, Frame luminance: %f, Average luminance: %f",
		values.x, values.y, values.z, values.w);
#endif
}


void ComputeHDR::Warmup(KCL::uint32 command_buffer, KCL::uint32 input_texture)
{
	SetInputTexture(input_texture);

	{
		nglBeginCommandBuffer(command_buffer);

		m_hdr_values.m_exposure_mode = EXPOSURE_AUTO;
		m_reduction_pass2_shader = m_reduction_pass2_auto_shader;

		Render(command_buffer);

		nglEndCommandBuffer(command_buffer);
		nglSubmitCommandBuffer(command_buffer);
	}

	{
		nglBeginCommandBuffer(command_buffer);

		m_hdr_values.m_exposure_mode = EXPOSURE_ADAPTIVE;
		m_reduction_pass2_shader = m_reduction_pass2_adaptive_shader;

		Render(command_buffer);

		nglEndCommandBuffer(command_buffer);
		nglSubmitCommandBuffer(command_buffer);
	}
}


void ComputeHDR::Reset()
{
	m_last_anim_time = -1;
}


KCL::uint32 ComputeHDR::CreateBuffer(KCL::uint32 stride, KCL::uint32 num)
{
	NGL_vertex_descriptor vl;
	vl.m_stride = stride;
	vl.m_unordered_access = true;

	KCL::uint32 buffer = 0;
	nglGenVertexBuffer(buffer, vl, num, nullptr);

	Transitions::Get().Register(buffer, vl);

	return buffer;
}


const char* ComputeHDR::GetExposureModeDefine(ExposureMode mode)
{
	switch (mode)
	{
	case EXPOSURE_MANUAL:
		return "EXPOSURE_MANUAL";

	case EXPOSURE_AUTO:
		return "EXPOSURE_AUTO";

	case EXPOSURE_ADAPTIVE:
		return "EXPOSURE_ADAPTIVE";

	case EXPOSURE_PREDEFINED:
		return "EXPOSURE_PREDEFINED";

	default:
		assert(0);
		return "Unknown exposure mode!";
	}
}


KCL::uint32 ComputeHDR::GetReductionPass1() const
{
	return m_reduction_pass1;
}


KCL::uint32 ComputeHDR::GetReductionPass2() const
{
	return m_reduction_pass2;
}


const HDRValues ComputeHDR::GetHDRValues() const
{
	return m_hdr_values;
}


KCL::Vector4D ComputeHDR::GetExposureValues() const
{
	if (m_hdr_values.m_exposure_mode == EXPOSURE_MANUAL)
	{
		return KCL::Vector4D(m_hdr_values.m_exposure, (float)pow(2.0f, m_hdr_values.m_exposure), -1.0f, -1.0f);
	}

	std::vector<uint8_t> content;
	bool result = nglGetVertexBufferContent(m_pass2_output_buffer, Transitions::Get().GetBufferState(m_pass2_output_buffer), content);
	if (!result || content.size() < sizeof(KCL::Vector4D))
	{
		INFO("ComputeHDR - Can not read back exposure value!");
		return KCL::Vector4D(-1.0f, -1.0f, -1.0f, -1.0f);
	}

	float *ptr = (float*)content.data();
	return KCL::Vector4D(ptr[0], ptr[1], ptr[2], ptr[3]);
}


ExposureMode ComputeHDR::GetExposureMode() const
{
	return m_hdr_values.m_exposure_mode;
}


void *ComputeHDR::GetUniformTonemapperConstants1()
{
	return m_hdr_abcd.v;
}


void *ComputeHDR::GetUniformTonemapperConstants2()
{
	return m_hdr_efw_tau.v;
}


void *ComputeHDR::GetUniformExposure()
{
	if (m_hdr_values.m_exposure_mode == EXPOSURE_MANUAL)
	{
		return m_hdr_exposure.v;
	}
	else
	{
		return &m_pass2_output_buffer;
	}
}


void *ComputeHDR::GetUniformTonemapWhite()
{
	return &m_hdr_tonemap_white;
}


void *ComputeHDR::GetUniformBloomParameters()
{
	return m_bloom_parameters.v;
}


KCL::uint32 ComputeHDR::GetLuminanceBuffer() const
{
	return m_pass2_output_buffer;
}


float ComputeHDR::FilmicTonemapFunc(float linear_color, const HDRValues& constants)
{
	const float &A = constants.m_shoulder_strength;
	const float &B = constants.m_linear_strength;
	const float &C = constants.m_linear_angle;
	const float &D = constants.m_toe_strength;
	const float &E = constants.m_toe_numerator;
	const float &F = constants.m_toe_denominator;
	float x = linear_color;

	return ((x*(A*x + C*B) + D*E) / (x*(A*x + B) + D*F)) - E / F;
}


float ComputeHDR::FilmicTonemap(float linear_color, const HDRValues& constants)
{
	float numerator = FilmicTonemapFunc(linear_color, constants);
	float denominator = FilmicTonemapFunc(constants.m_linear_white, constants);

	// Scale the color with the tonemapped linear white
	return numerator / denominator;
}


void ComputeHDR::GetTonemapperCurve(std::vector<float> &sample_points, const HDRValues& constants)
{
	const float width = float(sample_points.size());
	const float min_logX = -5.0f;
	const float max_logX = 5.0f;
	const float max_y = 1.0f;

	// Get the tone map curve
	for (size_t i = 0; i < sample_points.size(); i++)
	{
		float t = float(i) / width;
		float log_x = min_logX + ((max_logX - min_logX) * t);
		float x = std::pow(10.0f, log_x);
		sample_points[i] = FilmicTonemap(x, constants) / max_y;
	}
}


HDRValues::HDRValues()
{
	m_exposure_mode = EXPOSURE_MANUAL;
	m_exposure = 0.0f;
	m_auto_exposure_bias = 0.0f;
	m_auto_min_exposure = -10.0f;
	m_auto_max_exposure = 2.5f;
	m_adaptation_speed = 0.8f;

	m_bloom_threshold = 1.5f;
	m_bloom_intensity = 0.0f;

	ResetTonemapper();
}


HDRValues::~HDRValues()
{
}


void HDRValues::ResetTonemapper()
{
	// Hejl / Burgess-Dawson (Filmic Tonemapping) constants
	// Default values are from John Hable (Uncharted 2)
	m_shoulder_strength = 0.22f;	// A 0.22
	m_linear_strength = 0.3f;		// B 0.3
	m_linear_angle = 0.1f;			// C 0.1
	m_toe_strength = 0.2f;			// D 0.2
	m_toe_numerator = 0.01f;		// E 0.01
	m_toe_denominator = 0.3f;		// F 0.3
	m_linear_white = 7.0f;			// W ~7.0 to match with Marmoset, John Hable's default is 11.2
}


void Serialize(HDRValues &hdr, JsonSerializer& s)
{
	s.Serialize("exposure_mode", (KCL::uint32&)hdr.m_exposure_mode);
	s.Serialize("exposure", hdr.m_exposure);
	s.Serialize("exposure_bias", hdr.m_auto_exposure_bias);
	s.Serialize("exposure_min", hdr.m_auto_min_exposure);
	s.Serialize("exposure_max", hdr.m_auto_max_exposure);
	s.Serialize("adaptation_speed", hdr.m_adaptation_speed);

	s.Serialize("shoulder_strength", hdr.m_shoulder_strength);
	s.Serialize("linear_strength", hdr.m_linear_strength);
	s.Serialize("linear_angle", hdr.m_linear_angle);
	s.Serialize("toe_strength", hdr.m_toe_strength);
	s.Serialize("toe_numerator", hdr.m_toe_numerator);
	s.Serialize("toe_denominator", hdr.m_toe_denominator);
	s.Serialize("linear_white", hdr.m_linear_white);

	s.Serialize("bloom_threshold", hdr.m_bloom_threshold);
	s.Serialize("bloom_intensity", hdr.m_bloom_intensity);
}


void ComputeHDR::LoadReductionPass1()
{
	ShaderDescriptor sd("hdr_reduction_pass1.shader");
	sd.SetWorkgroupSize(PASS1_WORKGROUP_SIZE_X, PASS1_WORKGROUP_SIZE_Y, 1);
	sd.AddDefineInt("HDR_WIDTH", m_viewport_width);
	sd.AddDefineInt("HDR_HEIGHT", m_viewport_height);

	sd.AddDefineInt("HDR_DISPATCH_X", PASS1_DISPATCH_X);
	sd.AddDefineInt("HDR_DISPATCH_Y", PASS1_DISPATCH_Y);

	sd.AddDefineInt("HDR_WG_SIZE_X", PASS1_WORKGROUP_SIZE_X);
	sd.AddDefineInt("HDR_WG_SIZE_Y", PASS1_WORKGROUP_SIZE_Y);

	// per thread uv step
	sd.AddDefineFloat("HDR_THREAD_STEP_X", float(m_viewport_width) / float(m_sample_per_thread_x * PASS1_DISPATCH_X * PASS1_WORKGROUP_SIZE_X));
	sd.AddDefineFloat("HDR_THREAD_STEP_Y", float(m_viewport_height) / float(m_sample_per_thread_y * PASS1_DISPATCH_Y * PASS1_WORKGROUP_SIZE_Y));

	// per workgroup uv step
	sd.AddDefineFloat("HDR_WG_STEP_X", float(m_viewport_width) / PASS1_DISPATCH_X);
	sd.AddDefineFloat("HDR_WG_STEP_Y", float(m_viewport_height) / PASS1_DISPATCH_Y);
	
	// per sampling iteration uv step
	sd.AddDefineFloat("HDR_ITERATION_STEP_X", float(m_viewport_width) / float(PASS1_DISPATCH_X * m_sample_per_thread_x));
	sd.AddDefineFloat("HDR_ITERATION_STEP_Y", float(m_viewport_height) / float(PASS1_DISPATCH_Y * m_sample_per_thread_y));

	sd.AddDefineInt("HDR_SAMPLE_COUNT_X", m_sample_per_thread_x);
	sd.AddDefineInt("HDR_SAMPLE_COUNT_Y", m_sample_per_thread_y);

	sd.AddDefineInt("THREAD_COUNT", PASS1_WORKGROUP_SIZE_X * PASS1_WORKGROUP_SIZE_Y);

#if DEBUG_HDR_UVS
	sd.AddDefineInt("DEBUG_HDR_UVS", 1);
#endif

	m_reduction_pass1_shader = ShaderFactory::GetInstance()->AddDescriptor(sd);
}


void ComputeHDR::ReloadReductionPass1()
{
	nglDeletePipelines(m_reduction_pass1);
	LoadReductionPass1();
}


#if DEBUG_HDR_UVS
void ComputeHDR::InitDebug()
{
	if (m_uv_test_texture == 0)
	{
		NGL_texture_descriptor desc;
		desc.m_name = "uv_test_texture";
		desc.m_type = NGL_TEXTURE_2D;
		desc.m_filter = NGL_NEAREST;
		desc.m_wrap_mode = NGL_CLAMP_ALL;
		desc.m_size[0] = m_viewport_width;
		desc.m_size[1] = m_viewport_height;
		desc.m_format = NGL_R8_G8_B8_A8_UNORM;
		desc.m_unordered_access = true;
		desc.m_is_renderable = true;
		desc.SetAllClearValue(0.0f);
		nglGenTexture(m_uv_test_texture, desc, nullptr);
		Transitions::Get().Register(m_uv_test_texture, desc);
	}

	if (m_uv_test_texture_clear_job == 0)
	{
		NGL_job_descriptor jd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_uv_test_texture;
			ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			jd.m_attachments.push_back(ad);
		}
		{
			NGL_subpass sp;
			sp.m_name = "uv_test_texture_clear_job";
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
			jd.m_subpasses.push_back(sp);
		}

		m_uv_test_texture_clear_job = nglGenJob(jd);
	}
}
#endif




template<bool is_writing, size_t element_size>
inline size_t SerializeElementPtr(KCL::File &file, void *member, size_t element_index)
{
	assert(false);
	return 0;
}


template<>
inline size_t SerializeElementPtr<true, 4>(KCL::File &file, void *member, size_t element_index)
{
	uint32_t &ivalue = ((uint32_t*)member)[element_index];
	uint8_t c[4];

	c[0] = ivalue & 255;
	c[1] = (ivalue >> 8) & 255;
	c[2] = (ivalue >> 16) & 255;
	c[3] = (ivalue >> 24) & 255;

	return file.Write(c, 4, 1);
}


template<>
inline size_t SerializeElementPtr<false, 4>(KCL::File &file, void *member, size_t element_index)
{
	uint32_t &ivalue = ((uint32_t*)member)[element_index];
	uint8_t c[4];

	size_t ret = file.Read(c, 4, 1);

	ivalue = (uint32_t)c[0]
		+ ((uint32_t)c[1] << 8)
		+ ((uint32_t)c[2] << 16)
		+ ((uint32_t)c[3] << 24);

	return ret;
}


template<bool is_writing, size_t element_size, typename T>
inline size_t SerializeElement(KCL::File &file, T &member, size_t element_index = 0)
{
	return SerializeElementPtr<is_writing, element_size>(file, (void*)&member, element_index);
}


void ComputeHDR::SaveLuminanceValues(const std::string &filename)
{
	if (!m_hdr_save_luminance_values)
	{
		return;
	}

	INFO("Saving luminance values to %s", filename.c_str());

	KCL::File file(filename, KCL::Write, KCL::RWDir, true);

	SerializeElement<true, 4>(file, m_delta_time);

	std::map<KCL::uint32, float>::const_iterator it;
	for (it = m_luminance_map.begin(); it != m_luminance_map.end(); it++)
	{
		SerializeElement<true, 4>(file, it->first);
		SerializeElement<true, 4>(file, it->second);
	}

	file.Close();
}


void ComputeHDR::LoadLuminanceValues(const std::string &filename)
{
	if (m_hdr_save_luminance_values)
	{
		INFO("Cannot load predefined luminance values in save mode");
		return;
	}

	KCL::AssetFile file(filename);
	if (!file.Opened())
	{
		INFO("LoadLuminanceValues - Can not open: %s", filename.c_str());
		return;
	}

	if (SerializeElement<false, 4>(file, m_delta_time) != 1)
	{
		INFO("LoadLuminanceValues - Can not parse delta_time from: %s", filename.c_str());
	}

	m_luminance_map.clear();
	while (file.eof() == 0)
	{
		KCL::uint32 frame_time = 0;
		float luminance_value = 0.0f;
		
		if (SerializeElement<false, 4>(file, frame_time) != 1)
		{
			break;
		}

		if (SerializeElement<false, 4>(file, luminance_value) != 1)
		{
			break;
		}

		m_luminance_map[frame_time] = luminance_value;
	}
	file.Close();

	if (m_luminance_map.empty())
	{
		INFO("LoadLuminanceValues - Can not parse: %s", filename.c_str());
	}
	else
	{
		INFO("Loaded predefined luminance values from %s", filename.c_str());
	}
}

std::string GFXB::GetPredefinedLuminanceFilename(const std::string tier_level_name)
{
	return std::string("scene5_luminance_values_") + tier_level_name + std::string(".bin");
}
