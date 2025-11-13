/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <cstdlib>
#include <stdint.h>
#include "ng/timer.h"
#include "testfw.h"
#include "test_base0.h"
#include "test_descriptor.h"
#include "jsonutils.h"
#include <deque>

#ifdef ENABLE_FRAME_CAPTURE
#include "../framecapture/capturewrapper.h"
#endif

#include <string>

namespace tfw
{
	class Descriptor;
}
class QualityMatch;
class StatusReport;

class GFXBench : public tfw::TestBase {
public:
	GFXBench();
	virtual ~GFXBench();

	//virtual bool init();
    virtual std::string result() { return result_; }
	void run();
	bool step();
	virtual float progress();
	virtual void onCancel();
	virtual void cancel();
	virtual bool terminate();

protected:
	bool stepNoCtx();
	bool parseTestDescriptor(const tfw::Descriptor &desc, TestDescriptor &td);
	bool parseConfig(const std::string &config, TestDescriptor &td);
	void storeResults();
	void processMessages();
	bool getScreenshotName(char* const screenshotName);
	bool InitializeTestEnvironment();
	bool InitializeTest();
	void PrintDefaultFBOInfo();

	std::deque<int> GPUfreqs;
	bool gpu_logging_valid;
	std::deque<float> CPUtemps;
	bool temp_logging_valid;
	std::deque<int> CPUfreqs;
	int core_count;
	std::deque<int> renderTimesMin;
	std::deque<int> renderTimesMax;
	std::deque<int> renderTimesAvg;
	int renderTimeMin;
	int renderTimeMax;
	int renderTimesAccum;
	int renderFramesAccum;
	std::deque<int> queryTimes;

	TestDescriptor initial_descriptor_;
    std::string test_id_;
	uint32_t frames_;
    std::string tfw_descriptor_string_;
	std::string data_;
	std::string datarw_;
	int width_;
	int height_;
	float progress_;
	long elapsedTime_;
	GLB::TestBase0 *glb_;
	int m_fps_window;
	bool done;
	bool paused;
	int animationTimeMilliseconds;
	int lastTimeMilliseconds;
	int m_min_frame_time;
	bool m_frame_time_limit_enabled;
	ng::cpu_timer timer;
	ng::cpu_timer armedTimer;
	int fps_time;
	int fps_frames;
	bool firstStep;

	bool user_input_enabled ;
    std::string m_vendor;
	std::string m_renderer;
	std::string m_graphics_version;
    ng::cpu_timer loadTimer;
    
    GlobalTestEnvironment *m_gte ;

	int m_vsync_frames_accum_vsync;
	int m_vsync_time_accum;
	int m_vsync_frames_accum;
	int m_vsync_limit_count;

	int lastQueryTime;
	int m_query_interval_ms;
    std::string result_;
	std::string result_nocharts_;
	std::vector < QualityMatch* > m_qm_tests;

	bool m_is_battery_test;
	bool m_is_report_mode;
	int m_loop_count;
	std::string m_selected_device_id;

private:
	StatusReport *m_status_report;
};

template<class T>
class GFXBenchA: public GFXBench {
public:
	virtual bool init()
	{ 
		bool inited = InitializeTestEnvironment();
		if (inited == false)
		{
			return false;
		}
		glb_ = new T(m_gte);
#if defined ENABLE_FRAME_CAPTURE
		if (FrameCapture::isCapturing(m_gte->GetTestDescriptor()))
		{
			glb_ = new FrameCapture(m_gte->GetTestDescriptor(), glb_);
		}
#endif
		return InitializeTest();
	}
};

#ifdef USE_METAL
#include "metal_wrapper.h"
#endif

/*extern "C" APICALL*/ tfw::TestBase *create_test_by_name(const char* name);
