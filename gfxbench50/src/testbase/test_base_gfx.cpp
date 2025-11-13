/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "test_base_gfx.h"
#include "test_component.h"
#include "time_component.h"
#include "input_component.h"
#include "statistics_component.h"
#include "screenshot_component.h"

#include <result.h>
#include <graphics/graphicscontext.h>

#include <kcl_os.h>
#include <kcl_io.h>
#include <kcl_camera2.h>
#include <kcl_image.h>
#include <ng/timer.h>
#include <ng/log.h>
#include <sstream>
#include <cstdarg>

#include "ngl.h"
#include "gfxb_barrier.h"

#ifdef _MSC_VER
#include <Windows.h>
#include "graphics/windowsvulkangraphicscontext.h"

//#define GLB_USE_SYSTEM_MESSAGER 1
#endif

#ifdef ANDROID
#include "graphics/eglgraphicscontext.h" // TODO: This is just to get the selected EGL configuration, GFXBench normally should not include anything platform-specific.
#include <graphics/AndroidVulkanGraphicsContext.h>
#endif

#ifdef __linux__

#ifdef DISPLAY_PROTOCOL_WAYLAND
#include <graphics/wlgraphicscontext.h>
#endif

#ifdef DISPLAY_PROTOCOL_XCB
#include <graphics/xcbgraphicscontext.h>
#endif

#endif//__linux__

#ifdef ENABLE_METAL_IMPLEMENTATION
#include "context_ngl_adapter_metal.h"
#include <TargetConditionals.h>
#endif

#if defined(ENABLE_D3D11_IMPLEMENTATION) || defined(ENABLE_D3D12_IMPLEMENTATION)
#include "graphics/dxwin32graphicscontext.h"
#endif

#ifdef WITH_OVR_SDK
#include <graphics/ovrgraphicscontext.h>
#endif

#ifdef DISPLAY_PROTOCOL_SCREEN
#include <graphics/qnxgraphicscontext.h>
#endif//DISPLAY_PROTOCOL_SCREEN

#ifndef TEST_COMPILE_ACHITECTURE
#define TEST_COMPILE_ACHITECTURE "undefined"
#endif

#ifndef GLB_USE_SYSTEM_MESSAGER
#define GLB_USE_SYSTEM_MESSAGER 0
#endif

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)


#include "system_messager.h"

using namespace GLB;

#define FOR_EACH_COMPONENTS(m) \
	for (size_t i = 0; i < m_test_components.size(); ++i) \
		m_test_components[i]->m;

TestBaseGFX::TestBaseGFX()
{
	m_runtime_error = KCL::KCL_TESTERROR_NOERROR;
	m_load_time = 0;

	m_time_component = nullptr;
    m_input_component = nullptr;
    m_vsync_component = nullptr;
    m_charts_component = nullptr;
	m_screen_manager_component = nullptr;

	m_native_width = 0;
	m_native_height = 0;

	m_animation_time = 0;
	m_frames = 0;
	m_elapsed_time = 0.0;
	m_score = 0.0;
	m_uom = UOM_FRAMES;

	m_ngl_initialized = false;
	m_done = false;

	m_buffers_per_frame = 0;
	m_prerendered_frame_count = 0;

	m_no_result = false;
	m_normalized_score = true;

	m_system_messager = nullptr;
}


TestBaseGFX::~TestBaseGFX()
{
	terminate();
}


bool TestBaseGFX::terminate()
{
	for (size_t i = 0; i < m_test_components.size(); i++)
	{
		delete m_test_components[i];
	}
	m_test_components.clear();

	m_time_component = nullptr;
	m_input_component = nullptr;
	m_screen_manager_component = nullptr;

	delete m_system_messager;
	m_system_messager = nullptr;

	// Release NGL
	if (m_ngl_initialized)
	{
		nglDestroyContext();
		m_ngl_initialized = false;
	}

	// Release KCL
	KCL::Release();
	KCL::OS::DestroyI(&KCL::g_os);
	KCL::File::ClearScenePath();

	GFXB::Transitions::Release();

	return true;
}

bool TestBaseGFX::init()
{
	ng::cpu_timer load_timer = ng::cpu_timer(true);

#ifdef WITH_OVR_SDK
	OVRGraphicsContext *ovr_graphics_context = (OVRGraphicsContext*)ctx_;
	ovr_graphics_context->getOvrModeParams().Java.Vm->AttachCurrentThread(&ovr_graphics_context->getOvrModeParams().Java.Env, NULL);
#endif

	const std::string &config_str = config();

	std::string error;
	tfw::Descriptor desc;
	if (!tfw::Descriptor::fromJsonString(config_str, &desc, &error))
	{
		NGLOG_ERROR("Failed to parse config: %s", error);
		return false;
	}

	if (ParseConfig(desc) == false)
	{
		SetRuntimeError(KCL::KCL_TESTERROR_UNKNOWNERROR);
		return false;
	}

#if GLB_USE_SYSTEM_MESSAGER
	m_system_messager = new SystemMessager(m_test_id.c_str());
	m_system_messager->Send(SystemMessager::LOAD_BEGIN);
#endif

	// Initialize KCL OS
	KCL::g_os = KCL::OS::CreateI("");
	KCL::File::ClearScenePath();
	KCL::File::SetDataPath(m_datapath);
	KCL::File::SetRWDataPath(m_datapath_rw);

	INFO("Architecture: %s", TEST_COMPILE_ACHITECTURE);

	// Initialize Render API
	m_ngl_initialized = InitRenderAPI();
	if (m_ngl_initialized == false)
	{
		INFO("Can not initialize NGL!");
		SetRuntimeError(KCL::KCL_TESTERROR_INIT_RENDER_API);
		return false;
	}

	// Print system render target bit depths
	{
		int color_bits = nglGetInteger(NGL_SWAPCHAIN_COLOR_FORMAT);
		int depth_bits = nglGetInteger(NGL_SWAPCHAIN_DEPTH_FORMAT);
		std::string color_string = nglGetString(NGL_SWAPCHAIN_COLOR_FORMAT);
		std::string depth_string = nglGetString(NGL_SWAPCHAIN_DEPTH_FORMAT);

		INFO("System render target format: color %d (%s) - depth %d (%s)", color_bits, color_string.c_str(), depth_bits, depth_string.c_str());
	}

	// Initialize
	KCL::Initialize(nglGetInteger(NGL_DEPTH_MODE) == NGL_ZERO_TO_ONE);
	GFXB::Transitions::Release();

	// Initialize test components
	m_time_component = new TimeComponent(this);
	m_test_components.push_back(m_time_component);

    m_vsync_component = new VSyncComponent(this);
    m_test_components.push_back(m_vsync_component);

    m_charts_component = new ChartsComponent(this);
    m_test_components.push_back(m_charts_component);

	m_input_component = new InputComponent(this);
	m_input_component->SetQueue(msgQueue_);
	m_test_components.push_back( m_input_component );

	// must be added before screen manager component
	m_test_components.push_back(new ScreenshotComponent(this));

	if (StatisticsComponent::STATISTICS_MODE)
	{
		m_test_components.push_back(new StatisticsComponent(this, StatisticsComponent::STATISTICS_MODE));
	}

	// must be added after screen manager component
	m_screen_manager_component = ScreenManagerComponent::Create( this, m_test_descriptor, m_native_width, m_native_height );
	m_screen_manager_component->GetTestSize( m_test_width, m_test_height );
	m_test_components.push_back( m_screen_manager_component );

	for( size_t i = 0; i < m_test_components.size(); i++ )
	{
		if (m_test_components[i]->Init() == false)
		{
			NGLOG_ERROR("Can not initialize %s", m_test_components[i]->GetName());
			SetRuntimeError(KCL::KCL_TESTERROR_UNKNOWNERROR);
			return false;
		}
	}

	// Initialize the test
	INFO("Init test: %s", m_test_id.c_str());
	TestBaseGFX::setName(m_test_id.c_str());

	KCL::KCL_Status status = SetNextCommandBuffers();
	if (status != KCL::KCL_TESTERROR_NOERROR)
	{
		SetRuntimeError(status);
		return false;
	}

	status = Init();
	if (status != KCL::KCL_TESTERROR_NOERROR)
	{
		SetRuntimeError(status);
		return false;
	}

	m_load_time = KCL::uint32(load_timer.elapsed().wall * 1000.0);

	INFO("Test loaded in %d second(s)", m_load_time / 1000);

	// Execute warmup
	m_screen_manager_component->Warmup();
	Warmup();
	m_screen_manager_component->ClearScreen();

	nglFinish();
	INFO("Warm up ready");

	nglCustomAction(0, NGL_CUSTOM_ACTION_SWAPBUFFERS);
	require(ctx_->swapBuffers());

#if GLB_USE_SYSTEM_MESSAGER
	m_system_messager->Send(SystemMessager::LOAD_END);
#endif

	KCL::uint32 warmup_time = KCL::uint32(load_timer.elapsed().wall * 1000.0);
	INFO("Warm up executed in %d second(s)", (warmup_time - m_load_time) / 1000);

	return true;
}


void TestBaseGFX::run()
{
	require(ctx_->makeCurrent());

	m_input_component->BeginFrame(); // handle leftover events from init

#if GLB_USE_SYSTEM_MESSAGER
	m_system_messager->Send(SystemMessager::TEST_BEGIN);
#endif

	FOR_EACH_COMPONENTS(BeginTest());

	while (!m_done)
	{
		FOR_EACH_COMPONENTS(BeginFrame());

		SetNextCommandBuffers();

#ifdef WITH_OVR_SDK
#ifdef ANDROID
		//inserted here to make sure proper sync occurs between ovr and our context
		//kinda magic...
		require( ctx_->makeCurrent() );
#endif
#endif

		Animate();

		Render();

		KCL::uint32 last_command_buffer = GetLastCommandBuffer();

		m_screen_manager_component->SetCommandBuffer(last_command_buffer);
		m_screen_manager_component->FinishRender();

		nglEndCommandBuffer(last_command_buffer);
		nglSubmitCommandBuffer(last_command_buffer);

		FOR_EACH_COMPONENTS(AfterRender());

		if (m_screen_manager_component->NeedSwapbuffer())
		{
			nglCustomAction(0, NGL_CUSTOM_ACTION_SWAPBUFFERS);
			require(ctx_->swapBuffers());
		}

		m_frames++;

		FOR_EACH_COMPONENTS(EndFrame());
	}

	SetNextCommandBuffers();
	m_screen_manager_component->SetCommandBuffer(GetLastCommandBuffer());
	m_screen_manager_component->FinishTest();
	if (m_screen_manager_component->NeedSwapbuffer())
	{
		nglCustomAction(0, NGL_CUSTOM_ACTION_SWAPBUFFERS);
		require(ctx_->swapBuffers());
	}

	m_screen_manager_component->WaitFinish(ctx_->type() == GraphicsContext::OPENGL || ctx_->type() == GraphicsContext::GLES);

	m_time_component->StopTimer();

#if GLB_USE_SYSTEM_MESSAGER
	m_system_messager->Send(SystemMessager::TEST_END);
#endif

	CreateResults();

	ctx_->detachThread();
}


void TestBaseGFX::onCancel()
{
	FinishTest();
}


std::string TestBaseGFX::result()
{
	return m_result;
}


void TestBaseGFX::setMessageQueue(tfw::MessageQueue *msg_queue)
{
	TestBase::setMessageQueue(msg_queue);

	InputComponent *input_component = dynamic_cast<InputComponent*>(GetTestComponent(InputComponent::NAME));
	if (input_component)
	{
		input_component->SetQueue(msg_queue);
	}
}


std::string TestBaseGFX::CreateResults(const tfw::ResultGroup* result_group)
{
	if (result_group != nullptr && result_group->results().size() > 0)
	{
		m_tfw_result_group = *result_group;
        m_result = result_group->toJsonString();
        return m_result;
	}

	// caching testfw result cause no result on specific platform
	//if (m_tfw_result_group.results().size() > 0)
	//{
	//	return m_tfw_result_group.toJsonString();
	//}
	m_tfw_result_group = tfw::ResultGroup();

	tfw::Result test_result;

#ifdef PRODUCT_VERSION
    std::stringstream ss;
    test_result.setBenchmarkVersion(STRINGIFY(PRODUCT_VERSION));
#endif
	test_result.setTestId(m_test_id);
    test_result.setResultId(m_test_descriptor.m_name);

	tfw::Result::Status test_status = tfw::Result::OK;
	if (isCancelled())
	{
		test_status = tfw::Result::CANCELLED;
	}
	else if (m_runtime_error != KCL::KCL_TESTERROR_NOERROR)
	{
		test_status = tfw::Result::FAILED;
	}
	test_result.setStatus(test_status);
	test_result.setErrorString(KCL::KCL_Status_To_Cstr(m_runtime_error));

#ifdef ANDROID
	test_result.gfxResult().setEglConfigId(((EGLGraphicsContext*)ctx_)->selectedConfig());
#else
	test_result.gfxResult().setEglConfigId(-1);//can not interpreted on desktop
#endif

	if (m_ngl_initialized)
	{
		test_result.gfxResult().setVendor(nglGetString(NGL_VENDOR));
		test_result.gfxResult().setRenderer(nglGetString(NGL_RENDERER));
		test_result.gfxResult().setGraphicsVersion(nglGetString(NGL_RENDERER));
	}

	test_result.gfxResult().setSurfaceWidth(m_test_width);
	test_result.gfxResult().setSurfaceHeight(m_test_height);

	test_result.setResultId(m_test_id);

	{
		tfw::Config config = test_result.descriptor().env().graphics().config();

		int color_bits = nglGetInteger(NGL_SWAPCHAIN_COLOR_FORMAT);
		int depth_bits = nglGetInteger(NGL_SWAPCHAIN_DEPTH_FORMAT);

		if (color_bits > 0)
		{
			if (color_bits == 8888)
			{
				config.setRed(8);
				config.setGreen(8);
				config.setBlue(8);
				config.setAlpha(8);
			}
			else if (color_bits == 888)
			{
				config.setRed(8);
				config.setGreen(8);
				config.setBlue(8);
				config.setAlpha(0);
			}
			else
			{
				config.setRed(0);
				config.setGreen(0);
				config.setBlue(0);
				config.setAlpha(0);
			}
		}
		else
		{
			config.setRed(-1);
			config.setGreen(-1);
			config.setBlue(-1);
			config.setAlpha(-1);
		}

		if (depth_bits >= 0)
		{
			config.setDepth(depth_bits);
		}
		else
		{
			config.setDepth(0);
		}

		config.setVsync(false);

		test_result.descriptor().env().graphics().setConfig(config);
	}

    test_result.descriptor().env().graphics().setFullScreen(m_test_descriptor.m_is_fullscreen);
    test_result.descriptor().env().graphics().setDeviceIndex(m_device_index);

#ifndef DISABLE_RESULTS
	if (m_uom == UOM_FRAMES)
	{
		test_result.setUnit("frames");
	}

	if (isCancelled())
	{
		test_result.gfxResult().setFps(-1);
		test_result.gfxResult().setFrameCount(-1);
		test_result.setScore(-1);
        m_tfw_result_group.addResult(test_result);
	}
	else if (m_no_result == false)
	{
		m_elapsed_time = (double)m_time_component->GetElapsedTime();

		double fps = -1.0;

		if (m_elapsed_time > 0.0)
		{
			fps = double(m_frames) / m_elapsed_time * 1000.0;
		}

		test_result.setLoadTime(m_load_time);
		test_result.gfxResult().setFps(fps);
		test_result.gfxResult().setFrameCount(m_frames);

		if (m_normalized_score)
		{
			m_score = GetNormalizedScore();
		}
		else
		{
			m_score = m_frames;
		}

		test_result.setScore(m_score);

		m_tfw_result_group.addResult(test_result);
		for (size_t i = 0; i < m_test_components.size(); ++i)
		{
			m_test_components[i]->CreateResult(m_tfw_result_group);
		}
	}
#endif

    m_result = m_tfw_result_group.toJsonString();
	NGLOG_INFO("%s", m_result);
    return m_result;
}


double GLB::TestBaseGFX::GetScore()
{
	return m_score;
}


bool TestBaseGFX::ParseConfig(tfw::Descriptor &desc)
{
	m_test_id = desc.testId();

	m_selected_device_id = desc.env().graphics().deviceId();


    m_device_index = desc.env().graphics().deviceIndex();

	m_datapath = desc.env().readPath();
	m_datapath_rw = desc.env().writePath();
	m_native_width = desc.env().width();
	m_native_height = desc.env().height();

	m_test_descriptor.m_is_fullscreen = desc.env().graphics().isFullScreen();
	m_test_descriptor.parseTestDescriptor(desc);

	return true;
}


void TestBaseGFX::CreateScreenshot(const char *filename)
{
	if (!filename)
	{
		return;
	}

	KCL::uint32 width;
	KCL::uint32 height;
	std::vector<KCL::uint8> data;

	KCL::uint32 fb_id = m_screen_manager_component->GetBackbuffers()[m_screen_manager_component->GetActiveBackbufferId()];
	bool ret_value = nglGetTextureContent( fb_id, 0, 0, 0, NGL_R8_G8_B8_A8_UNORM, GFXB::Transitions::Get().GetTextureState( fb_id ), width, height, data );

	if (ret_value == false || width == 0 || height == 0 || data.empty())
	{
		NGLOG_ERROR("Could not capture screen to %s", filename);
		return;
	}

	std::vector<KCL::uint8> data_rgb;
	data_rgb.resize( width*height * 3 );
	for( unsigned c = 0; c < data.size()/4; ++c )
	{
		data_rgb[c * 3 + 0] = data[c * 4 + 0];
		data_rgb[c * 3 + 1] = data[c * 4 + 1];
		data_rgb[c * 3 + 2] = data[c * 4 + 2];
	}

	KCL::Image image(width, height, KCL::Image_RGB888);
	memcpy(image.getData(), data_rgb.data(), image.getDataSize());

	std::stringstream sstream;
	sstream << KCL::File::GetDataRWPath();
	sstream << filename;
	sstream << ".tga";
	std::string tga_file = sstream.str();
	if (image.saveTga(tga_file.c_str(), true) == false)
	{
		NGLOG_ERROR("Can not save screenshot: %s", tga_file.c_str());
	}
	else
	{
		NGLOG_INFO("Screenshot captured: %s", tga_file.c_str());
	}
}


KCL::KCL_Status TestBaseGFX::SetNextCommandBuffers()
{
	if (m_command_buffers.empty())
	{
		GetCommandBufferConfiguration(m_buffers_per_frame, m_prerendered_frame_count);

		if (m_buffers_per_frame == 0 || m_prerendered_frame_count == 0)
		{
			INFO("Illegal command buffer configuration: %d x %d", m_buffers_per_frame, m_prerendered_frame_count);
			return KCL::KCL_TESTERROR_UNKNOWNERROR;
		}

		m_command_buffers.resize(m_buffers_per_frame);
		for (KCL::uint32 i = 0; i < m_buffers_per_frame; i++)
		{
			m_command_buffers[i] = i;
		}
	}
	else
	{
		const KCL::uint32 m = m_buffers_per_frame * m_prerendered_frame_count;
		for (KCL::uint32 i = 0; i < m_buffers_per_frame; i++)
		{
			m_command_buffers[i] = (m_command_buffers[i] + m_buffers_per_frame) % m;
		}
	}

	SetCommandBuffers(m_command_buffers);

	return KCL::KCL_TESTERROR_NOERROR;
}


void TestBaseGFX::FinishTest()
{
	m_done = true;
}


KCL::uint32 TestBaseGFX::GetFrameCount()
{
	return m_frames;
}


double TestBaseGFX::GetNormalizedScore() const
{
	double test_length = m_test_descriptor.m_play_time - m_test_descriptor.m_start_animation_time;
	if (test_length > 0.0 && m_elapsed_time > 0.0)
	{
		return (double)m_frames * test_length / m_elapsed_time;
	}
	return 0.0;
}


void TestBaseGFX::SetAnimationTime(KCL::uint32 time)
{
	m_animation_time = time;
}


KCL::uint32 TestBaseGFX::GetAnimationTime() const
{
	return m_animation_time;
}


KCL::uint32 TestBaseGFX::GetElapsedTime() const
{
	return static_cast<uint32_t>(m_time_component->GetElapsedTime());
}


TestDescriptor &TestBaseGFX::GetTestDescriptor()
{
	return m_test_descriptor;
}


const TestDescriptor &TestBaseGFX::GetTestDescriptor() const
{
	return m_test_descriptor;
}


void TestBaseGFX::SetRuntimeError(KCL::KCL_Status error)
{
	m_runtime_error = error;
}


TestComponent *TestBaseGFX::GetTestComponent(const char *name) const
{
	for (size_t i = 0; i < m_test_components.size(); i++)
	{
		if (m_test_components[i]->GetName() == name)
		{
			return m_test_components[i];
		}
	}
	return nullptr;
}


void TestBaseGFX::GPUApiLogFunction(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int len = vsnprintf( 0, 0, format, args);
	va_end(args);

	va_start(args, format);
	std::vector<char> buffer(len + 1, (char)0);
	vsprintf(buffer.data(), format, args);
	va_end(args);

	// Remove the last tailing new line as KCL log always includes it
	if (buffer.size() >= 2 && buffer[buffer.size() - 2] == '\n')
	{
		buffer.erase(buffer.end() - 2);
	}

	KCL::g_os->Log("%s", buffer.data());
}

bool TestBaseGFX::InitRenderAPI()
{
	require(ctx_->makeCurrent());

	NGL_context_descriptor context_descriptor;

	context_descriptor.m_display_width = m_native_width;
	context_descriptor.m_display_height = m_native_height;

	switch (ctx_->type())
	{
	case GraphicsContext::OPENGL:
		context_descriptor.m_api = NGL_OPENGL;
		context_descriptor.m_major_version = ctx_->versionMajor();
		context_descriptor.m_minor_version = ctx_->versionMinor();
		break;

	case GraphicsContext::GLES:
		context_descriptor.m_api = NGL_OPENGL_ES;
		context_descriptor.m_major_version = ctx_->versionMajor();
		context_descriptor.m_minor_version = ctx_->versionMinor();
		break;

	case GraphicsContext::METAL:
#if TARGET_OS_IPHONE
		context_descriptor.m_api = NGL_METAL_IOS;
#else
		context_descriptor.m_api = NGL_METAL_MACOS;
#endif
		break;

	case GraphicsContext::DIRECTX:
		context_descriptor.m_api = NGL_DIRECT3D_11;
		break;

	case GraphicsContext::DIRECTX12:
		context_descriptor.m_api = NGL_DIRECT3D_12;
		break;

	case GraphicsContext::VULKAN:
		context_descriptor.m_api = NGL_VULKAN;
		break;

	case GraphicsContext::NONE:
		context_descriptor.m_api = NGL_NULL_DRIVER;
		break;

	default:
		return false;
	}

	if (StatisticsComponent::STATISTICS_MODE)
	{
		if (ctx_->type() != GraphicsContext::OPENGL && ctx_->type() != GraphicsContext::VULKAN)
		{
			NGLOG_ERROR("OpenGL or Vulkan desktop context is required for PROFILER RenderAPI!");
			return false;
		}
	}

	context_descriptor.m_system_attachment_is_transfer_source =
		m_test_descriptor.GetScreenMode() == SMode_Onscreen && !m_test_descriptor.m_screenshot_frames.empty();


#ifdef ENABLE_METAL_IMPLEMENTATION
	if ((context_descriptor.m_api == NGL_METAL_IOS) || (context_descriptor.m_api == NGL_METAL_MACOS))
	{
		bool screenshot_mode = !m_test_descriptor.m_screenshot_frames.empty();
		NGL_metal_adapter* metal_adapter = GetNGLMetalAdapter(ctx_, m_selected_device_id, screenshot_mode, m_test_descriptor.m_metal_macos_use_subpass);
		context_descriptor.m_user_data.push_back(metal_adapter);
	}
#endif


#ifdef ENABLE_D3D11_IMPLEMENTATION
	if (context_descriptor.m_api == NGL_DIRECT3D_11)
	{
		DXGraphicsContext* dx_ctx = (DXGraphicsContext*)ctx_;
		context_descriptor.m_user_data.push_back(dx_ctx->getD3D11Device());
		context_descriptor.m_user_data.push_back(dx_ctx->getD3D11DeviceContext());
		context_descriptor.m_user_data.push_back(dx_ctx->getD3D11RenderTargetView());
#if 0
		_resource_manager_platform_interface_d3d11 data;
		DXGraphicsContext* dx_ctx = (DXGraphicsContext*)ctx_;

		data.m_d3dDevice = dx_ctx->getD3D11Device();
		data.m_d3dContext = dx_ctx->getD3D11DeviceContext();
		data.m_d3dRenderTargetView = dx_ctx->getD3D11RenderTargetView();
		data.m_d3dDepthStencilView = dx_ctx->getD3D11DepthStencilView();

		SetResourceManagerInitData(m_rm, data);
#endif
	}
#endif


#if ENABLE_D3D12_IMPLEMENTATION

	if (context_descriptor.m_api == NGL_DIRECT3D_12)
	{
		DxBareGraphicsContext* dx_ctx = (DxBareGraphicsContext*)ctx_;
		context_descriptor.m_user_data.push_back(dx_ctx->hwnd());
	}
#endif

	if (context_descriptor.m_api == NGL_VULKAN)
	{
#ifdef _MSC_VER
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
		context_descriptor.m_user_data.push_back((void*)GetModuleHandle(NULL));
		//context_descriptor.m_user_data.push_back((void*)FindWindow("HWNDGraphicsWindow", "HWND + null"));

		WindowsVulkanGraphicsContext* vk_ctx = (WindowsVulkanGraphicsContext*)ctx_;
		context_descriptor.m_user_data.push_back(vk_ctx->hwnd());
#endif
#elif defined(ANDROID)
		AndroidVulkanGraphicsContext* vk_ctx = (AndroidVulkanGraphicsContext*)ctx_;
		context_descriptor.m_user_data.push_back((void*)vk_ctx->getWindow());
#elif defined(__linux__)
#ifdef DISPLAY_PROTOCOL_WAYLAND
		WLGraphicsContext* wl_ctx = (WLGraphicsContext*)ctx_;
		context_descriptor.m_user_data.push_back((void*)(size_t)wl_ctx->display());
		context_descriptor.m_user_data.push_back((void*)(size_t)wl_ctx->surface());
#elif defined DISPLAY_PROTOCOL_XCB
		XCBGraphicsContext* xcb_ctx = (XCBGraphicsContext*)ctx_;
		context_descriptor.m_user_data.push_back( (void*)xcb_ctx->getConnection() );
		uint32_t tmp = xcb_ctx->getWindow();
		context_descriptor.m_user_data.push_back((void*)(size_t)tmp);
		context_descriptor.m_user_data.push_back((void*)(size_t)xcb_ctx->screen()->root_visual);
#endif
#elif defined DISPLAY_PROTOCOL_SCREEN

		QNXGraphicsContext* qnx_ctx = (QNXGraphicsContext*)ctx_;
		context_descriptor.m_user_data.push_back((void*)(size_t)qnx_ctx->context());
		context_descriptor.m_user_data.push_back((void*)(size_t)qnx_ctx->window());
		context_descriptor.m_user_data.push_back((void*)(size_t)qnx_ctx->format());
		context_descriptor.m_user_data.push_back((void*)(size_t)qnx_ctx->nbuffers());
#endif
	}

	context_descriptor.m_selected_device_id = m_selected_device_id;
	context_descriptor.__logf = GPUApiLogFunction;
	context_descriptor.m_enable_validation = m_test_descriptor.m_enable_validation;

	return nglCreateContext(context_descriptor);
}


GFXB::ShotHandler* GLB::TestBaseGFX::GetCurrentShotIndex()
{
	return nullptr;
}
