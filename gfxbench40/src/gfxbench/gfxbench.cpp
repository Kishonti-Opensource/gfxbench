/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "schemas/descriptors.h"
#include "schemas/result.h"
#include "graphics/graphicscontext.h"
#include "platform.h"

#include "newnotificationmanager.h"
#include "testfw.h"
#include "ng/log.h"
#include <cstdlib>
#include "ng/json.h"
#include "kcl_os.h"
#if USE_ANY_GL
#include "opengl/glb_opengl_state_manager.h"
#include "opengl/glb_discard_functions.h"
    #if ENABLE_COMPLETE_STATS
        #include "opengl/gl_wrapper/gl_wrapper.h"
    #endif
#endif

#include "messagequeue.h"
#include "test_base0.h"
#include "test_descriptor.h"
#include "printable_result.h"
#include "global_test_environment.h"

#include "render_statistics_defines.h"

#include "kcl_io.h"
#include "gfxbench.h"
#include <float.h>
#include <fstream>
#include "messages_const.h"


#include "qualitymatch.h"

#ifdef USE_ANY_GL
#include "opengl/gl_wrapper/gl_wrapper.h"
#include "render_statistics.h"

#include "opengl/ext.h"
#include "opengl/glb_texture.h"
#endif

#include "status_report.h"

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

#ifdef USE_METAL
#include "mtl_types.h"
#endif


#ifdef ANDROID
#include "graphics/eglgraphicscontext.h" // TODO: This is just to get the selected EGL configuration, GFXBench normally should not include anything platform-specific.
#endif

#ifdef HAVE_DX
#include "d3d11/DX.h"
#include "d3d11/shader.h"

#include <locale>
#include <iostream>
#include <string>
#include <sstream>

#pragma comment(lib,"DXGI.lib")
using namespace std;
#endif

#include <memory>

// a frame is considered vsynced if fps is between these.
#define VSYNC_LIMIT_MAX 65
#define VSYNC_LIMIT_MIN 55
// vsync checks are made on frames accumulated during this time period
#define VSYNC_CHECK_INTERVAL_MS 1000
// the test is considered vsynced if this portion of the frames are vsynced during a check
#define VSYNC_LIMIT_TRIGGER_TRESHOLD 0.5f
// the test is maxed when this portion of the checks marks the test as vsynced
#define VSYNC_MAXED_TRIGGER_TRESHOLD 0.8f

using namespace tfw::JsonUtils;
#if ENABLE_COMPLETE_STATS
std::vector<MeasureResults> measured_frames;
GLWrapper  *glwrapper = GetGLWrapper();
#endif
#define USE_OPENGL_DEBUG 0
#define USE_AMD_OPENGL_DEBUG 0

// can be SEVERITY_LOW, SEVERITY_MEDIUM, SEVERITY_HIGH, GL_DONT_CARE
#define OPENGL_DEBUG_OUTPUT_SEVERITY GL_DEBUG_SEVERITY_HIGH
#define OPENGL_DEBUG_MESSAGE_LENGTH 4096


#if defined _WIN32 && defined _DEBUG && defined HAVE_GLEW && USE_OPENGL_DEBUG
	void FormatDebugOutputARB(char outStr[], size_t outStrSize, GLenum source, GLenum type,GLuint id, GLenum severity, const char *msg) ;
	void __stdcall DebugCallbackARB(GLenum source, GLenum type, GLuint id, GLenum severity,GLsizei length, const GLchar *message, const void *userParam) ;
#endif

#if defined _WIN32 && defined _DEBUG && defined HAVE_GLEW && USE_AMD_OPENGL_DEBUG
	void FormatDebugOutputAMD(char outStr[], size_t outStrSize, GLenum category, GLuint id, GLenum severity, const char *msg) ;
	void __stdcall DebugCallbackAMD(GLuint id, GLenum category, GLenum severity, GLsizei length,const GLchar *message, GLvoid *userParam) ;
#endif


GFXBench::GFXBench() :
	frames_(0),
	width_(256),
	height_(256),
	progress_(0),
	elapsedTime_(0),
	glb_(0),
	m_fps_window(0),
	m_query_interval_ms(1000)
{
    m_gte = NULL ;
	user_input_enabled = false;
    loadTimer.start();

	m_is_battery_test = false;
	m_is_report_mode = false;
	m_loop_count = 0;
	m_status_report = 0;
}

GFXBench::~GFXBench()
{
	terminate();
}

void GFXBench::onCancel()
{
	if (KCL::g_os!=NULL)
	{
		KCL::g_os->cancelTestLoading();
	}
}

bool GFXBench::terminate()
{
	if (m_status_report)
	{
		delete m_status_report;
		m_status_report = nullptr;
	}

	if (m_gte)
	{
		delete m_gte;
		m_gte = nullptr;
	}
	if (glb_)
	{
		delete glb_;
		glb_ = nullptr;
	}

	NGLOG_INFO("GFXBench::terminate");
	return true;
}

void GFXBench::cancel()
{
#ifndef OPT_COMMUNITY_BUILD
	if (initial_descriptor_.m_never_cancel) return;
#endif // !OPT_COMMUNITY_BUILD
	TestBase::cancel();
}

#ifdef HAVE_DX
char* narrow(const wstring& str)
{
	ostringstream stm;
	const ctype<char>& ctfacet = use_facet< ctype<char> >(stm.getloc());

	for (size_t i = 0; i < str.size(); ++i)
		stm << ctfacet.narrow(str[i], 0);

	string str_temp = stm.str();

	char* c = new char[str.size() + 1];
	strcpy(c, str_temp.c_str());

	return c;
}
#endif

bool GFXBench::InitializeTestEnvironment()
{
 	require(ctx_ != NULL);
	require(ctx_->makeCurrent());

	parseConfig(config(), initial_descriptor_);

    m_gte = new GlobalTestEnvironment(ctx_);

#if defined HAVE_GLEW || defined HAVE_GLES3
	if ( !(graphicsContext()->type() == graphicsContext()->OPENGL || graphicsContext()->type() == graphicsContext()->GLES))
	{
		NGLOG_INFO("Application's context mismatched! Test built with HAVE_GLEW. Supported context is %s", graphicsContext()->type());
		return false;
	}
#endif
#ifdef HAVE_DX
	if (graphicsContext()->type() != graphicsContext()->DIRECTX)
	{
		NGLOG_INFO("Application's context mismatched! Test built with HAVE_DX. Supported context is %s", graphicsContext()->type());
		return false;
	}
#endif
	if (m_gte->IsGraphicsContextGLorES())
    {
#if (defined USE_ANY_GL) && ((defined HAVE_GLES3) || (defined HAVE_GLEW))
#ifdef HAVE_GLEW
		glewExperimental = GL_TRUE;
		GLenum glew_status = glewInit();
		if (glew_status != GLEW_OK)
		{
			NGLOG_INFO("GLEW init error: %s", glew_status);
		}
        glGetError();//suppress error: GLEW has a problem with core contexts. It calls glGetString(GL_EXTENSIONS), which causes GL_INVALID_ENUM
#endif
		getES31ProcAddresses();
#endif
    }

#ifdef HAVE_DX
	DX::Attach((DXGraphicsContext*)ctx_);
	Shader::InitShaders("", false);
#elif defined USE_METAL
	MetalRender::InitMetalGraphicsContext(ctx_, m_selected_device_id);
#endif

#if defined _WIN32 && defined _DEBUG && defined HAVE_GLEW && USE_OPENGL_DEBUG
	if (glewIsSupported("GL_ARB_debug_output"))
	{
		glDebugMessageCallback(DebugCallbackARB, stderr);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glDebugMessageControl(GL_DONT_CARE,GL_DONT_CARE,OPENGL_DEBUG_OUTPUT_SEVERITY,0,nullptr,GL_TRUE) ;
	}
#endif


#if defined _WIN32 && defined _DEBUG && defined HAVE_GLEW && USE_AMD_OPENGL_DEBUG
	if (glewIsSupported("GL_AMD_debug_output")) {
		glDebugMessageCallbackAMD(DebugCallbackAMD,stderr) ;
		glDebugMessageEnableAMD(0,0,0,nullptr,true) ;
	}
#endif

    m_gte->SetTestDescriptor(&initial_descriptor_) ;
	m_gte->SetTestId(test_id_);

	if (!GLB::CreateGlobals(data_.c_str(), datarw_.c_str(), m_gte))
	{
		NGLOG_INFO("GLB::CreateGlobals failed!");
		return false;
	}

#ifdef HAVE_DX

/* legacy code before device selection
	IDXGIFactory1 *dxgiFactory;
	if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void **)&dxgiFactory)))
	{
		NGLOG_INFO("Couldn't create DXGIFactory");
		return false;
	}

	IDXGIAdapter *dxgiAdapter;
	UINT adapter_index = 0;
	while (dxgiFactory->EnumAdapters(adapter_index, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND)
	{
		auto& myAdapter = *dxgiAdapter;
		auto adapterDescription = new DXGI_ADAPTER_DESC();
		myAdapter.GetDesc(adapterDescription);

		if ((adapterDescription->VendorId == 0x1414) && (adapterDescription->DeviceId == 0x8c))
		{
			adapter_index++;
			continue; // http://msdn.microsoft.com/en-us/library/windows/desktop/bb205075(v=vs.85).aspx#WARP_new_for_Win8
		}

		const wchar_t* wcstr = adapterDescription->Description;
		vendor_ = narrow(wcstr);

		adapter_index++;
		dxgiAdapter->Release();

		//need to first adapter
		break;
	}
	dxgiFactory->Release();
*/

	{
		size_t delimiter_pos = m_selected_device_id.find_last_of(";");
		if (!(delimiter_pos == std::string::npos || m_selected_device_id.size() == 0))
		{
			m_vendor = m_selected_device_id.substr(0, delimiter_pos);
		}
		else
		{
			m_vendor = m_selected_device_id;
		}
	}

	m_renderer = "Direct3D";
	m_graphics_version = "Direct3D 11";
#elif defined USE_ANY_GL
	if (m_gte->IsGraphicsContextGLorES())
	{
		m_vendor = (const char*) glGetString(GL_VENDOR);
		m_renderer = (const char*) glGetString(GL_RENDERER);
		m_graphics_version = (const char*) glGetString(GL_VERSION);
	}

#ifdef HAVE_GLEW
	// Read back OpenGL context profile (OpenGL 3.2 and above)
	if (ctx_->versionMajor() >= 3 && ctx_->versionMinor() >= 2)
	{
		GLint mask;
		glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &mask);

		if (mask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
		{
			NGLOG_INFO("OpenGL profile:          Compatible");
		}
		else if (mask & GL_CONTEXT_CORE_PROFILE_BIT)
		{
			NGLOG_INFO("OpenGL profile:          Core");
		}
	}
#endif
#elif defined USE_METAL
    if (m_gte->IsGraphicsContextMetal())
    {
        m_vendor = "Apple" ;
        m_renderer = MetalRender::GetDeviceName() ;
        m_graphics_version = "Metal" ;
    }
#endif

	NGLOG_INFO("OpenGL Vendor:           %s", m_vendor);
	NGLOG_INFO("OpenGL Renderer:         %s", m_renderer);
	NGLOG_INFO("OpenGL Graphics Version: %s", m_graphics_version);

	PrintDefaultFBOInfo();

#ifndef IPHONE
#if defined HAVE_GLES3 || defined HAVE_GLES2
	typedef void (GFXB_APIENTRY *_PFNGLDISCARDFRAMEBUFFEREXT) (GLenum target, GLsizei numAttachments, const GLenum *attachments);
	extern _PFNGLDISCARDFRAMEBUFFEREXT _glDiscardFramebufferEXT;

	typedef void (GFXB_APIENTRY *_PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples);
	extern _PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC _glFramebufferTexture2DMultisampleEXT;

	typedef void (GFXB_APIENTRY *_PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
	extern _PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC _glRenderbufferStorageMultisampleEXT;

    if (strstr((const char *)glGetString(GL_EXTENSIONS), "GL_EXT_discard_framebuffer"))
    {
	    if(!_glDiscardFramebufferEXT)
	    {
#ifdef HAVE_EGL
			_glDiscardFramebufferEXT = (void (GFXB_APIENTRY *) (GLenum target, GLsizei numAttachments, const GLenum *attachments)) eglGetProcAddress("glDiscardFramebufferEXT");
#endif
	    }
    }

    if (strstr((const char *)glGetString(GL_EXTENSIONS), "GL_EXT_multisampled_render_to_texture"))
	{
        if(!_glFramebufferTexture2DMultisampleEXT)
	    {
#ifdef HAVE_EGL
			_glFramebufferTexture2DMultisampleEXT = (void(GFXB_APIENTRY *) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples)) eglGetProcAddress("glFramebufferTexture2DMultisampleEXT");
#endif
	    }
    }

    if (strstr((const char *)glGetString(GL_EXTENSIONS), "GL_EXT_multisampled_render_to_texture"))
    {
	    if(!_glRenderbufferStorageMultisampleEXT)
	    {
#ifdef HAVE_EGL
			_glRenderbufferStorageMultisampleEXT = (void (GFXB_APIENTRY *) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)) eglGetProcAddress("glRenderbufferStorageMultisampleEXT");
#endif
        }
	}
#endif
#endif

	core_count = KCL::g_os->GetNumOfCPUCores();
	if (core_count < 0) core_count = 0;
	if (core_count > 0)
	{
		// TODO: When number of CPU cores returned, but no frequency data
		if (KCL::g_os->GetCurrentCPUFrequency(0) <= 0) core_count = 0;
	}

	temp_logging_valid = KCL::g_os->GetCoreTemperature() > 0;
	gpu_logging_valid = KCL::g_os->GetCurrentGPUFrequency() >= 0;

	GPUfreqs.clear();
	CPUtemps.clear();
	CPUfreqs.clear();
	renderTimesMin.clear();
	renderTimesMax.clear();
	renderTimesAvg.clear();
	queryTimes.clear();
	renderTimesAccum = 0;
	renderFramesAccum = 0;
	renderTimeMin = 0;
	renderTimeMax = 0;

	progress_ = 0.0f;

	m_loop_count = 0;

	m_is_report_mode = initial_descriptor_.m_report_interval > 0 && initial_descriptor_.m_report_filename.size();
	if (m_is_report_mode)
	{
		m_status_report = new StatusReport();
		m_status_report->SetFilename(initial_descriptor_.m_report_filename);
		m_status_report->SetInterval(initial_descriptor_.m_report_interval);
	}

    //remove any FSAA flags if the test is Manhattan (Engine3)
    //NOTE: Manhattan based ES3 doesnt support multisampled textures that would
    //      be needed by the deferred renderer
    if( (initial_descriptor_.m_engine == "Engine3") && initial_descriptor_.m_fsaa > 0)
    {
        initial_descriptor_.SetFSAA(0);
    }

#ifdef USE_ANY_GL
	if (m_gte->IsGraphicsContextGLorES())
    {
        GLB::GLBTextureES3::resetSamplerMap(); // TODO: this needs refactoring

        std::string engine = initial_descriptor_.m_engine ;

        if ( (engine == "Engine31") ||
		 (engine == "Engine4" ) ||
		 (engine == "tess_test") ||
		 (engine == "driver_overhead_test2") ||
		 (engine == "compressed_fill_test2") ||
		 (engine == "alu_test2"))
        {
            GLB::g_extension->enableFeature(GLB::GLBFEATURE_vertex_array_object) ;
            GLB::g_extension->enableFeature(GLB::GLBFEATURE_sampler_object);
            GLB::g_extension->enableFeature(GLB::GLBFEATURE_invalidate_framebuffer);
            GLB::g_extension->enableFeature(GLB::GLBFEATURE_es3_compatibility);
            GLB::g_extension->enableExtension(GLB::GLBEXT_es3_compatibility);
        }
        else if ( engine == "Engine3" )
        {
            if (ctx_->type() == GraphicsContext::GLES)
            {
                GLB::g_extension->disableFeature(GLB::GLBFEATURE_vertex_array_object) ;
            }
            else
            {
                GLB::g_extension->enableFeature(GLB::GLBFEATURE_vertex_array_object) ;
            }
            GLB::g_extension->enableFeature(GLB::GLBFEATURE_sampler_object);
            GLB::g_extension->enableFeature(GLB::GLBFEATURE_invalidate_framebuffer);
            GLB::g_extension->enableFeature(GLB::GLBFEATURE_es3_compatibility);
            GLB::g_extension->enableExtension(GLB::GLBEXT_es3_compatibility);
        }
        else // engine <= "Engine2"
        {
            GLB::g_extension->disableFeature(GLB::GLBFEATURE_vertex_array_object) ;
            GLB::g_extension->disableFeature(GLB::GLBFEATURE_sampler_object);
            GLB::g_extension->disableFeature(GLB::GLBFEATURE_invalidate_framebuffer);
            GLB::g_extension->disableFeature(GLB::GLBFEATURE_es3_compatibility);
            GLB::g_extension->disableExtension(GLB::GLBEXT_es3_compatibility);
        }

        GLB::OpenGLStateManager::Reset() ;

        GLB::g_extension->dumpFeatureState() ;
    }

    glViewport(0, 0, width_, height_);

#endif
	return true;
}

bool GFXBench::InitializeTest()
{
	bool ok = false;

	if(glb_ != NULL)
	{
		KCL::KCL_Status err;
		glb_->setLoadingProgressPtr(&progress_);

        std::unique_ptr<GLB::NewNotificationManager> nnm(GLB::NewNotificationManager::NewInstance());
        nnm->ShowLoadingLogo();

        require(ctx_->swapBuffers());

		err = glb_->init0();
		if(err == KCL::KCL_TESTERROR_NOERROR)
		{
			ok = true;
		}
		else
		{
			storeResults();
			NGLOG_ERROR("Failed to init GLB test: %s", KCL::KCL_Status_To_Cstr(err));
		}
	}
	else
	{
		NGLOG_ERROR("Failed to create GLB test");
	}
	if (!ok)
	{
		GLB::DestroyGlobals(m_gte);
		return false;
	}

    int warmup_frame_count = glb_->GetSetting().m_warmup_frames;
    if (warmup_frame_count > 0)
    {
        // The frame time of the warm up frame
        int warmup_frame_time = animationTimeMilliseconds = glb_->GetSetting().m_single_frame >= 0 ?
		    glb_->GetSetting().m_single_frame :
		    glb_->GetSetting().m_start_animation_time;

        glb_->m_is_warmup = true;
#ifdef USE_ANY_GL
    	glViewport(0, 0, width_, height_);
#endif
        // Warm up: Animate and render
        NGLOG_INFO("Warming up. Frame time: %s Frame count: %s", warmup_frame_time, warmup_frame_count);
        for (int i = 0; i < warmup_frame_count; i++)
        {
            glb_->animate(warmup_frame_time);
            glb_->render0();
#ifdef USE_ANY_GL
            glFinish();
 #endif
        }

        // Cleanup
        glb_->m_is_warmup = false;
        glb_->ResetFrameCounter();
    }

	progress_ = 1.0f;
	armedTimer = ng::cpu_timer(false);
	const TestDescriptor &descriptor_ = glb_->GetSetting();
	animationTimeMilliseconds = descriptor_.m_single_frame >= 0 ?
		descriptor_.m_single_frame :
		descriptor_.m_start_animation_time;
	frames_ = 0;
	elapsedTime_ = 0;
	m_min_frame_time = descriptor_.m_fps_limit > 0.0 ? 1000.0 / descriptor_.m_fps_limit : -1.0;
	m_frame_time_limit_enabled = m_min_frame_time > 1.0;
		// !descriptor_.m_is_battery
		//&& !GetCurrentTest().isFixedTime()
		//&& !GetCurrentTest().GetSetting().m_is_endless
		//&& GetCurrentTest().GetSetting().m_single_frame == -1
		//&& m_min_frame_time > 1.0;

	m_vsync_frames_accum_vsync = 0;
	m_vsync_time_accum = 0;
	m_vsync_frames_accum = 0;
	m_vsync_limit_count = 0;

	paused = false;
	done = false;

	fps_time = 0;
	fps_frames = 0;

	loadTimer.stop();

    //double lt = loadTimer.elapsed().wall;
    //INFO("LOADED IN: %.1f sec", lt);

	timer = ng::cpu_timer(false);
	firstStep = true;
	return ok;
}

/*bool GFXBench::init()
{
	InitializeTestEnvironment();
	glb_ = GLB::CreateTest(m_gte);
	return InitializeTest();
}*/

bool GFXBench::step()
{
	require(ctx_->makeCurrent());
	bool ret = stepNoCtx();
	ctx_->detachThread();
	return ret;
}

void GFXBench::processMessages()
{
	if (msgQueue_)
	{
		while (msgQueue_->has_next())
		{
			tfw::Message msg = msgQueue_->pop_front();
			if (msg.type == MSG_TYPE_CLOSE)
			{
				cancel();
			}
			if (msg.type == MSG_TYPE_BATTERY)
			{
				KCL::g_os->SetBatteryCharging(msg.arg1);
				KCL::g_os->SetBatteryLevel(msg.arg2 / 100.0);
			}

#if defined WIN32
			//#if defined _DEBUG && (defined WIN32 || defined _METRO_)
			else if (msg.type == MSG_TYPE_KEY)
			{
				if ((msg.arg2 < 2) && user_input_enabled) //press == 1, release == 0
				{
					glb_->setKeysCurrent(msg.arg1, msg.arg2);
				}
			}
#endif
                else if(msg.type == MSG_TYPE_CURSOR)
                {
                    glb_->setCursor(msg.arg1, msg.arg2);
                }
                else if(msg.type == MSG_TYPE_MOUSE)
                {
                    glb_->setMouseCurrent(msg.arg1, msg.arg2);
                }

			//arg1: tag
			//arg2: value

			else if (msg.type == MSG_TYPE_CHECKBOX)
			{
				glb_->onCheckboxChanged(msg.arg1, msg.arg2);
				if (msg.arg1 == -1)
				{
					paused = msg.arg2;
				}
			}
			else if (msg.type == MSG_TYPE_RANGE)
			{
				glb_->onSliderChanged(msg.arg1, (float)msg.arg2 / 100.f);
			}


			else if (msg.type == MSG_TYPE_RESIZE)
			{
				glb_->resize(msg.arg1, msg.arg2);
			}
		}
	}
}

bool GFXBench::getScreenshotName(char* const screenshotName)
{
	const std::vector<int>& frames = glb_->GetSetting().m_screenshot_frames;
	for (size_t i = 0; i < frames.size(); i++)
	{
		if ((glb_->GetSetting().isFixedTime() && frames[i] == glb_->getFrames()) ||
			((glb_->GetSetting().m_single_frame>-1) && frames[i] == glb_->getFrames()) ||
			(!glb_->GetSetting().isFixedTime() && frames[i] == animationTimeMilliseconds))
		{
			sprintf(screenshotName, "scr_%s_%06dms", glb_->GetSetting().m_name.c_str(), animationTimeMilliseconds);
			return true;
		}
	}
	return false;
}

bool GFXBench::stepNoCtx()
{
	const TestDescriptor &descriptor_ = glb_->GetSetting();
	double playTime = descriptor_.m_play_time / 1000.0;

	if (firstStep)
	{
		firstStep = false;
		progress_ = 0.0f;

#ifdef USE_ANY_GL
		glViewport(0, 0, width_, height_);
#endif
		lastTimeMilliseconds = timer.elapsed().wall * 1000;

		lastQueryTime = lastTimeMilliseconds;

		if (descriptor_.GetScreenMode() != SMode_Onscreen)
		{
            std::auto_ptr<GLB::NewNotificationManager> nnm(GLB::NewNotificationManager::NewInstance());
            nnm->ShowRunningLogo();
            require(ctx_->swapBuffers());
            return true;
		}
	}

#ifdef GL_WRAPPER_STATS
    GetGLWrapper()->ResumeMeasurement();
#endif

	if (timer.is_stopped()) timer.start();
	bool swapbuffers = false;
	while (!swapbuffers)
	{
		if (isCancelled()) done = true;
		//INFO("TIME: %d", animationTimeMilliseconds);
		//enter ms time here (like 62000) for single-frame shader debugging, see DEBUG_SHADERS_ON_DEVICE define
		processMessages();

		if (glb_->GetSetting().m_max_rendered_frames >= 0 && glb_->getFrames() >= glb_->GetSetting().m_max_rendered_frames) done = true;

		if (!done) done = !glb_->animate(animationTimeMilliseconds);

		if (done)
		{
#if ENABLE_COMPLETE_STATS
			//=========Stat=========//
			KCL::uint32 min_vertex_count = UINT_MAX;
			KCL::uint32 max_vertex_count = 0;
			KCL::uint64 avg_vertex_count = 0;

			KCL::uint32 min_triangle_count = UINT_MAX;
			KCL::uint32 max_triangle_count = 0;
			KCL::uint64 avg_triangle_count = 0;

			KCL::uint32 min_texture_count = UINT_MAX;
			KCL::uint32 max_texture_count = 0;
			KCL::uint64 avg_texture_count = 0;

			KCL::uint32 min_samples_passed = UINT_MAX;
			KCL::uint32 max_samples_passed = 0;
			KCL::uint64 avg_samples_passed = 0;

			KCL::uint32 min_generated_primitives = UINT_MAX;
			KCL::uint32 max_generated_primitives = 0;
			KCL::uint64 avg_generated_primitives = 0;

            KCL::uint32 min_tess_draw_count = UINT_MAX;
			KCL::uint32 max_tess_draw_count = 0;
			KCL::uint64 avg_tess_draw_count = 0;

			KCL::uint32 min_gs_draw_count = UINT_MAX;
			KCL::uint32 max_gs_draw_count = 0;
			KCL::uint64 avg_gs_draw_count = 0;

            KCL::uint32 min_pipeStats[DrawCallStatistics::query_stats_count];
            std::fill_n(min_pipeStats, DrawCallStatistics::query_stats_count, UINT_MAX);
            KCL::uint32 max_pipeStats[DrawCallStatistics::query_stats_count] = {};
            KCL::uint64 avg_pipeStats[DrawCallStatistics::query_stats_count] = {};
			for (int i = 0; i < measured_frames.size(); i++)
			{
				KCL::uint32 sumOfVertices = 0;
				KCL::uint32 sumOfTriangles = 0;
				KCL::uint32 sumOfTexture = 0;
				KCL::uint32 sumOfSamples = 0;
				KCL::uint32 sumOfDispatchCount = 0;
				KCL::uint32 sumOfGeneratedPrimitives = 0;
                KCL::uint32 sumOfPipeStats[DrawCallStatistics::query_stats_count] = {};

                KCL::uint32 sumOfTesDraws = 0;
                KCL::uint32 sumOfGsDraws = 0;

				std::set<KCL::uint32> texture_ids;
				for (int h = 0; h < measured_frames[i].draw_calls.size(); h++)
				{
					sumOfVertices += measured_frames[i].draw_calls[h].num_draw_vertices;
					sumOfTriangles += measured_frames[i].draw_calls[h].num_draw_triangles;
					sumOfSamples += measured_frames[i].draw_calls[h].num_samples_passed;
					sumOfDispatchCount += measured_frames[i].m_dispatch_count;
					texture_ids.insert(measured_frames[i].draw_calls[h].texture_ids.begin(), measured_frames[i].draw_calls[h].texture_ids.end());
					sumOfGeneratedPrimitives += measured_frames[i].draw_calls[h].generated_primitives;
                    for(int psc=0; psc<DrawCallStatistics::query_stats_count; ++psc)
                    {
                        sumOfPipeStats[psc] += measured_frames[i].draw_calls[h].num_query_stats[psc];
                    }

                    if(measured_frames[i].draw_calls[h].uses_TES)
                    {
                        ++sumOfTesDraws;
                    }
                    if(measured_frames[i].draw_calls[h].uses_GS)
                    {
                        ++sumOfGsDraws;
                    }
				}
				sumOfTexture += texture_ids.size();

				min_tess_draw_count = std::min(min_tess_draw_count, sumOfTesDraws);
				max_tess_draw_count = std::max(max_tess_draw_count, sumOfTesDraws);
				avg_tess_draw_count += sumOfTesDraws;

				min_gs_draw_count = std::min(min_gs_draw_count, sumOfGsDraws);
				max_gs_draw_count = std::max(max_gs_draw_count, sumOfGsDraws);
				avg_gs_draw_count += sumOfGsDraws;

				min_vertex_count = std::min(min_vertex_count, sumOfVertices);
				max_vertex_count = std::max(max_vertex_count, sumOfVertices);
				avg_vertex_count += sumOfVertices;

                //assert(sumOfPipeStats[0] != 0);
                //assert(sumOfPipeStats[1] != 0);
                //assert(sumOfPipeStats[2] != 0);
                //assert(sumOfPipeStats[7] != 0);

				min_triangle_count = std::min(min_triangle_count, sumOfTriangles);
				max_triangle_count = std::max(max_triangle_count, sumOfTriangles);
				avg_triangle_count += sumOfTriangles;

				min_texture_count = std::min(min_texture_count, sumOfTexture);
				max_texture_count = std::max(max_texture_count, sumOfTexture);
				avg_texture_count += sumOfTexture;

				min_samples_passed = std::min(min_samples_passed, sumOfSamples);
				max_samples_passed = std::max(max_samples_passed, sumOfSamples);
				avg_samples_passed += sumOfSamples;

				min_generated_primitives = std::min(min_generated_primitives, sumOfGeneratedPrimitives);
				max_generated_primitives = std::max(max_generated_primitives, sumOfGeneratedPrimitives);
				avg_generated_primitives += sumOfGeneratedPrimitives;

                for(int psc=0; psc<DrawCallStatistics::query_stats_count; ++psc)
                {
                    min_pipeStats[psc] = std::min(min_pipeStats[psc], sumOfPipeStats[psc]);
                    max_pipeStats[psc] = std::max(max_pipeStats[psc], sumOfPipeStats[psc]);
                    avg_pipeStats[psc] += sumOfPipeStats[psc];
                }
			}

            //assert(min_pipeStats[0] != 0);
            //assert(min_pipeStats[1] != 0);
            //assert(min_pipeStats[7] != 0);

            KCL::uint32 amnt = measured_frames.size();
			avg_vertex_count /= amnt;
			avg_triangle_count /= amnt;
			avg_texture_count /= amnt;
			avg_samples_passed /= amnt;
			avg_generated_primitives /= amnt;
            avg_tess_draw_count /= amnt;
            avg_gs_draw_count /= amnt;

            for(int psc=0; psc<DrawCallStatistics::query_stats_count; ++psc)
            {
                avg_pipeStats[psc] /= measured_frames.size();
            }


			KCL::uint32 min_draw_count = UINT_MAX;
			KCL::uint32 max_draw_count = 0;
			KCL::uint32 avg_draw_count = 0;

			KCL::uint32 min_dispatch_count = UINT_MAX;
			KCL::uint32 max_dispatch_count = 0;
			KCL::uint32 avg_dispatch_count = 0;
			for (int i = 0; i < measured_frames.size(); i++)
			{
				min_draw_count = std::min(min_draw_count, measured_frames[i].draw_calls.size());
				max_draw_count = std::max(max_draw_count, measured_frames[i].draw_calls.size());
				avg_draw_count += measured_frames[i].draw_calls.size();

				min_dispatch_count = std::min(min_dispatch_count, measured_frames[i].m_dispatch_count);
				max_dispatch_count = std::max(max_dispatch_count, measured_frames[i].m_dispatch_count);
				avg_dispatch_count += measured_frames[i].m_dispatch_count;
			}
			avg_draw_count /= measured_frames.size();
			avg_dispatch_count /= measured_frames.size();
			std::fstream csvFile("stat.csv", std::ios::out);
            INFO("Statistics calculated for resolution: %dx%d", initial_descriptor_.m_viewport_width, initial_descriptor_.m_viewport_height);

            const char* pipeStatsNames[] =
            {
                "Verts Submitted",
                "Prims Submitted",
                "VS Invocations",
                "TCS Patches",
                "TES Invocations",
                "GS Invocations",
                "GS Prims Emitted",
                "FS Invocations",
                "CS Invocations",
                "Clipping In Prims",
                "Clipping Out Prims"
            };

            for(int psc=0; psc<DrawCallStatistics::query_stats_count; ++psc)
            {
                INFO("min_%s=%u", pipeStatsNames[psc], min_pipeStats[psc]);
			    INFO("max_%s=%u", pipeStatsNames[psc], max_pipeStats[psc]);
			    INFO("avg_%s=%u", pipeStatsNames[psc], avg_pipeStats[psc]);
			    csvFile << "min_" << pipeStatsNames[psc] << " " << min_pipeStats[psc] << std::endl;
			    csvFile << "max_" << pipeStatsNames[psc] << " " << max_pipeStats[psc] << std::endl;
			    csvFile << "avg_" << pipeStatsNames[psc] << " " << avg_pipeStats[psc] << std::endl;
            }

            INFO("min_generated_primitives=%u", min_generated_primitives);
			INFO("max_generated_primitives=%u", max_generated_primitives);
			INFO("avg_generated_primitives=%u", avg_generated_primitives);
			csvFile << "min_generated_primitives" << " " << min_generated_primitives << std::endl;
			csvFile << "max_generated_primitives" << " " << max_generated_primitives << std::endl;
			csvFile << "avg_generated_primitives" << " " << avg_generated_primitives << std::endl;

			INFO("min_vertex_count=%u", min_vertex_count);
			INFO("max_vertex_count=%u", max_vertex_count);
			INFO("avg_vertex_count=%u", avg_vertex_count);

			csvFile << "min_vertex_count" << " " << min_vertex_count << std::endl;
			csvFile << "max_vertex_count" << " " << max_vertex_count << std::endl;
			csvFile << "avg_vertex_count" << " " << avg_vertex_count << std::endl;

			INFO("min_triangle_count=%u", min_triangle_count);
			INFO("max_triangle_count=%u", max_triangle_count);
			INFO("avg_triangle_count=%u", avg_triangle_count);

			csvFile << "min_triangle_count" << " " << min_triangle_count << std::endl;
			csvFile << "max_triangle_count" << " " << max_triangle_count << std::endl;
			csvFile << "avg_triangle_count" << " " << avg_triangle_count << std::endl;

			INFO("min_draw_count=%u", min_draw_count);
			INFO("max_draw_count=%u", max_draw_count);
			INFO("avg_draw_count=%u", avg_draw_count);

			csvFile << "min_draw_count" << " " << min_draw_count << std::endl;
			csvFile << "max_draw_count" << " " << max_draw_count << std::endl;
			csvFile << "avg_draw_count" << " " << avg_draw_count << std::endl;

			INFO("min_tess_draw_count=%u", min_tess_draw_count);
			INFO("max_tess_draw_count=%u", max_tess_draw_count);
			INFO("avg_tess_draw_count=%u", avg_tess_draw_count);

			csvFile << "min_tess_draw_count" << " " << min_tess_draw_count << std::endl;
			csvFile << "max_tess_draw_count" << " " << max_tess_draw_count << std::endl;
			csvFile << "avg_tess_draw_count" << " " << avg_tess_draw_count << std::endl;

			INFO("min_GS_draw_count=%u", min_gs_draw_count);
			INFO("max_GS_draw_count=%u", max_gs_draw_count);
			INFO("avg_GS_draw_count=%u", avg_gs_draw_count);

			csvFile << "min_GS_draw_count" << " " << min_gs_draw_count << std::endl;
			csvFile << "max_GS_draw_count" << " " << max_gs_draw_count << std::endl;
			csvFile << "avg_GS_draw_count" << " " << avg_gs_draw_count << std::endl;

			INFO("min_texture_count=%u", min_texture_count);
			INFO("max_texture_count=%u", max_texture_count);
			INFO("avg_texture_count=%u", avg_texture_count);

			csvFile << "min_texture_count" << " " << min_texture_count << std::endl;
			csvFile << "max_texture_count" << " " << max_texture_count << std::endl;
			csvFile << "avg_texture_count" << " " << avg_texture_count << std::endl;

			INFO("min_samples_passed=%u", min_samples_passed);
			INFO("max_samples_passed=%u", max_samples_passed);
			INFO("avg_samples_passed=%u", avg_samples_passed);

			csvFile << "min_samples_passed" << " " << min_samples_passed << std::endl;
			csvFile << "max_samples_passed" << " " << max_samples_passed << std::endl;
			csvFile << "avg_samples_passed" << " " << avg_samples_passed << std::endl;

			//*********************
			StatisticsHelper::OverdrawFromSamplesPassed overdrawFromSamplesPassed(initial_descriptor_.m_viewport_width, initial_descriptor_.m_viewport_height);
			double overdraw_min = overdrawFromSamplesPassed(min_samples_passed);
			double overdraw_max = overdrawFromSamplesPassed(max_samples_passed);
			double overdraw_avg = overdrawFromSamplesPassed(avg_samples_passed);
			INFO("overdraw_min:%f", overdraw_min);
			INFO("overdraw_max:%f", overdraw_max);
			INFO("overdraw_avg:%f", overdraw_avg);
            csvFile << "overdraw_min" << " " << overdraw_min << std::endl;
			csvFile << "overdraw_max" << " " << overdraw_max << std::endl;
			csvFile << "overdraw_avg" << " " << overdraw_avg << std::endl;

			StatisticsHelper::MemBWFromSamplesPassed bwCalc;
			double bwMin = bwCalc(min_samples_passed);
			double bwMax = bwCalc(max_samples_passed);
			double bwAvg = bwCalc(avg_samples_passed);
			//*********************

			INFO("min_dispatch_count=%u", min_dispatch_count);
			INFO("max_dispatch_count=%u", max_dispatch_count);
			INFO("avg_dispatch_count=%u", avg_dispatch_count);

			csvFile << "min_dispatch_count" << " " << min_dispatch_count << std::endl;
			csvFile << "max_dispatch_count" << " " << max_dispatch_count << std::endl;
			csvFile << "avg_dispatch_count" << " " << avg_dispatch_count << std::endl;

			csvFile.close();

			// Per frame statistics
			std::fstream stat_file(test_id_ + "_per_frame_stat.csv", std::ios::out);
			stat_file << "frametime, draw calls, clipping input, clipping output" << std::endl;
			for (size_t i = 0; i < measured_frames.size(); i++)
			{
				KCL::uint32 sum_clipping_input = 0;
				KCL::uint32 sum_clipping_output = 0;
				for (size_t j = 0; j < measured_frames[i].draw_calls.size(); j++)
				{
					sum_clipping_input += measured_frames[i].draw_calls[j].num_query_stats[9];
					sum_clipping_output += measured_frames[i].draw_calls[j].num_query_stats[10];
				}

				stat_file << measured_frames[i].m_frame_time << ',';
				stat_file << measured_frames[i].draw_calls.size() << ',';
				stat_file << sum_clipping_input << ',';
				stat_file << sum_clipping_output << std::endl;
			}
			stat_file.close();


			//=========Stat=========//
#endif
			m_loop_count++;

			if (glb_->GetSetting().m_is_endless && !isCancelled())
			{
				// Re-initialize animation time. See code before main loop.
				animationTimeMilliseconds = descriptor_.m_single_frame >= 0 ?
					descriptor_.m_single_frame :
					descriptor_.m_start_animation_time;
				//timer.set_elapsed(ng::cpu_times());
				//timer.start();
				//lastTimeMilliseconds = 0;

				done = false;
			}
			else if (!m_is_battery_test && glb_->GetSetting().m_loop_count > 1)
			{
				if (m_loop_count < glb_->GetSetting().m_loop_count)
				{
					NGLOG_INFO("Restarting test: %s/%s", m_loop_count + 1, glb_->GetSetting().m_loop_count);
					// Re-initialize animation time. See code before main loop.
					animationTimeMilliseconds = descriptor_.m_single_frame >= 0 ?
						descriptor_.m_single_frame :
						descriptor_.m_start_animation_time;
					done = false;
				}
			}

			if (m_is_report_mode)
			{
				m_status_report->SetLoopCount(m_loop_count);
				m_status_report->WriteReport();
			}

			if (done)
			{
                // Ensure we swap the buffers after the last frame
                glb_->finishTest();

				elapsedTime_ = timer.elapsed().wall * 1000;
				glb_->setElapsedTime(elapsedTime_);
#define NEW_QUALITY_TEST 0
#if NEW_QUALITY_TEST

#define SAVE_QM_IMAGES 0
#if SAVE_QM_IMAGES
				m_gte->GetTestDescriptor()->m_qm_save_image = 1;
				m_qm_tests.push_back(new QualityMatch(m_gte, (GLB::TestBase*) glb_, "SAVE_MODE", 1));
#endif
				if (ctx_->type() == GraphicsContext::GLES && STRINGIFY(PRODUCT_VERSION)[0] == '4')//check major version
				//if (STRINGIFY(PRODUCT_VERSION)[0] == '4')//to create ref. image
				{
					if (m_gte->GetTestDescriptor()->m_name != "gl_trex_qmatch" &&
						m_gte->GetTestDescriptor()->m_name != "gl_trex_qmatch_highp" &&
						m_gte->GetTestDescriptor()->m_name != "gl_trex_battery" &&
						!(m_gte->GetTestDescriptor()->m_single_frame > -1)
						)
					{
						m_gte->GetTestDescriptor()->SetTestWidth(1920);
						m_gte->GetTestDescriptor()->SetTestHeight(1080);
						m_qm_tests.push_back(new QualityMatch(m_gte, (GLB::TestBase*) glb_, "hd7700m", 1));
						m_qm_tests.push_back(new QualityMatch(m_gte, (GLB::TestBase*) glb_, "hd5500", 1));
						m_qm_tests.push_back(new QualityMatch(m_gte, (GLB::TestBase*) glb_, "gt540", 1));
					}
				}
				for (int i = 0; i < m_qm_tests.size(); ++i)
				{
					m_qm_tests[i]->SetSetting().SetName(m_qm_tests[i]->GetSetting().m_name + "_quality");
				}

				std::vector < QualityMatch* > tmp;
				for (auto it = m_qm_tests.begin(); it != m_qm_tests.end(); ++it)
				{
					int compare_frame = (*it)->GetSetting().qm_compare_frame;

					if (compare_frame> -1)
					{
						(*it)->SetSetting().SetSingleFrame(compare_frame);//set the animate time
						if ((*it)->init0() != KCL::KCL_TESTERROR_NOERROR)
						{
							continue;
						}


						//render 4times
						for (int i = 0; i < 4; ++i)
						{
							(*it)->animate(compare_frame);//animation time came from single frame time
							(*it)->render0("");
						}
						if( (*it)->getScore() > -1.0 )
						{
							tmp.push_back(*it);
						}
					}
				}
				if (tmp.empty())
				{
					for (int h = 0; h < m_qm_tests.size(); ++h)
					{
						delete m_qm_tests[h];
					}
				}
				m_qm_tests = tmp;
#endif
				progress_ = 1.0f;
				storeResults();

#if STATISTICS_LOGGING_ENABLED
				glb_->saveStatistics();
#endif
#ifdef SIGNIFICANT_FRAME_MODE
				glb_->saveStatistics();
#endif

				delete glb_;
				glb_ = 0;

				GLB::DestroyGlobals(m_gte);

				return false;
			}
		}

		char screenshotName[1024];

#if ENABLE_COMPLETE_STATS
        //NOTE: pipestats need GL 4.5 driver!
        GLuint frame_result_id = glwrapper->BeginMeasure(MeasureFlags::DrawCalls | MeasureFlags::TextureCount | MeasureFlags::Samples | MeasureFlags::VertexBuffer | MeasureFlags::PipeStats);
#endif
		swapbuffers = glb_->render0(getScreenshotName(screenshotName) ? screenshotName : NULL);
		if (swapbuffers)
		{
#ifndef SIGNIFICANT_FRAME_MODE
			require(ctx_->swapBuffers());
#else
			glFinish();
#endif
		}
#if ENABLE_COMPLETE_STATS
		MeasureResults measure_result = glwrapper->EndMeasure(frame_result_id);
		measure_result.m_frame_time = animationTimeMilliseconds;
		measured_frames.push_back(measure_result);
#endif
		if (isArmed())
		{
			if (armedTimer.is_stopped())
			{
				armedTimer.resume();
			}
			++frames_;
			progress_ = armedTimer.elapsed().wall / playTime;
		}
		else
		{
			armedTimer.stop();
		}

		int currentTimeMilliseconds = timer.elapsed().wall * 1000;
		int renderTime = currentTimeMilliseconds - lastTimeMilliseconds;
		if (m_frame_time_limit_enabled && renderTime < m_min_frame_time)
		{
			KCL::g_os->Sleep(m_min_frame_time - renderTime);
			currentTimeMilliseconds = timer.elapsed().wall * 1000;
			renderTime = currentTimeMilliseconds - lastTimeMilliseconds;
		}

		if (m_is_report_mode)
		{
			m_status_report->OnFrame(currentTimeMilliseconds, renderTime);
		}

		if (renderTime>(1000 / VSYNC_LIMIT_MAX) && renderTime < (1000 / VSYNC_LIMIT_MIN))
		{
			++m_vsync_frames_accum_vsync;
		}
		m_vsync_time_accum += renderTime;
		++m_vsync_frames_accum;

		if (m_vsync_time_accum > VSYNC_CHECK_INTERVAL_MS)
		{
			if (((float)m_vsync_frames_accum_vsync) / m_vsync_frames_accum > VSYNC_LIMIT_TRIGGER_TRESHOLD)
			{
				++m_vsync_limit_count;
			}
			m_vsync_time_accum = 0;
			m_vsync_frames_accum = 0;
			m_vsync_frames_accum_vsync = 0;
		}

#ifndef DISABLE_RESULTS
		if (m_fps_window>0)
		{
			++fps_frames;
			fps_time += renderTime;
			if (fps_time >= m_fps_window)
			{
				INFO("FPS: %.2f", fps_frames*(1000.0f / fps_time));
				fps_frames = 0;
				fps_time = 0;
			}
		}
#endif

		int nextAnimationTime;
		if (descriptor_.m_frame_step_time > 0)
		{
			nextAnimationTime = animationTimeMilliseconds + descriptor_.m_frame_step_time;
		}
		else if (descriptor_.m_single_frame >= 0)
		{
			nextAnimationTime = animationTimeMilliseconds;
		}
		else
		{
			nextAnimationTime = animationTimeMilliseconds + renderTime;
		}

		// Check if we jumped over a timestamp to create screenshot for
		if (!glb_->GetSetting().isFixedTime() &&
			glb_->GetSetting().m_screenshot_frames.size() > 0)
		{
			const std::vector<int>& frames = glb_->GetSetting().m_screenshot_frames;
			for (size_t i = 0; i < frames.size(); i++)
			{
				if (nextAnimationTime > frames[i] &&
					animationTimeMilliseconds < frames[i])
				{
					nextAnimationTime = frames[i];
					break;
				}
			}
		}

		++renderFramesAccum;
		renderTimesAccum += renderTime;
		if (renderTime > renderTimeMax) renderTimeMax = renderTime;
		if (renderTimeMin<0 || renderTime < renderTimeMin) renderTimeMin = renderTime;

		if (currentTimeMilliseconds - lastQueryTime > m_query_interval_ms)
		{
			lastQueryTime = currentTimeMilliseconds;
			float temp = KCL::g_os->GetCoreTemperature();
			if (temp_logging_valid) CPUtemps.push_back(temp);
			int gpu = KCL::g_os->GetCurrentGPUFrequency();
			if (gpu_logging_valid) GPUfreqs.push_back(gpu);
			for (int i = 0; i < core_count; ++i)
			{
				int freq = KCL::g_os->GetCurrentCPUFrequency(i);
				if (freq>0) freq /= 1000;
				CPUfreqs.push_back(freq);
			}

			renderTimesMin.push_back(renderTimeMin);
			renderTimesMax.push_back(renderTimeMax);
			renderTimesAvg.push_back(renderTimesAccum / renderFramesAccum);

			renderFramesAccum = 0;
			renderTimesAccum = 0;
			renderTimeMin = -1;
			renderTimeMax = -1;

			queryTimes.push_back(currentTimeMilliseconds);
		}

#if STATISTICS_LOGGING_ENABLED
		glb_->logStatistics(renderTime);
#endif
#ifdef SIGNIFICANT_FRAME_MODE
		glb_->logStatistics(renderTime);
#endif

		animationTimeMilliseconds = nextAnimationTime;
		lastTimeMilliseconds = currentTimeMilliseconds;
	}
	return true;
}

void GFXBench::run()
{
	require(ctx_->makeCurrent());
	while (stepNoCtx());
	ctx_->detachThread();
}

float GFXBench::progress() { return progress_; }

bool GFXBench::parseTestDescriptor(const tfw::Descriptor &d, TestDescriptor &td)
{
    bool ret = false;
	try
	{
        td.SetName(test_id_);
		m_fps_window = d.rawConfign("fps_log_window", 0);
		td.m_brightness = d.rawConfign("brightness", -1);
        td.SetDebugBattery(d.rawConfigb("debug_battery", false));
		td.SetColorBpp(d.rawConfign("color_bpp"));
		td.SetDepthBpp(d.rawConfign("depth_bpp"));
		td.m_is_endless = d.rawConfigb("endless", false);
		td.SetEngine(d.rawConfigs("engine"));
		td.m_fps_limit = d.rawConfign("fps_limit", -1);
		td.SetFrameStepTime(d.rawConfign("frame_step_time", -1));
		td.SetFSAA(d.rawConfign("fsaa", 0));
		td.m_max_rendered_frames = d.rawConfign("max_rendered_frames", -1);
		td.SetMinRamRequired(d.rawConfign("min_ram_required", -1));
		td.SetPlayTime(d.rawConfign("play_time"));
		td.SetForceHighp(d.rawConfigb("force_highp", false));
        td.SetTessellationEnabled(d.rawConfigb("tessellation_enabled", true));
		td.SetSceneFile(d.rawConfigs("scenefile", ""));
		td.SetScreenMode((ScreenMode)(int)d.rawConfign("screenmode"));
		td.SetSingleFrame(d.rawConfign("single_frame", -1));
		td.SetStartAnimationTime(d.rawConfign("start_animation_time", -1));
		td.m_stencil_bpp = d.rawConfign("stencil_bpp");
		td.SetTextureType(d.rawConfigs("texture_type"));
		td.SetZPrePass(d.rawConfigb("zprepass", false));

		td.m_qm_metric = d.rawConfigs("qm_metric", "");
		td.m_qm_reference_filename = d.rawConfigs("qm_reference_filename", "");
		td.m_qm_save_image = d.rawConfigb("qm_save_image", false);

		td.m_loop_count = d.rawConfign("loop_count", 0);
		td.m_battery_charge_drop = d.rawConfign("battery_charge_drop", -1);

		td.SetTestWidth(d.rawConfign("test_width", 1920));
		td.SetTestHeight(d.rawConfign("test_height", 1080));
        td.SetWidth(width_);
        td.SetHeight(height_);
	    td.SetVirtualResolution(d.rawConfigb("virtual_resolution", false));
		td.SetScreenshotFrames(d.rawConfigs("screenshot_frames", ""));
		td.SetParticleBufferSaveFrames(d.rawConfigs("particle_save_frames", ""));
		td.m_disabledRenderBits = d.rawConfign("disabled_render_bits", 0);
        td.SetWarmupFrames(d.rawConfign("warmup_frames", 0));
        td.SetAdaptationMode(d.rawConfign("adaptation_mode", 0));
		td.SetWorkgroupSizes(d.rawConfigs("wg_sizes", ""));
		td.qm_compare_frame = d.rawConfign("qm_compare_frame", -1);
		td.m_never_cancel = d.rawConfigb("never_cancel", false);
		td.m_report_filename = d.rawConfigs("report_file", "");
		td.m_report_interval = d.rawConfign("report_interval", 0);
		ret = true;
	}
	catch(const std::exception &e)
	{
		NGLOG_ERROR("failed to parse descriptor: %s", e.what());
	}
	return ret;
}


void ParseIOSOverrideRawconfig(const std::string &datarw_, tfw::Descriptor &desc);


bool GFXBench::parseConfig(const std::string &config, TestDescriptor &td)
{
    std::string error;
    tfw::Descriptor desc;
    tfw_descriptor_string_ = config;
    if(!tfw::Descriptor::fromJsonString(config, &desc, &error))
    {
        NGLOG_ERROR("failed to parse config: %s", error);
        return false;
    }
    test_id_ = desc.testId();

    data_ = desc.env().readPath();
    datarw_ = desc.env().writePath();
    width_ = desc.env().width();
    height_ = desc.env().height();

    td.SetFullscreen(desc.env().graphics().isFullScreen());

#ifndef OPT_COMMUNITY_BUILD
    ParseIOSOverrideRawconfig(datarw_, desc);
#endif

    parseTestDescriptor(desc, td);

    m_query_interval_ms=(int)desc.rawConfign("query_interval_ms",m_query_interval_ms);

	m_is_battery_test = strstr(test_id_.c_str(), "battery") != 0;
	m_selected_device_id = desc.env().graphics().deviceId();

    return true;
}


static inline bool myfn(QualityMatch* a, QualityMatch* b)
{
	return a->getScore() < b->getScore();
}


void GFXBench::storeResults()
{
#ifdef DISABLE_RESULTS
	{
		tfw::ResultGroup tfwResultGroup2;
		tfwResultGroup2.addResult(tfw::Result());
		result_ = tfwResultGroup2.toJsonString();
		return;
	}
#endif

	typedef std::vector<PrintableResult> Results;
	Results results;
	Results qm_results;
	glb_->result(&results);
	tfw::ResultGroup tfwResultGroup;

    std::string error;
    tfw::Descriptor desc;
    if(!tfw::Descriptor::fromJsonString(tfw_descriptor_string_, &desc, &error))
    {
        NGLOG_ERROR("failed to parse config: %s", error);
    }

	if (!m_qm_tests.empty())
	{
#ifndef NDEBUG
		for (int h = 0; h < m_qm_tests.size(); ++h)
		{
			INFO("score %s %f", m_qm_tests[h]->GetSetting().m_name.c_str(), m_qm_tests[h]->getScore());
		}
#endif
		auto largest = std::max_element(m_qm_tests.begin(), m_qm_tests.end(), myfn);


		(*largest)->result(&qm_results);

		for (int h = 0; h < m_qm_tests.size(); ++h)
		{
			delete m_qm_tests[h];
		}
		m_qm_tests.clear();
	}

	std::deque<double> queryTimesSeconds(queryTimes.begin(), queryTimes.end());
	for (std::deque<double>::iterator i = queryTimesSeconds.begin(); i != queryTimesSeconds.end();++i)
	{
		(*i) /= 1000;
	}

	if ((results.size() > 0) && (m_vsync_limit_count > 0) && ((results.begin()->m_uom == "frames") || (results.begin()->m_uom == "msec")) && (results.begin()->m_test_descriptor.GetScreenMode() == SMode_Onscreen))
	{
		if (m_vsync_limit_count*VSYNC_CHECK_INTERVAL_MS / (float)elapsedTime_ > VSYNC_MAXED_TRIGGER_TRESHOLD)
		{
			tfwResultGroup.flags().push_back("VSYNC_MAXED");
		}
		else
		{
			tfwResultGroup.flags().push_back("VSYNC_LIMITED");
		}
	}

	for (Results::const_iterator it = results.begin(); it != results.end(); ++it)
	{
		tfwResultGroup.addResult(tfw::Result());
		tfw::Result &tfwResult = tfwResultGroup.results()[tfwResultGroup.results().size() - 1];

        tfwResult.setDescriptor(desc);

        const auto is_limited =
            std::find_if(tfwResultGroup.flags().cbegin(), tfwResultGroup.flags().cend(), [](const std::string& flag) {
            return flag.find_first_of("VSYNC_") != std::string::npos;
                }) != std::end(tfwResultGroup.flags()) ? true : false;

        tfwResult.gfxResult().setIsVSyncLimited(is_limited);

#ifdef PRODUCT_VERSION
		std::stringstream ss;
		tfwResult.setBenchmarkVersion(STRINGIFY(PRODUCT_VERSION));
#endif
		tfwResult.setTestId(test_id_);
		tfwResult.setResultId(it->m_result_id);

		if (isCancelled())
		{
			tfwResult.setStatus(tfw::Result::CANCELLED);
		}
		else if (it->m_error != KCL::KCL_TESTERROR_NOERROR)
		{
			tfwResult.setStatus(tfw::Result::FAILED);
		}
		else
		{
			tfwResult.setStatus(tfw::Result::OK);
		}

		tfwResult.setErrorString(KCL::KCL_Status_To_Cstr(it->m_error));

		double printedScore = (it->m_score != it->m_score) ? -1 : it->m_score; // avoid NaN on output
        if ((printedScore >= DBL_MAX || printedScore <= -DBL_MAX) || (tfwResult.status() != tfw::Result::OK))// avoid inf. on output
		{
			printedScore = -1;
		}
		tfwResult.setScore(printedScore);
		tfwResult.setUnit(it->m_uom);

		ng::cpu_times time = loadTimer.elapsed();
		tfwResult.setLoadTime(time.wall * 1000); //convert to msec
		tfwResult.setElapsedTime((int)elapsedTime_);
		tfwResult.setMeasuredTime(it->m_test_length >= 0 ? it->m_test_length : (int)elapsedTime_);


        double printedFps = -1.0 ;

        if (m_is_battery_test)
        {
            tfwResult.gfxResult().setFrameCount(it->m_num_frames) ;
            printedFps = it->m_fps_d;
        }
        else
        {
            tfwResult.gfxResult().setFrameCount(glb_->getFrames() ? glb_->getFrames() : it->m_num_frames);
            printedFps = double(tfwResult.gfxResult().frameCount()) / tfwResult.elapsedTime() * 1000;
        }


		if (tfwResult.gfxResult().frameCount() == -1) printedFps = -1;
		printedFps = (printedFps != printedFps) ? -1 : printedFps; // avoid NaN on output
		if ((printedFps >= DBL_MAX || printedFps <= -DBL_MAX))// avoid inf. on output
		{
			printedFps = -1;
		}
		tfwResult.gfxResult().setFps(printedFps);
#ifdef ANDROID
		tfwResult.gfxResult().setEglConfigId( ((EGLGraphicsContext*)ctx_)->selectedConfig());
#else
		tfwResult.gfxResult().setEglConfigId(it->m_eglconfig_id);
#endif
		tfwResult.gfxResult().setVendor(m_vendor);
		tfwResult.gfxResult().setRenderer(m_renderer);
		tfwResult.gfxResult().setGraphicsVersion(m_graphics_version);
		tfwResult.gfxResult().setSurfaceWidth(it->m_viewport_width);
		tfwResult.gfxResult().setSurfaceHeight(it->m_viewport_height);
//		tfwResult.gfxResult().extraData() = it->m_extraData;

        if(!it->m_extraData.empty())
        {
            tfwResultGroup.charts().push_back(tfw::Chart());
            tfwResultGroup.charts().push_back(tfw::Chart());
            tfw::Chart &batteryChart = tfwResultGroup.charts()[tfwResultGroup.charts().size() - 2];
            tfw::Chart &performanceChart = tfwResultGroup.charts()[tfwResultGroup.charts().size() - 1];

            batteryChart.setChartID("Battery - Lifetime");
            batteryChart.setDomainAxis("");
            batteryChart.setSampleAxis("Charge (%)");

            performanceChart.setChartID("Battery - Performance");
            performanceChart.setDomainAxis("");
            performanceChart.setSampleAxis("Perf. (" + it->m_uom + ")");

            batteryChart.domain().setName("Iterations");
            batteryChart.domain().values().resize(it->m_extraData.size());
            batteryChart.values().resize(1);
            batteryChart.values()[0].setName("Battery");
            batteryChart.values()[0].values().resize(it->m_extraData.size());

            performanceChart.domain().setName("Iterations");
            performanceChart.domain().values().resize(it->m_extraData.size());
            performanceChart.values().resize(1);
            performanceChart.values()[0].setName("Performance");
            performanceChart.values()[0].values().resize(it->m_extraData.size());

            for(int index = 0; index < it->m_extraData.size(); ++index)
            {
                std::string s = it->m_extraData[index];
                std::size_t separator = s.find("|");

                batteryChart.domain().values()[index] = index+1;
                batteryChart.values()[0].values()[index] = atof(s.substr(0, separator).c_str())*100;

                performanceChart.domain().values()[index] = index+1;
                performanceChart.values()[0].values()[index] = atof(s.substr(separator + 1, s.length() - separator - 1).c_str());
            }

        }

		ss.clear();
		if (!qm_results.empty())
		{
			ss << qm_results[0].m_score;
			tfwResult.gfxResult().extraData().push_back(std::string("qm_score:") + ss.str());
		}
	}

	result_nocharts_ = tfwResultGroup.toJsonString();

	{
        if (queryTimesSeconds.size()>0)
        {
            queryTimesSeconds.pop_front();
            renderTimesAvg.pop_front();
            renderTimesMax.pop_front();
            renderTimesMin.pop_front();
        }

		tfwResultGroup.charts().push_back(tfw::Chart());
		tfw::Chart &chart = tfwResultGroup.charts()[tfwResultGroup.charts().size() - 1];
		chart.setChartID("Frametimes");
		chart.setDomainAxis("");
		chart.setSampleAxis("Frametimes (ms)");

		chart.domain().setName("Time (s)");
		chart.domain().values().resize(queryTimesSeconds.size());
		chart.domain().values().assign(queryTimesSeconds.begin(), queryTimesSeconds.end());

		chart.values().resize(3);
		chart.values()[0].setName("Minimum");
		chart.values()[0].values().resize(renderTimesMin.size());
		chart.values()[0].values().assign(renderTimesMin.begin(), renderTimesMin.end());
		chart.values()[1].setName("Maximum");
		chart.values()[1].values().resize(renderTimesMax.size());
		chart.values()[1].values().assign(renderTimesMax.begin(), renderTimesMax.end());
		chart.values()[2].setName("Average");
		chart.values()[2].values().resize(renderTimesAvg.size());
		chart.values()[2].values().assign(renderTimesAvg.begin(), renderTimesAvg.end());
	}

	if (CPUtemps.size() > 0)
	{
		tfwResultGroup.charts().push_back(tfw::Chart());
		tfw::Chart &chart = tfwResultGroup.charts()[tfwResultGroup.charts().size() - 1];
		chart.setChartID("Temperature");
		chart.setDomainAxis("");
		chart.setSampleAxis("Temperature (C)");

		chart.domain().setName("Time (s)");
		chart.domain().values().resize(queryTimesSeconds.size());
		chart.domain().values().assign(queryTimesSeconds.begin(), queryTimesSeconds.end());

		chart.values().resize(1);
		chart.values()[0].setName("Temperature (C)");
		chart.values()[0].values().resize(CPUtemps.size());
		chart.values()[0].values().assign(CPUtemps.begin(), CPUtemps.end());
	}

	if (CPUfreqs.size() > 0 || GPUfreqs.size() > 0)
	{
		tfwResultGroup.charts().push_back(tfw::Chart());
		tfw::Chart &chart = tfwResultGroup.charts()[tfwResultGroup.charts().size() - 1];
		chart.setChartID("Performance");
		chart.setDomainAxis("");
		chart.setSampleAxis("Clock speed (MHz)");

		chart.domain().setName("Time (s)");
		chart.domain().values().resize(queryTimesSeconds.size());
		chart.domain().values().assign(queryTimesSeconds.begin(), queryTimesSeconds.end());

		chart.values().resize(core_count + (GPUfreqs.size() > 0 ? 1 : 0));

		if (CPUfreqs.size() > 0)
		{
			for (int core = 0; core < core_count; ++core)
			{
				char cpuname[10];
				sprintf(cpuname, "CPU%d", core + 1);
				chart.values()[core].setName(cpuname);
				size_t CPUfreqsSize = CPUfreqs.size() / core_count;
				chart.values()[core].values().resize(CPUfreqsSize);
				for (int i = 0; i < CPUfreqsSize; ++i)
				{
					chart.values()[core].values()[i] = CPUfreqs[i*core_count + core];
				}
			}
		}

		if (GPUfreqs.size() > 0)
		{
			chart.values()[core_count].setName("GPU");
			chart.values()[core_count].values().assign(GPUfreqs.begin(), GPUfreqs.end());
		}
	}

	result_ = tfwResultGroup.toJsonString();
}

void GFXBench::PrintDefaultFBOInfo()
{
     const char* info_strings[] =
     {
        "Default FBO red bits: %s",
        "Default FBO green bits: %s",
        "Default FBO blue bits: %s",
        "Default FBO alpha bits: %s",
        "Default FBO depth bits: %s",
        "Default FBO stencil bits: %s"
     };

#if defined USE_ANY_GL
#if defined HAVE_GLES3 || defined HAVE_GLEW
    if (m_gte->IsGraphicsContextGLorES())
    {
        if (ctx_->versionMajor() >= 3)
        {
            GLenum attachments[6];
            if (ctx_->type() == GraphicsContext::GLES)
            {
                // OpenGL ES
                attachments[0] = GL_BACK;
                attachments[1] = GL_BACK;
                attachments[2] = GL_BACK;
                attachments[3] = GL_BACK;
                attachments[4] = GL_DEPTH;
                attachments[5] = GL_STENCIL;
            }
            else
            {
                // Desktop GL
                attachments[0] = GL_BACK_LEFT;
                attachments[1] = GL_BACK_LEFT;
                attachments[2] = GL_BACK_LEFT;
                attachments[3] = GL_BACK_LEFT;
                attachments[4] = GL_DEPTH;
                attachments[5] = GL_STENCIL;
            }

            GLenum pnames[6];
            pnames[0] = GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE;
            pnames[1] = GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE;
            pnames[2] = GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE;
            pnames[3] = GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE;
            pnames[4] = GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE;
            pnames[5] = GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE;

            GLint bits[6];
            for (int i = 0; i < 6; i++)
            {
                bits[i] = -1;

                GLint type = -1;
                glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, attachments[i], GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
                if (type == GL_NONE)
                {
                    bits[i] = 0;
                    continue;
                }

                glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, attachments[i], pnames[i], &bits[i]);
            }

            NGLOG_INFO("Default FBO bits:        RGBA: %s %s %s %s - DS: %s %s", bits[0], bits[1], bits[2], bits[3], bits[4], bits[5]);
        }
        else
        {
             // ES 2.0 implementation
        }
    }
#endif
#endif
}

#if defined _WIN32 && defined _DEBUG && defined HAVE_GLEW && USE_OPENGL_DEBUG

void FormatDebugOutputARB(char outStr[], size_t outStrSize, GLenum source, GLenum type,
    GLuint id, GLenum severity, const char *msg)
{
    char sourceStr[32];
    const char *sourceFmt = "UNDEFINED(0x%04X)";
    switch(source)

    {
    case GL_DEBUG_SOURCE_API_ARB:             sourceFmt = "API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:   sourceFmt = "WINDOW_SYSTEM"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB: sourceFmt = "SHADER_COMPILER"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:     sourceFmt = "THIRD_PARTY"; break;
    case GL_DEBUG_SOURCE_APPLICATION_ARB:     sourceFmt = "APPLICATION"; break;
    case GL_DEBUG_SOURCE_OTHER_ARB:           sourceFmt = "OTHER"; break;
    }

    _snprintf(sourceStr, 32, sourceFmt, source);

    char typeStr[32];
    const char *typeFmt = "UNDEFINED(0x%04X)";
    switch(type)
    {

    case GL_DEBUG_TYPE_ERROR_ARB:               typeFmt = "ERROR"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB: typeFmt = "DEPRECATED_BEHAVIOR"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:  typeFmt = "UNDEFINED_BEHAVIOR"; break;
    case GL_DEBUG_TYPE_PORTABILITY_ARB:         typeFmt = "PORTABILITY"; break;
    case GL_DEBUG_TYPE_PERFORMANCE_ARB:         typeFmt = "PERFORMANCE"; break;
    case GL_DEBUG_TYPE_OTHER_ARB:               typeFmt = "OTHER"; break;
    }
    _snprintf(typeStr, 32, typeFmt, type);


    char severityStr[32];
    const char *severityFmt = "UNDEFINED";
    switch(severity)
    {
    case GL_DEBUG_SEVERITY_HIGH_ARB:        severityFmt = "HIGH";   break;
    case GL_DEBUG_SEVERITY_MEDIUM_ARB:      severityFmt = "MEDIUM"; break;
    case GL_DEBUG_SEVERITY_LOW_ARB:         severityFmt = "LOW"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:    severityFmt = "NOTIFICATION"; break;
    }

    _snprintf(severityStr, 32, severityFmt, severity);

    _snprintf(outStr, outStrSize, "ARB Debug: %s [source=%s type=%s severity=%s id=%d]",
        msg, sourceStr, typeStr, severityStr, id);
}

void __stdcall DebugCallbackARB(GLenum source, GLenum type, GLuint id, GLenum severity,GLsizei length, const GLchar *message, const void *userParam)
{
    if(id == 131185)
        return;

    (void)length;
    FILE *outFile = (FILE*)userParam;
	char finalMessage[OPENGL_DEBUG_MESSAGE_LENGTH];
	FormatDebugOutputARB(finalMessage, OPENGL_DEBUG_MESSAGE_LENGTH, source, type, id, severity, message);
    fprintf(outFile, "%s\n", finalMessage);
}


#endif



#if defined _WIN32 && defined _DEBUG && defined HAVE_GLEW && USE_AMD_OPENGL_DEBUG

void FormatDebugOutputAMD(char outStr[], size_t outStrSize, GLenum category, GLuint id,
                         GLenum severity, const char *msg)
{
    char categoryStr[32];
    const char *categoryFmt = "UNDEFINED(0x%04X)";
    switch(category)
    {
    case GL_DEBUG_CATEGORY_API_ERROR_AMD:          categoryFmt = "API_ERROR"; break;
    case GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD:      categoryFmt = "WINDOW_SYSTEM"; break;
    case GL_DEBUG_CATEGORY_DEPRECATION_AMD:        categoryFmt = "DEPRECATION"; break;
    case GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD: categoryFmt = "UNDEFINED_BEHAVIOR"; break;
    case GL_DEBUG_CATEGORY_PERFORMANCE_AMD:        categoryFmt = "PERFORMANCE"; break;
    case GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD:    categoryFmt = "SHADER_COMPILER"; break;
    case GL_DEBUG_CATEGORY_APPLICATION_AMD:        categoryFmt = "APPLICATION"; break;
    case GL_DEBUG_CATEGORY_OTHER_AMD:              categoryFmt = "OTHER"; break;
    }
    _snprintf(categoryStr, 32, categoryFmt, category);

    char severityStr[32];
    const char *severityFmt = "UNDEFINED";
    switch(severity)
    {
    case GL_DEBUG_SEVERITY_HIGH_AMD:   severityFmt = "HIGH";   break;
    case GL_DEBUG_SEVERITY_MEDIUM_AMD: severityFmt = "MEDIUM"; break;
    case GL_DEBUG_SEVERITY_LOW_AMD:    severityFmt = "LOW";    break;
    }
    _snprintf(severityStr, 32, severityFmt, severity);

    _snprintf(outStr, outStrSize, "AMD Debug: %s [category=%s severity=%s id=%d]",
        msg, categoryStr, severityStr, id);
}



void __stdcall DebugCallbackAMD(GLuint id, GLenum category, GLenum severity, GLsizei length,const GLchar *message, GLvoid *userParam)
{
    (void)length;
    FILE *outFile = (FILE*)userParam;
    char finalMsg[256];
    FormatDebugOutputAMD(finalMsg, 256, category, id, severity, message);
    fprintf(outFile, "%s\n", finalMsg);
}

#endif
