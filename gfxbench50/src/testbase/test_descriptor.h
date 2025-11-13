/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TEST_DESCRIPTOR_H
#define TEST_DESCRIPTOR_H

#include "schemas/descriptors.h"

#include <string>
#include <list>
#include <map>
#include <vector>
#include <cstring>

namespace GLB
{

enum ScreenMode {SMode_Onscreen = 0, SMode_Offscreen};

ScreenMode IntToScreenMode(const int value);
const char * ScreenModeToCStr(const ScreenMode mode);

/*!Every member is public! Don't write getter/setter only really need!
*/
class TestDescriptor
{
public:
	TestDescriptor();

	TestDescriptor& SetDefaults();

	bool parseTestDescriptor(const tfw::Descriptor &d);

	TestDescriptor& SetWidth(const unsigned int width);
	TestDescriptor& SetHeight(const unsigned int height);
	TestDescriptor& SetHybridRefreshMillisec(const int hybrid_refresh_msec);
	TestDescriptor& ParseFrameList(const std::string& frames, std::vector<int> &frames_array);
	TestDescriptor& SetExtraData(const std::string& extradata);
	TestDescriptor& SetWarmupFrames(const int warmup_frames);

	const std::string &GetTextureType() const;
	const bool ValidateTextureType() const;
	const int GetRedColorComponent() const;
	const int GetGreenColorComponent() const;
	const int GetBlueColorComponent() const;
	const int GetMinRamRequired() const;
	bool IsFixedTime() const;
	bool IsSingleFrame() const;
	ScreenMode GetScreenMode() const;
	int GetTestWidth() const;
	int GetTestHeight() const;

	std::string m_name;
	std::string m_engine;
	std::string m_scenefile;
	std::string m_screenshot_frames_list;


	int m_fps_window;
	bool m_is_endless;
	bool m_is_fullscreen;
	bool m_is_debug_battery;
	bool m_force_highp;
	bool m_tessellation_enabled;
	int m_test_width;
	int m_test_height;
	bool m_virtual_resolution;

	unsigned int m_loop_count;
	int m_battery_charge_drop;
	int m_min_ram_required;
	int m_color_bpp;
	int m_depth_bpp;
	int m_fsaa;
	int m_frame_step_time;
	int m_play_time;
	float m_brightness;
	float m_fps_limit;
	int m_stencil_bpp;
	int m_single_frame;
	int m_max_rendered_frames; //must be able to represent-1, means not set
	int m_start_animation_time; //must be able to represent-1, means not set
	int m_hybrid_refresh_msec;
	bool m_zprepass;
	std::vector<int> m_screenshot_frames;
	int m_disabledRenderBits; //combination of flags - the selected render passes will be disabled, defaults to 0
	std::string m_texture_type;
	int m_warmup_frames;
	int m_adaptation_mode;
	int m_charts_query_frequency;

	bool m_qm_save_image;
	std::string m_qm_reference_filename;
	std::string m_qm_metric;

	std::string m_workgroup_sizes;

	bool m_enable_validation;
    bool m_metal_macos_use_subpass;
	uint32_t max_offscreen_tile;

	std::vector<int> m_particle_save_frames;
	bool m_gi_use_texture_sh_atlas;
	bool m_hdr_save_luminance_values;

private:
	ScreenMode m_screen_mode;
};


inline bool operator!=( const TestDescriptor &a, const TestDescriptor &b)
{
	return memcmp(&a, &b, sizeof(TestDescriptor)) != 0;
}

}//namespace GLB

#endif
