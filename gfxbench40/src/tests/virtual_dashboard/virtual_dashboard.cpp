/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "schemas/result.h"
#include "virtual_dashboard.h"
#include "vdashboard.h"
#include "schemas/descriptors.h"
#include "graphics/graphicscontext.h"
#include "ng/timer.h"
#include "ng/json.h"
#include "opengl/ext.h"

#include "platform.h"

#include "opengl/glb_opengl_state_manager.h"
#include "opengl/ext.h"
#include "opengl/fbo.h"

#include "krl_material.h"

using namespace KCL;



PlanarMap* PlanarMap::Create(int w, int h, const char *name)
{
	return NULL;
}

virtual_dashboard::virtual_dashboard() : m_test_time( 0), m_frames( 0), m_is_endless( false), elapsedTime_( 0)
{
    loadTimer.start();
	m_scene = new VirtualDashboardScene();
}


virtual_dashboard::~virtual_dashboard()
{
	delete m_scene;
}


bool virtual_dashboard::parseConfig(const std::string &config, tfw::Descriptor &td)
{
    std::string error;
	if(!tfw::Descriptor::fromJsonString(config, &td, &error))
    {
        //NGLOG_ERROR("failed to parse config: %s", error);
        return false;
    }

    return true;
}

bool CreateGlobals(const char* data, const char* datarw,GraphicsContext* const graphics_context)
{
   	KCL::g_os = KCL::OS::CreateI("");

	KCL::AssetFile::SetDataPath(data);
	KCL::AssetFile::SetRWDataPath(datarw);

	GLB::g_extension = new GLB::Extension(graphics_context);
	GLB::g_extension->init ();

	KCL::Initialize(false);
	GLB::FBO::CreateGlobalFBO();
	GLB::OpenGLStateManager::Reset();
	return true;
}


void DestroyGlobals()
{
	GLB::FBO::DeleteGlobalFBO();
	KCL::Release();
	GLB::OpenGLStateManager::Reset();
	delete GLB::g_extension;
	GLB::g_extension = 0;
	KCL::OS::DestroyI(&KCL::g_os);
	KCL::g_os = 0;
}


bool virtual_dashboard::init ()
{
	float m_loadingProgressPtr = 0.0f;
	m_viewport_width = 800;
	m_viewport_height = 600;
	m_color_bpp = 24;
	m_depth_bpp = 16;
	m_fsaa = 0;
	std::string m_scenefile = "vdashboard/scene_vdb.xml";
	tfw::Descriptor desc;

	ctx_->makeCurrent();

	parseConfig( config(), desc);

	{
#ifdef HAVE_GLEW
		glewInit();
#endif
		bool ok = false;
		if (!CreateGlobals(desc.env().readPath().c_str(), desc.env().writePath().c_str(), ctx_))
		{
			return false;
		}

		GLB::g_extension->disableFeature(GLB::GLBFEATURE_sampler_object);
		GLB::g_extension->disableFeature(GLB::GLBFEATURE_invalidate_framebuffer);
		GLB::g_extension->disableFeature(GLB::GLBFEATURE_es3_compatibility);
		GLB::g_extension->disableExtension(GLB::GLBEXT_es3_compatibility);


	}

	m_viewport_width = desc.env().width();
	m_viewport_height = desc.env().height();

	m_test_time = desc.rawConfign("play_time");
	m_is_endless = desc.rawConfigb("endless", false);

	m_scene->SetProgressPtr( &m_loadingProgressPtr);

	m_scene->setTextureCompressionType( "888");
	m_scene->SetZPrePass( false);

	GLB::FBO_COLORMODE color_mode;
	GLB::FBO_DEPTHMODE depth_mode;
	KCL::KCL_Status result = KCL_TESTERROR_NOERROR;

	try
	{
		int max_uniform_num = 1024;

#if (defined GL_MAX_VERTEX_UNIFORM_VECTORS) && (!defined HAVE_DX)
		glGetIntegerv( GL_MAX_VERTEX_UNIFORM_VECTORS, &max_uniform_num);
#endif

		m_scene->Resize( m_viewport_width , m_viewport_height);

		result = m_scene->Process_Load( max_uniform_num, m_scenefile.c_str());

		if(result != KCL_TESTERROR_NOERROR)
		{
			return result;
		}

		if( m_color_bpp >= 24)
		{
			color_mode = GLB::RGBA8888_Linear;
		}
		else
		{
			color_mode = GLB::RGBA5551_Linear;
		}

		if( m_depth_bpp >= 24)
		{
			depth_mode = GLB::DEPTH_24_RB;
		}
		else
		{
			depth_mode = GLB::DEPTH_16_RB;
		}

        m_scene->SetForceHighp( false);

		m_scene->CreateVBOPool();
		result = m_scene->Process_GL( color_mode, depth_mode, m_fsaa);

		if( m_test_time == -1)
		{
			m_test_time = m_scene->m_play_time;
		}

		if(result != KCL_TESTERROR_NOERROR)
		{
			return result;
		}

	}
	catch(std::exception &ex)
	{
		INFO("%s", ex.what());
		return false;//GLB_TESTERROR_OUT_OF_MEMORY;
	}

	return true;//result;
}


void virtual_dashboard::run ()
{
    loadTimer.stop();

	ng::cpu_timer animation_timer;
	ng::cpu_timer test_timer;

	animation_timer.start();
	test_timer.start();

	while (!isCancelled())
	{
		double animation_time = animation_timer.elapsed().wall * 1000;
		double test_time = test_timer.elapsed().wall * 1000;

		if( animation_time >= m_scene->m_play_time)
		{
			animation_timer.set_elapsed(ng::cpu_times());
			animation_timer.start();
		}

		if( !m_is_endless)
		{
			if( test_time >= m_test_time)
			{
				break;
			}
		}

		require(ctx_->makeCurrent());

		m_scene->m_animation_time = animation_time;

		m_scene->Animate();

		m_scene->Render();

		require(ctx_->swapBuffers());

		m_frames++;
	}

	glFinish();

	elapsedTime_ = test_timer.elapsed().wall * 1000;

	storeResults();

	m_scene->DeleteShaders();

	m_scene->DeleteVBOPool();

	delete m_scene;

	m_scene = 0;

	DestroyGlobals();

	ctx_->detachThread();
}


void virtual_dashboard::storeResults()
{
	float fps = 1000.0 * (float)m_frames / (float)elapsedTime_;
	tfw::ResultGroup tfwResultGroup;
	tfw::Result tfwResult;

	tfwResult.setTestId( name());
	//tfwResult.setResultId( 0);

	if (isCancelled())
	{
		tfwResult.setStatus(tfw::Result::CANCELLED);
	}
	//else if(it->m_error != GLB_TESTERROR_NOERROR)
	//{
	//	tfwResult.setStatus(tfw::Result::FAILED);
	//}
	else
	{
		tfwResult.setStatus(tfw::Result::OK);
	}

	tfwResult.setErrorString( "");

	tfwResult.setScore( fps);
	tfwResult.setUnit( "fps");

	ng::cpu_times time = loadTimer.elapsed();
	tfwResult.setLoadTime(time.wall * 1000); //convert to msec
	tfwResult.setElapsedTime((int)elapsedTime_);
	tfwResult.setMeasuredTime((int)elapsedTime_);
	//tfwResult.setMeasuredTime(it->m_test_length>=0?it->m_test_length:(int)elapsedTime_);

	tfwResult.gfxResult().setFps(fps);
	tfwResult.gfxResult().setFrameCount( m_frames);
//#ifdef ANDROID
//	tfwResult.gfxResult().setEglConfigId( ((EGLGraphicsContext*)ctx_)->selectedConfig());
//#else
//	tfwResult.gfxResult().setEglConfigId(it->m_eglconfig_id);
//#endif

	//tfwResult.gfxResult().setRenderer(renderer_);
	//tfwResult.gfxResult().setGraphicsVersion(graphics_version_);
	tfwResult.gfxResult().setSurfaceWidth( m_viewport_width);
	tfwResult.gfxResult().setSurfaceHeight( m_viewport_height);
	//tfwResult.gfxResult().extraData() = it->m_extraData;

	tfwResultGroup.addResult(tfwResult);

	result_ = tfwResultGroup.toJsonString();
}

CREATE_FACTORY( virtual_dashboard, virtual_dashboard)
