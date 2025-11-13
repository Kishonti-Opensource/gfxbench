/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "test_base.h"
#include "kcl_image.h"
#include "kcl_io.h"
#include "kcl_camera2.h"

#include "sstream"
#include "time.h"

#if defined USE_ANY_GL
#include "opengl/fbo.h"
    #ifdef GL_WRAPPER_STATS
        #include "opengl/gl_wrapper/gl_wrapper.h"
    #endif
#elif defined HAVE_DX
#include "d3d11/fbo3.h"
#elif defined USE_METAL
#include "metal/fbo.h"
#include "mtl_types.h"
#endif

#define FINISH_FBO_WIDTH 1
#define FINISH_FBO_HEIGHT 1

using namespace KCL;

namespace GLB
{


void saveScreenshotImage(const char* fileName)
{
	KCL::Image img;
	if (GLB::FBO::GetScreenshotImage(img) != 0)
	{
		return;
	}

	img.flipY();
	char name[1024];

	sprintf(name, "%s%s.tga", KCL::File::GetDataRWPath().c_str(), fileName);

    KCL::File::MkDir(KCL::File::GetDataRWPath().c_str());

	img.saveTga(name, true);

	////sprintf(name, "%s/%s.tga", KCL::File::GetDataRWPath().c_str(), fileName);
	////img.saveTga(name, true);

	////sprintf(name, "%s/%s.png", KCL::File::GetDataRWPath().c_str(), fileName);
	////img.savePng(name, true);
}
	
void TestBase::saveStatistics()
{ 
    std::stringstream ss;
	ss << "statistics_" << (int)time(0) << ".txt";

    m_statistics_array->saveInTextFile(ss.str().c_str());
}

int TestBase::result(std::vector<PrintableResult> *results) const
{
	PrintableResult *resultArray = 0;
	
	int size = 0;
	getTestResult(&resultArray, &size);
	for (int i = 0; i < size; ++i)
	{
		results->push_back(resultArray[i]);
	}
	delete [] resultArray;
	return size;
}

inline void TestBase::getTestResult(PrintableResult** results, int* count) const
{
	if (m_settings->m_frame_step_time > 0)
	{
		*count = 2;
		if (!results)
		{
			// The caller only queries the number of test results
			return;
		}
		
		bool isUsingFramesAsUom = (strcmp(getUom(), "frames") == 0);
		bool isFpsScore = (strcmp(getUom(), "fps") == 0);
		bool isBattery = (strstr(m_settings->m_name.c_str(), "battery") != NULL);

		PrintableResult* result = new PrintableResult[2];
		result[0] = PrintableResult(
			getTextureType(),
			(float(m_frames) / float(m_elapsedtime) * 1000.0f),
			KCL_Status_To_Cstr(m_runtime_error),
			(isWarmup() || (m_time == 0) || (!isUsingFramesAsUom)) ? (isFpsScore ? getScore() : -1.0f) : (float(m_frames) / float(m_elapsedtime) * 1000.0f),
			"fps",
			false,
			isWarmup(),
			m_settings->m_name + "_fps",
			GetFrameStepTime(),
			m_settings->GetScreenMode() ? m_settings->GetTestWidth() : getViewportWidth(),
			m_settings->GetScreenMode() ? m_settings->GetTestHeight() : getViewportHeight(),
			getFrames(),
			-1,
			-1,
			m_runtime_error,
			std::vector<std::string>());
		result[0].m_test_descriptor = this->GetSetting();

		result[1] = PrintableResult(
			getTextureType(),
			m_elapsedtime,
			KCL_Status_To_Cstr(m_runtime_error),
			(isWarmup() || (m_time == 0) || (!isUsingFramesAsUom)) ? (isFpsScore ? getScore() : -1.0f) : (float(m_frames) / float(m_elapsedtime) * 1000.0f),
			"msec",
			false,
			isWarmup(),
			m_settings->m_name + "_time",
			GetFrameStepTime(),
			m_settings->GetScreenMode() ? m_settings->GetTestWidth() : getViewportWidth(),
			m_settings->GetScreenMode() ? m_settings->GetTestHeight() : getViewportHeight(),
			getFrames(),
			-1,
			-1,
			m_runtime_error,
			std::vector<std::string>());
		result[1].m_test_descriptor = this->GetSetting();

		*results = result;
	}
	else
	{
		*count = 1;
		if (!results)
		{
			// The caller only queries the number of test results
			return;
		}

		bool isUsingFramesAsUom = (strcmp(getUom(), "frames") == 0);
		bool isFpsScore = (strcmp(getUom(), "fps") == 0);
		bool isBattery = (strstr(m_settings->m_name.c_str(), "battery") != NULL);

		PrintableResult* result = new PrintableResult[1];
		result[0] = PrintableResult(
			getTextureType(),
			(isUsingFramesAsUom && !isBattery) ? getNormalizedScore() : getScore(), // Normalize the frame based score
			KCL_Status_To_Cstr(m_runtime_error),
			(isWarmup() || (m_time == 0) || (!isUsingFramesAsUom)) ? (isFpsScore ? getScore() : -1.0f) : (float(m_frames) / float(m_elapsedtime) * 1000.0f),
			getUom(),
			false,
			isWarmup(),
			m_settings->m_name,
			GetFrameStepTime(),
			m_settings->GetScreenMode() ? m_settings->GetTestWidth() : getViewportWidth(),
			m_settings->GetScreenMode() ? m_settings->GetTestHeight() : getViewportHeight(),
			getFrames(),
			-1,
			-1,
			m_runtime_error,
			std::vector<std::string>());
		result[0].m_test_descriptor = this->GetSetting();

		*results = result;
	}
}

float TestBase::getNormalizedScore() const
{    
    // Calculate the frame count based normalized score
    float test_length = m_settings->m_play_time - m_settings->m_start_animation_time;
    if (test_length > 0.0f && m_elapsedtime > 0)
    {
        return getScore() * test_length / float(m_elapsedtime); 
    }
    return getScore();
}

TestBase::TestBase (const GlobalTestEnvironment* const gte): 
	TestBase0(*gte->GetTestDescriptor()),
	m_window_width(0),
	m_window_height(0),
	m_offscreenmanager(0),
	m_statistics_array( NewStatisticsArray(*gte->GetTestDescriptor())),
	m_time(0),
	m_runtime_error( KCL_TESTERROR_NOERROR),
	m_screenRefreshNeeded(false),
    m_offscreen_virtual_res(false),
	m_gte(gte) 
#if defined HAVE_DX
    ,m_pEventQuery(NULL)
#endif
#if defined USE_ANY_GL
    ,m_finish_fbo(NULL)
#endif
{
}


inline KCL::KCL_Status TestBase::init0()
{
	m_window_width=m_settings->m_viewport_width;
	m_window_height=m_settings->m_viewport_height;    

    KCL::Camera2::enable_orrientation_rotation = true;
    m_offscreen_virtual_res = m_settings->GetScreenMode() == SMode_Onscreen && m_settings->m_virtual_resolution;
#ifdef ANDROID
    // On Android we use hardware scaler instead of Offscreen Manager for custom resolution
    if (m_offscreen_virtual_res)
    {
        SetSetting().m_viewport_width = m_settings->m_test_width;
		SetSetting().m_viewport_height = m_settings->m_test_height;
        m_offscreen_virtual_res = false;
        KCL::Camera2::enable_orrientation_rotation = false;
    }  
#endif

    if (m_settings->GetScreenMode() != SMode_Onscreen || m_offscreen_virtual_res)
	{
		SetSetting().m_viewport_width = m_settings->m_test_width;
		SetSetting().m_viewport_height = m_settings->m_test_height;

		m_offscreenmanager = OffscreenManager::Create(m_gte, m_settings->m_test_width,m_settings->m_test_height);

		int err = m_offscreenmanager->Init(m_window_width,m_window_height,*m_settings);
		if (err)
		{
			if (m_settings->m_fsaa)
			{
				SetRuntimeError(KCL_TESTERROR_OFFSCREEN_NOT_SUPPORTED_IN_MSAA);
				return KCL_TESTERROR_OFFSCREEN_NOT_SUPPORTED_IN_MSAA;
			} else
			{
				SetRuntimeError(KCL_TESTERROR_OFFSCREEN_NOT_SUPPORTED);
				return KCL_TESTERROR_OFFSCREEN_NOT_SUPPORTED;
			}
		}
        KCL::Camera2::enable_orrientation_rotation = false;
	}   
	
    KCL::KCL_Status testError;
    try {
        testError = init();
    } catch (const KCL::IOException&) {
        testError = KCL_TESTERROR_FILE_NOT_FOUND;
    }
	SetRuntimeError(testError);

	m_last_swap_buffer_time = KCL::g_os->GetTimeMilliSec();

#if defined HAVE_DX
    CD3D11_QUERY_DESC queryDesc(D3D11_QUERY_EVENT);
    DX::getDevice()->CreateQuery(&queryDesc, &m_pEventQuery);
#endif

#if defined USE_ANY_GL
    if (m_offscreenmanager)
    {
        m_finish_fbo = new FBO(FINISH_FBO_WIDTH, FINISH_FBO_HEIGHT, 0, RGB565_Linear, DEPTH_None, "::FinishFBO");
    }
#endif

	return testError;
}

inline bool TestBase::render0(const char* screenshotName)
{   
	if (m_settings->GetScreenMode() != SMode_Onscreen || m_offscreen_virtual_res)
	{
#ifdef GL_WRAPPER_STATS
        GetGLWrapper()->PauseMeasurement();
#endif
		m_offscreenmanager->PreRender();
#ifdef GL_WRAPPER_STATS
        GetGLWrapper()->ResumeMeasurement();
#endif

	}

	resetEnv();
	render();

	if (screenshotName != NULL)
	{
		saveScreenshotImage(screenshotName);
	}

	if (m_settings->GetScreenMode() == SMode_Onscreen && !m_offscreen_virtual_res)
	{
		m_screenRefreshNeeded = true;
	}
	else
	{
		double now = KCL::g_os->GetTimeMilliSec();
		double elapsed = now - m_last_swap_buffer_time;

		bool force_swap_buffer = (elapsed > 5000);
#ifdef GL_WRAPPER_STATS
        GetGLWrapper()->PauseMeasurement();
#endif  
		m_offscreenmanager->PostRender(m_time,m_frames,m_window_width,m_window_height, force_swap_buffer);
#ifdef GL_WRAPPER_STATS
        GetGLWrapper()->ResumeMeasurement();
#endif

		m_screenRefreshNeeded = m_offscreenmanager->IsSwapBufferNeeded() || force_swap_buffer || m_offscreen_virtual_res;
		if (m_screenRefreshNeeded)
		{
			m_last_swap_buffer_time = now;
		}
	}

	++m_frames;

    //update key states
//#if defined WIN32
    memcpy(m_keys_last, m_keys_current, sizeof(m_keys_last));
    memcpy(m_buttons_last, m_buttons_current, sizeof(m_buttons_last));
//#endif

	return m_screenRefreshNeeded;
}

void TestBase::finishTest()
{
    // Ensure to swap the buffers and render the last frames when using Offscreen Manager
    bool offscreen_mgr_active = m_settings->GetScreenMode() != SMode_Onscreen || m_offscreen_virtual_res;
    if (m_settings->GetScreenMode() != SMode_Onscreen && !m_offscreen_virtual_res && m_offscreenmanager)
    {
        if (!m_screenRefreshNeeded)
        {
            // Offscreen manager has some frames in it's queue. Render and show them
            m_offscreenmanager->RenderLastFrames(m_window_width, m_window_height);
#ifndef SIGNIFICANT_FRAME_MODE 
            m_gte->GetGraphicsContext()->swapBuffers();
#else
            glFinish();
#endif
        }
    }

#if defined HAVE_DX
    assert(m_pEventQuery);
    DX::getContext()->End(m_pEventQuery); // insert a fence into the pushbuffer
    while(DX::getContext()->GetData( m_pEventQuery, NULL, 0, 0 ) == S_FALSE ) {} // spin until every event is finished
#elif defined USE_ANY_GL
    // Finish all GL commands, in ES2 compatible way
    if (m_offscreenmanager && m_finish_fbo)
    {
        FBO::bind(m_finish_fbo);
        glViewport(0, 0, FINISH_FBO_WIDTH, FINISH_FBO_HEIGHT);

        m_offscreenmanager->RenderCurrentMosaic();

        GLubyte values[FINISH_FBO_WIDTH * FINISH_FBO_HEIGHT * 4];
        glReadPixels(0, 0, FINISH_FBO_WIDTH, FINISH_FBO_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, values);

        //KCL::uint32 center = (FINISH_FBO_WIDTH * FINISH_FBO_HEIGHT / 4) * 4;
        //NGLOG_INFO("\nPixels read: %s %s %s %s\n", int(values[center]), int(values[center + 1]), int(values[center + 2]), int(values[center + 3]));
    }
    else
    {
        glFinish();
    }
#elif defined USE_METAL
    MetalRender::Finish();
#endif
}

TestBase::~TestBase()
{
	if (m_offscreenmanager) m_offscreenmanager->Clear();
	delete m_offscreenmanager;
	DeleteStatisticsArray(m_statistics_array);

#if defined HAVE_DX
    if(m_pEventQuery)
    {
        m_pEventQuery->Release();
        m_pEventQuery = NULL;
    }
#endif

#if defined USE_ANY_GL
    delete m_finish_fbo;
#endif
}

void TestBase::initEffects (int , const char *const*)
{
}

bool TestBase::isLowLevel() const 
{
	return true; 
}

void TestBase::resetEnv()
{
}


bool TestBase::resize(int width, int height)
{
	return false;
}

KCL::uint32 TestBase::getRenderedVerticesCount() const
{
	return 0;
}


KCL::uint32 TestBase::getRenderedTrianglesCount() const
{
	return 0;
}


KCL::uint32 TestBase::getDrawCalls() const
{
	return 0;
}


KCL::int32 TestBase::getUsedTextureCount() const
{
	return -1;
}


KCL::int32 TestBase::getSamplesPassed() const //needs occlusion query, not always available
{
	return -1;
}


float TestBase::getPixelCoverage() const //needs occlusion query, not always available
{
	return -1.0f;
}


KCL::int32 TestBase::getInstructionCount() const //needs some tool, e.g. cgc.exe, not always available
{
	return -1;
}


void TestBase::logStatistics(double frameTime)
{
	StatisticsSample statisticsSample;

	statisticsSample.SetFrameTime(frameTime);
	//statisticsSample.SetAvgFps(avgFPS);

	statisticsSample.SetVertexCount (getRenderedVerticesCount());
	statisticsSample.SetPrimitiveCount (getRenderedTrianglesCount());
	statisticsSample.SetDrawCalls (getDrawCalls());

	statisticsSample.SetUsedTextureCount (getUsedTextureCount());
    statisticsSample.SetPixelCoverage (getPixelCoverage());
	
    //only allow occlusion-based stats if coverage measurements are off - they turn off depth test for solids
    if(statisticsSample.GetPixelCoverage() == 0.0f)
    {
        statisticsSample.SetSamplesPassed (getSamplesPassed());
	    statisticsSample.SetInstructionCount (getInstructionCount());
    }

	m_statistics_array->m_data.push_back(statisticsSample);
}

}