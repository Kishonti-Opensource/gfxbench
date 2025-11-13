/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "test_descriptor.h"
#include "ng/log.h"

#include <algorithm>

using namespace GLB;

ScreenMode GLB::IntToScreenMode(const int value)
{
	switch(value)
	{
	case 0: return SMode_Onscreen;
	case 1: return SMode_Offscreen;

	default: return SMode_Onscreen;
	}
}


ScreenMode TestDescriptor::GetScreenMode() const
{
	return m_screen_mode;
}


bool TestDescriptor::parseTestDescriptor(const tfw::Descriptor &d)
{
	bool ret = false;
	try
	{
		m_name = d.testId();
		m_fps_window = (int)d.rawConfign("fps_log_window", 2000);
		m_brightness = static_cast<float>(d.rawConfign("brightness", -1));
		m_is_debug_battery = d.rawConfigb("debug_battery", false);
		m_color_bpp = static_cast<int>(d.rawConfign("color_bpp"));
		m_depth_bpp = static_cast<int>(d.rawConfign("depth_bpp"));
		m_is_endless = d.rawConfigb("endless", false);
		m_engine = d.rawConfigs("engine");
		m_fps_limit = static_cast<float>(d.rawConfign("fps_limit", -1));
		m_frame_step_time = static_cast<int>(d.rawConfign("frame_step_time", -1));
		m_fsaa = static_cast<int>(d.rawConfign("fsaa", 0));
		m_max_rendered_frames = static_cast<int>(d.rawConfign("max_rendered_frames", -1));
		m_min_ram_required = static_cast<int>(d.rawConfign("min_ram_required", -1));
		m_play_time = static_cast<int>(d.rawConfign("play_time"));
		m_force_highp = d.rawConfigb("force_highp", false);
		m_tessellation_enabled = d.rawConfigb("tessellation_enabled", true);
		m_scenefile = d.rawConfigs("scenefile", "");
		m_screen_mode = (GLB::ScreenMode)(int)d.rawConfign("screenmode", SMode_Onscreen);
		m_single_frame = static_cast<int>(d.rawConfign("single_frame", -1));
		m_start_animation_time = static_cast<int>(d.rawConfign("start_animation_time", -1));
		m_stencil_bpp = static_cast<int>(d.rawConfign("stencil_bpp"));
		m_texture_type = d.rawConfigs("texture_type");
		m_zprepass = d.rawConfigb("zprepass", false);
		m_test_width = static_cast<int>(d.rawConfign("test_width", 1920));
		m_test_height = static_cast<int>(d.rawConfign("test_height", 1080));

		m_qm_metric = d.rawConfigs("qm_metric", "");
		m_qm_reference_filename = d.rawConfigs("qm_reference_filename", "");
		m_qm_save_image = d.rawConfigb("qm_save_image", false);

		m_loop_count = static_cast<unsigned int>(d.rawConfign("loop_count", 0));
		m_battery_charge_drop = static_cast<int>(d.rawConfign("battery_charge_drop", -1));

		m_charts_query_frequency = (int)d.rawConfign("query_interval_ms", 1000);

		m_virtual_resolution = d.rawConfigb("virtual_resolution", false);
		m_screenshot_frames_list = d.rawConfigs("screenshot_frames", "");
		ParseFrameList(m_screenshot_frames_list, m_screenshot_frames);
		m_disabledRenderBits = static_cast<int>(d.rawConfign("disabled_render_bits", 0));
		SetWarmupFrames(static_cast<int>(d.rawConfign("warmup_frames", 0)));
		m_adaptation_mode = static_cast<int>(d.rawConfign("adaptation_mode", 0));
		m_workgroup_sizes = d.rawConfigs("wg_sizes", "");

		m_enable_validation = d.rawConfigb("validation_layer", false);

		std::string particle_save_frames_list = d.rawConfigs("particle_save_frames", "");
		ParseFrameList(particle_save_frames_list, m_particle_save_frames);

		m_gi_use_texture_sh_atlas = d.rawConfigb("gi_use_texture_sh_atlas", false);

		m_hdr_save_luminance_values = d.rawConfigb("hdr_save_luminance_values", false);

		max_offscreen_tile = static_cast<uint32_t>(d.rawConfign("max_offscreen_tile", 100000000));
        m_metal_macos_use_subpass = d.rawConfigb("metal_macos_use_subpass", false);

		ret = true;
	}
	catch (const std::exception &e)
	{
		NGLOG_ERROR("failed to parse descriptor: %s", e.what());
	}
	return ret;
}


const char * ScreenModeToCStr(const ScreenMode mode)
{
	switch(mode)
	{
	case SMode_Onscreen: return "Onscreen";
	case SMode_Offscreen: return "Offscreen";

	default: return "";
	}
}


/*!defaultSort used by SetScreenshotFrames
*/
static bool defaultSort(int u, int v)
{
	return u < v;
}


TestDescriptor& TestDescriptor::ParseFrameList(const std::string& frames, std::vector<int> &frames_array)
{
	frames_array.clear();

	char *p = (char*)frames.c_str();

	char* retVal = strtok(p, ",");
	if(retVal)
	{
		while(p != 0)
		{
			int result2;
			int result;

			if(strchr(p, '-') != 0)
			{
				result = atoi( p);
				result2 = atoi( strchr(p, '-') + 1);
			}
			else
			{
				result = result2 = atoi( p);
			}

			for(int i = result;i < result2 + 1; i++)
			{
				frames_array.push_back(i);
			}
			p = strtok(0, ",");
		}
	}

	std::stable_sort(frames_array.begin(), frames_array.end(), defaultSort);

	return *this;
}


TestDescriptor& TestDescriptor::SetWarmupFrames(const int warmup_frames)
{
	 m_warmup_frames = warmup_frames < 0 ? 0 : warmup_frames;
	 return *this;
}


TestDescriptor::TestDescriptor()
{
	SetDefaults();
}


const int TestDescriptor::GetRedColorComponent() const
{
	if(m_color_bpp == -1)
	{
		return m_color_bpp;
	}
	return m_color_bpp==16 ? 5 : 8;
}


const int TestDescriptor::GetGreenColorComponent() const
{
	if(m_color_bpp == -1)
	{
		return m_color_bpp;
	}
	return m_color_bpp==16 ? 6 : 8;
}


const int TestDescriptor::GetBlueColorComponent() const
{
	if(m_color_bpp == -1)
	{
		return m_color_bpp;
	}
	return m_color_bpp==16 ? 5 : 8;
}


TestDescriptor& TestDescriptor::SetDefaults()
{
	m_screenshot_frames.clear();
	m_name.clear();
	m_engine.clear();
	m_is_endless = false;
	m_is_fullscreen = false;
	m_min_ram_required = -1;
	m_test_height = -1;
	m_test_width = -1;
	m_virtual_resolution = false;
	m_color_bpp = -1;
	m_depth_bpp = -1;
	m_fsaa = 0;
	m_frame_step_time = -1;
	m_play_time = -1;
	m_brightness = -1.0f;
	m_fps_limit = -1.0f;
	m_stencil_bpp = -1;
	m_single_frame = -1;
	m_max_rendered_frames = -1;
	m_start_animation_time = 0;
	m_texture_type.clear();
	m_screen_mode = SMode_Onscreen;
	m_hybrid_refresh_msec = 40;
	m_zprepass = false;
	m_disabledRenderBits = 0;
	m_qm_save_image = false;
	m_warmup_frames = 0;
	m_loop_count = 0;
	m_battery_charge_drop = 0;
	m_charts_query_frequency = 0;
	m_enable_validation = false;
	return *this;
}


bool TestDescriptor::IsFixedTime() const
{
	return m_frame_step_time > 0;
}


bool TestDescriptor::IsSingleFrame() const
{
	return m_single_frame >= 0;
}

//TODO it should add the render_api support
const std::string &TestDescriptor::GetTextureType() const
{
	return m_texture_type;
}

//TODO it should add the render_api support
const bool TestDescriptor::ValidateTextureType() const
{
	return true;
}
