/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TEST_BASE_GFX_H
#define TEST_BASE_GFX_H

#include "testfw.h"
#include "test_descriptor.h"
#include "kcl_base.h"
#include "result.h"

#include "components/screen_manager_component.h"
#include "components/statistics_component.h"
#include "components/input_component.h"
#include "components/vsync_component.h"
#include "components/charts_component.h"

#include <string>
#include <vector>

struct NGLStatistic;

namespace GFXB
{
	class ShotHandler;
}

namespace GLB
{

class TestComponent;
class TimeComponent;
class SystemMessager;

class TestBaseGFX : public tfw::TestBase
{
public:
	enum UOM
	{
		UOM_FRAMES = 0,
	};

	TestBaseGFX();
	virtual ~TestBaseGFX();

	virtual bool terminate() override;

	virtual NGLStatistic& GetFrameStatistics() = 0;
	virtual GFXB::ShotHandler* GetCurrentShotIndex();

	void FinishTest();
	void CreateScreenshot(const char *filename);

	void SetRuntimeError(KCL::KCL_Status error);

	void SetAnimationTime(KCL::uint32 time);
	KCL::uint32 GetAnimationTime() const;
	KCL::uint32 GetElapsedTime() const;
	KCL::uint32 GetFrameCount();
	double GetNormalizedScore() const;

	TestDescriptor &GetTestDescriptor();
	const TestDescriptor &GetTestDescriptor() const;

	TestComponent *GetTestComponent(const char *name) const;

	void SetUOM(UOM uom)
	{
		m_uom = uom;
	}

	static void GPUApiLogFunction(const char *format, ...);

	KCL::uint32 GetTestWidth() const
	{
		return m_test_width;
	}
	KCL::uint32 GetTestHeight() const
	{
		return m_test_height;
	}

	bool GetNoResultFlag() const
	{
		return m_no_result;
	}

protected:
	virtual bool InitRenderAPI();

	virtual KCL::KCL_Status Init() = 0;
	virtual KCL::KCL_Status Warmup() = 0;
	virtual void Animate() = 0;
	virtual void Render() = 0;
	virtual void HandleUserInput(GLB::InputComponent *input_component, float frame_time_secs) = 0;

	virtual void GetCommandBufferConfiguration(KCL::uint32 &buffers_in_frame, KCL::uint32 &prerendered_frame_count) = 0;
	virtual void SetCommandBuffers(const std::vector<KCL::uint32> &buffers) = 0;
	virtual KCL::uint32 GetLastCommandBuffer() = 0;

	virtual bool ParseConfig(tfw::Descriptor &desc);

	virtual double GetScore();

	KCL::uint32 GetNativeWidth() const
	{
		return m_native_width;
	}

	KCL::uint32 GetNativeHeight() const
	{
		return m_native_height;
	}

	std::string m_test_id;
	std::string m_selected_device_id;
    int m_device_index = -1;
	std::string m_datapath;
	std::string m_datapath_rw;
	KCL::uint32 m_test_width;
	KCL::uint32 m_test_height;
	bool m_no_result;
	bool m_normalized_score;

	ScreenManagerComponent* m_screen_manager_component;

	std::string CreateResults(const tfw::ResultGroup* result_group = nullptr);
	virtual void run() override;
	virtual std::string result() override;

private:
	TestDescriptor m_test_descriptor;

	std::vector<GLB::TestComponent*> m_test_components;
	TimeComponent *m_time_component;
    VSyncComponent *m_vsync_component;
    ChartsComponent *m_charts_component;

	KCL::uint32 m_load_time;
	KCL::KCL_Status m_runtime_error;

	bool m_ngl_initialized;
	bool m_done;

	KCL::uint32 m_animation_time;
	KCL::uint32 m_frames;
	double m_elapsed_time;
	double m_score;

	KCL::uint32 m_native_width;
	KCL::uint32 m_native_height;

	InputComponent* m_input_component;

	UOM m_uom;

	KCL::uint32 m_buffers_per_frame;
	KCL::uint32 m_prerendered_frame_count;
	std::vector<KCL::uint32> m_command_buffers;

	KCL::KCL_Status SetNextCommandBuffers();


	//tfw::TestBase
	virtual bool init() override;
	virtual void onCancel() override;
	virtual void setMessageQueue(tfw::MessageQueue *msg_queue) override;

	tfw::ResultGroup m_tfw_result_group;
	std::string m_result;

	SystemMessager *m_system_messager;
};

} //namespace GLB

#endif
