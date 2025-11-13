/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TEST_DESCRIPTOR_H
#define TEST_DESCRIPTOR_H

#include <string>
#include <list>
#include <map>
#include <vector>
#include <cstring>


enum ScreenMode {SMode_Onscreen = 0, SMode_Offscreen, SMode_Hybrid};


ScreenMode IntToScreenMode(const int value);


const char * ScreenModeToCStr(const ScreenMode mode);


class TestDescriptor
{
public:
	friend class gui_testdescriptor;
	friend class gui_mainmenu;

public:
	TestDescriptor();

	TestDescriptor& SetDefaults();

	TestDescriptor& SetName(const std::string& name);
	TestDescriptor& SetPlayTime(const int playtime);
	TestDescriptor& SetStartAnimationTime(const int startAnimationTime);
	TestDescriptor& SetColorBpp(const int colorbpp);
	TestDescriptor& SetDepthBpp(const int depthbpp);
	TestDescriptor& SetFrameStepTime(const int timestep);
	TestDescriptor& SetFSAA(const int samples);
	TestDescriptor& SetFullscreen(const int fullscreen);
	TestDescriptor& SetEngine(const std::string& engine);
	TestDescriptor& SetSceneFile(const std::string& scenefile);
	TestDescriptor& SetTextureType(const std::string& textureType);
	TestDescriptor& SetWidth(const unsigned int width);
	TestDescriptor& SetHeight(const unsigned int height);
    TestDescriptor& SetVirtualResolution(const unsigned int virtres);
	TestDescriptor& SetTestWidth(const unsigned int width);
	TestDescriptor& SetTestHeight(const unsigned int height);
	TestDescriptor& SetHybridRefreshMillisec(const int hybrid_refresh_msec);
	TestDescriptor& SetZPrePass(const bool use_zprepass);
	TestDescriptor& SetScreenMode(const ScreenMode screen_mode);
	TestDescriptor& SetMinRamRequired(const int sizeInMB);
	TestDescriptor& SetSingleFrame(const int singleFrame);
	TestDescriptor& SetForceHighp(const bool forceHighp);
    TestDescriptor& SetTessellationEnabled(const bool setTessEnabled);
	TestDescriptor& SetScreenshotFrames(const std::string& frames);
	TestDescriptor& SetParticleBufferSaveFrames(const std::string& frames);
    TestDescriptor& SetDebugBattery(const bool debugbattery);
	TestDescriptor& SetExtraData(const std::string& extradata);
    TestDescriptor& SetWarmupFrames(const int warmup_frames);
    TestDescriptor& SetAdaptationMode(const int adaptation_mode);
	TestDescriptor& SetWorkgroupSizes(const std::string &workgroup_sizes);
	TestDescriptor& SetReportInterval(const int report_interval);
	TestDescriptor& SetReportFilename(const std::string &report_file);

	const std::string GetTextureType() const;
	const bool ValidateTextureType() const;
	const int GetRedColorComponent() const;
	const int GetGreenColorComponent() const;
	const int GetBlueColorComponent() const;
	const int GetMinRamRequired() const;
	bool isFixedTime() const;
	ScreenMode GetScreenMode() const;
	int GetTestWidth() const;
	int GetTestHeight() const;

	std::string m_name;
	std::string m_engine;
	std::string m_scenefile;
	std::string m_screenshot_frames_list;

	bool m_is_endless;
	bool m_is_fullscreen;
    bool m_is_debug_battery;
	bool m_force_highp;
    bool m_tessellation_enabled;
	int m_viewport_width;
	int m_viewport_height;
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
	std::vector<int> m_particle_buffer_save_frames;
    int m_disabledRenderBits; //combination of flags - the selected render passes will be disabled, defaults to 0
	std::string m_texture_type;
    int m_warmup_frames;
    int m_adaptation_mode;
	bool m_never_cancel;
	
	bool m_qm_save_image;
	int qm_compare_frame;
	std::string m_qm_reference_filename;
	std::string m_qm_metric;

	std::string m_workgroup_sizes;

	int m_report_interval;
	std::string m_report_filename;

private:
	ScreenMode m_screen_mode;

	void ParseFrameList(std::vector<int> &frame_list, const std::string &frame_list_str);
};


inline bool operator!=( const TestDescriptor &a, const TestDescriptor &b)
{
	return memcmp(&a, &b, sizeof(TestDescriptor)) != 0;
}

#endif

