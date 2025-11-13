/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "engine.h"
#include "platform.h"
#include "../gfxbench/global_test_environment.h"

#ifdef USE_METAL
#include "../gfx3_0/high_level_tests_3/metal/mtl_factories.h"
#endif

#if defined HAVE_DX
#include "d3d11/dxb_scene_27.h"
#include "d3d11/dxb_scene_30.h"
#elif defined USE_ANY_GL

#ifdef OPT_TEST_GFX2
#include "../gfx3_0/high_level_tests_3/opengl/glb_scene.h"
#endif 

#ifdef OPT_TEST_GFX30
#include "../gfx3_0/high_level_tests_3/opengl/glb_scene_opengl.h"
#endif 

#ifdef OPT_TEST_GFX31
#include "../gfx3_1/high_level_tests_3/opengl/glb_scene_opengl31.h"
#endif

#ifdef OPT_TEST_GFX40
#include "opengl/glb_scene_opengl4.h"
#endif

#endif

#include "krl_scene.h"
#include "glb_kcl_adapter.h"

#include <string>
#include <stdexcept>

using namespace GLB;

static int lastTime = 0;
static double animationMultiplier = 1.0f;
static bool animationPause = false;
static double animationTime = 0.0f;
static double multiplier = 1;

Engine2::Engine2(const GlobalTestEnvironment* const gte) : TestBase(gte), m_scene(NULL)
{
#if defined HAVE_DX
	if( gte->IsEngine("Engine2") )
	{
		m_scene = new DXB_Scene_27;
	}
	else
	{
		m_scene = new DXB_Scene_30;
	}
#elif USE_METAL
#ifdef OPT_TEST_GFX31
    if ( gte->IsEngine("Engine31") )
    {
        if ( gte->IsGraphicsContextMetal() )
        {
            m_scene = MetalRender::CreateMTLScene31(gte) ;
			m_scene->SetTestId(gte->GetTestId());
        }
    }
#endif
    if ( gte->IsEngine("Engine3") )
    {
        if ( gte->IsGraphicsContextMetal() )
        {
            m_scene = MetalRender::CreateMTLScene30(gte) ;
			m_scene->SetTestId(gte->GetTestId());
        }
    }
    if ( gte->IsEngine("Engine2"))
    {
        if ( gte->IsGraphicsContextMetal() )
        {
            m_scene = MetalRender::CreateMTLScene27(gte) ;
        }
    }
#ifdef OPT_TEST_GFX40
	if ( gte->IsEngine("Engine4"))
	{
		if ( gte->IsGraphicsContextMetal() )
		{
			m_scene = MetalRender::CreateMTLScene40(gte) ;
		}
	}
#endif

#else
#ifdef OPT_TEST_GFX2
	if( gte->IsEngine("Engine2") )
	{
		
		m_scene = new GLB_Scene_ES2;
	}
#endif
#ifdef OPT_TEST_GFX30
	if ( gte->IsEngine("Engine3") )
	{
        m_scene = new GLB_Scene_ES3;
        m_scene->m_disabledRenderBits = gte->GetTestDescriptor()->m_disabledRenderBits;       
		m_scene->SetTestId(gte->GetTestId());
	}
#endif
#ifdef OPT_TEST_GFX31
	if ( gte->IsEngine("Engine31") )
	{
		m_scene = new GLB_Scene_ES31;
		m_scene->m_disabledRenderBits = gte->GetTestDescriptor()->m_disabledRenderBits;
        m_scene->m_adaptation_mode = gte->GetTestDescriptor()->m_adaptation_mode;
		m_scene->SetTestId(gte->GetTestId());
	}
#endif
#ifdef OPT_TEST_GFX40
	if( gte->IsEngine("Engine4") )
	{
		m_scene = new GLB_Scene4(gte);
	}
#endif
	if(m_scene == NULL)
	{
		throw std::runtime_error("Engine version not supported:" + gte->GetTestDescriptor()->m_engine);
	}
#endif
}

Engine2::~Engine2 ()
{
	FreeResources();
}


void Engine2::FreeResources()
{
	m_scene->DeleteVBOPool();
	m_scene->DeleteShaders();
	delete m_scene;
	m_scene = 0;
}


KCL::KCL_Status Engine2::init ()
{
	// Reset globals
	lastTime = 0;
	animationMultiplier = 1.0f;
	animationPause = false;
	animationTime = 0.0f;
	multiplier = 1;

	m_scene->SetProgressPtr(m_loadingProgressPtr);

	m_scene->setTextureCompressionType( m_settings->GetTextureType());
	m_scene->SetZPrePass( m_settings->m_zprepass );

	FBO_COLORMODE color_mode;
	FBO_DEPTHMODE depth_mode;
	KCL::KCL_Status result = KCL::KCL_TESTERROR_NOERROR;

    // TODO
    // Legacy context validation removed
    // Do we should check this here?


	try
	{
		int max_uniform_num = 1024;

		if( !GetSetting().ValidateTextureType())
		{
			return KCL::KCL_TESTERROR_UNSUPPORTED_TC_TYPE;
		}

#if (defined GL_MAX_VERTEX_UNIFORM_VECTORS) && (!defined HAVE_DX)
		glGetIntegerv( GL_MAX_VERTEX_UNIFORM_VECTORS, &max_uniform_num);
#endif

		m_scene->Resize(m_settings->m_viewport_width , m_settings->m_viewport_height);

        m_scene->m_tessellation_enabled = GetSetting().m_tessellation_enabled; //NOTE: set before loading, so that tess shaders are not even compiled if it is off
        // Turn off tesselation
        //m_scene->m_tessellation_enabled = false;

		m_scene->InitSceneVersion(m_settings->m_scenefile.c_str());
		m_scene->InitFactories();

		result = (KCL::KCL_Status)m_scene->Process_Load( max_uniform_num, m_settings->m_scenefile.c_str());

		if(result != KCL::KCL_TESTERROR_NOERROR)
		{
			return result;
		}

		if( GetSetting().m_color_bpp >= 24)
		{
			color_mode = RGBA8888_Linear;
		}
		else
		{
			color_mode = RGBA5551_Linear;
		}

		if( GetSetting().m_depth_bpp >= 24)
		{
			depth_mode = DEPTH_24_RB;
		}
		else
		{
			depth_mode = DEPTH_16_RB;
		}

        m_scene->SetForceHighp(GetSetting().m_force_highp);

		m_scene->CreateVBOPool();
		result = m_scene->Process_GL( color_mode, depth_mode, GetSetting().m_fsaa);

		if(result != KCL::KCL_TESTERROR_NOERROR)
		{
			return result;
		}

	}
	catch(KCL::IOException &ex)
	{
		INFO("%s", ex.what());
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}
	catch(std::bad_alloc &ex)
	{
		INFO("%s", ex.what());
		return KCL::KCL_TESTERROR_OUT_OF_MEMORY;
	}
	catch(std::exception &ex)
	{
		INFO("%s", ex.what());
		return KCL::KCL_TESTERROR_UNKNOWNERROR;
	}

	return result;
}

#define GLFW_MOUSE_BUTTON_1   0
#define GLFW_MOUSE_BUTTON_2   1
#define GLFW_MOUSE_BUTTON_3   2

#define GLFW_KEY_F4             293
#define GLFW_KEY_F5             294
#define GLFW_KEY_F6             295
#define GLFW_KEY_F7             296
#define GLFW_KEY_F8             297
#define GLFW_KEY_F9             298
#define GLFW_KEY_F10            299
#define GLFW_KEY_TAB            258
#define GLFW_KEY_R              82
#define GLFW_KEY_L              76
#define GLFW_KEY_W              87
#define GLFW_KEY_A              65
#define GLFW_KEY_B              66
#define GLFW_KEY_C              67
#define GLFW_KEY_S              83
#define GLFW_KEY_D              68
#define GLFW_KEY_H              72
#define GLFW_KEY_I              73
#define GLFW_KEY_J              74
#define GLFW_KEY_M              77
#define GLFW_KEY_O              79
#define GLFW_KEY_P              80

#define GLFW_KEY_SPACE          32
#define GLFW_KEY_RIGHT          262
#define GLFW_KEY_LEFT           263
#define GLFW_KEY_DOWN           264
#define GLFW_KEY_UP             265
#define GLFW_KEY_PAGE_UP        266
#define GLFW_KEY_PAGE_DOWN      267
#define GLFW_KEY_LEFT_SHIFT     340
#define GLFW_KEY_LEFT_CONTROL   341
#define GLFW_KEY_LEFT_ALT       342

#define GLFW_KEY_2 50

#define GLFW_KEY_3 51
#define GLFW_KEY_4 52
#define GLFW_KEY_5 53

#define GLFW_KEY_6 54
#define GLFW_KEY_7 55
#define GLFW_KEY_8 56
#define GLFW_KEY_9 57

const float max_tessallation = 64.0f ;
void updateTesselationFactor(KRL_Scene *scene,const char* name,float value, bool set = false) {
	const float min_tessallation = 1.0f  ;


	for (unsigned int i = 0 ; i < scene->m_materials.size(); i++) {
		KCL::Material* m = scene->m_materials[i] ;

		if (strstr( m->m_name.c_str(), name)) {
			if (set) {
				m->m_tessellation_factor.x = value ;
			} else {
				m->m_tessellation_factor.x += value ;
			}
			if (m->m_tessellation_factor.x < min_tessallation) {
				m->m_tessellation_factor.x = min_tessallation ;
			}
			else if ( m->m_tessellation_factor.x > max_tessallation) {
				m->m_tessellation_factor.x = max_tessallation ;
			}
		}
	}
}

void Engine2::onCheckboxChanged(int id, bool state)
{
    switch(id)
    {
    case 0:
        m_scene->m_wireframe_render = state;
        break;
    case 1:
        m_scene->m_tessellation_enabled = state;
        break;
    default:
        break;
    }
}

void Engine2::onSliderChanged(int id, float value)
{
    switch(id)
    {
    case 0:
        updateTesselationFactor(m_scene,"gfxb4_car_disp",value * max_tessallation, true) ;
        break;
    case 1:
        updateTesselationFactor(m_scene,"gfxb4_cliff1_disp",value * max_tessallation, true) ;
		updateTesselationFactor(m_scene,"gfxb4_cliff2_disp",value * max_tessallation, true) ;
        break;
    default:
        break;
    }
}

//TODO: insert to KCL::Math
KCL::Vector3D AzElDegToVec(float azimuth, float altitude)
{
    float radAz = Math::Rad(azimuth);
    float radAlt = Math::Rad(altitude);

    float y = sin(radAlt);
    float hyp = cos(radAlt);
    float x = hyp*cos(radAz);
    float z = hyp*sin(radAz);
    return KCL::Vector3D(x,y,z);
}

int Engine2::updateInput(const int time)
{
	if( WasKeyPressed(GLFW_KEY_2))
	{
		KCL::uint32 actor_index = 0;
		if (m_scene->m_actors.size() > actor_index)
		{
			KCL::Matrix4x4 & actor_pom = m_scene->m_actors[actor_index]->m_meshes[0]->m_world_pom;
			KCL::Vector3D target(actor_pom.v[12], actor_pom.v[13], actor_pom.v[14]);		 
			KCL::Vector3D pos = target;
			pos.z += 5.0;
			pos.y += 5.0;

			m_scene->m_camera_position = pos;
			m_scene->m_camera_ref = target;
		}	
	}

	if( WasKeyPressed(GLFW_KEY_5))
	{
        m_scene->SetWireframeRenderEnabled(!m_scene->GetWireframeRenderEnabled());
	}

	if(IsKeyPressed(GLFW_KEY_8))
	{
		updateTesselationFactor(m_scene,"gfxb4_car_disp",-0.1f) ;
	}

	if( IsKeyPressed(GLFW_KEY_9))
	{
		updateTesselationFactor(m_scene,"gfxb4_car_disp",+0.1f) ;
	}


	if(IsKeyPressed(GLFW_KEY_6))
	{
		updateTesselationFactor(m_scene,"gfxb4_cliff1_disp",-0.1f) ;
		updateTesselationFactor(m_scene,"gfxb4_cliff2_disp",-0.1f) ;
	}

	if( IsKeyPressed(GLFW_KEY_7))
	{
		updateTesselationFactor(m_scene,"gfxb4_cliff1_disp",+0.1f) ;
		updateTesselationFactor(m_scene,"gfxb4_cliff2_disp",+0.1f) ;
	}

	if (WasKeyPressed(GLFW_KEY_4)) {
		m_scene->m_tessellation_enabled ^= true ;
	}

	/*if(WasKeyPressed( GLFW_KEY_3))
	{
		updateTesselationFactor(m_scene,"gfxb4_car_disp",1.0f,true) ;
	}

	if( WasKeyPressed( GLFW_KEY_4))
	{
		updateTesselationFactor(m_scene,"gfxb4_cliff1_disp",1.0f,true) ;
		updateTesselationFactor(m_scene,"gfxb4_cliff2_disp",1.0f,true) ;
	}*/

#if 1
	int diff = KCL::Max(time - lastTime, 0);
	
    if(!animationPause)
    {
		if (lastTime > time)
		{
			animationTime = time;
		}
		else
		{
			animationTime += diff * animationMultiplier;
		}		
    }

    if( IsKeyPressed(GLFW_KEY_F4) ) //animtime --
	{
		animationTime -= diff*2*animationMultiplier;
	}
    if( WasKeyPressed(GLFW_KEY_F5)) //speed --
	{
		animationMultiplier-=0.25f;
	}
    if( WasKeyPressed(GLFW_KEY_F6))  //pause
	{
		animationPause = !animationPause;
	}
    if( WasKeyPressed(GLFW_KEY_F7)) //reset
	{
		animationMultiplier=1.0f;
	}
	if( WasKeyPressed(GLFW_KEY_F8)) //speed ++
	{
		animationMultiplier+=0.25f;
	}
    if( IsKeyPressed(GLFW_KEY_F9)) //animtime ++
	{
		animationTime += diff*2*animationMultiplier;
	}
    if( IsKeyPressed(GLFW_KEY_F10)) //animtime ++
	{
		animationTime = 4152;
	}

    if( WasKeyPressed(GLFW_KEY_TAB))
	{
		m_scene->SelectNextCamera();
	}

    if(!diff)
        diff = 10;

    multiplier = IsKeyPressed(GLFW_KEY_LEFT_SHIFT) ? 30.0 : 2.0;
    multiplier *= IsKeyPressed(GLFW_KEY_LEFT_CONTROL) ? 0.1 : 1.0;

	if( IsKeyPressed(GLFW_KEY_W))
	{
		m_scene->Move( 0.01f*diff*multiplier);
	}
	if( IsKeyPressed(GLFW_KEY_S))
	{
		m_scene->Move( -0.01f*diff*multiplier);
	}
	if( IsKeyPressed(GLFW_KEY_A))
	{
		m_scene->Strafe(-0.01f*diff*multiplier);
	}
	if( IsKeyPressed(GLFW_KEY_D))
	{
		m_scene->Strafe( 0.01f*diff*multiplier);
	}
    if( IsKeyPressed(GLFW_KEY_UP))
    {
	    m_scene->Tilt(-0.15f*diff);
    }
    if( IsKeyPressed(GLFW_KEY_DOWN))
    {
	    m_scene->Tilt(0.15f*diff);
    }
	if( IsKeyPressed(GLFW_KEY_LEFT))
	{
        m_scene->Rotate( -0.15f*diff*(multiplier < 1.0f ? multiplier : 1.0f));        
	}
	if( IsKeyPressed(GLFW_KEY_RIGHT))
	{
       m_scene->Rotate( 0.15f*diff*(multiplier < 1.0f ? multiplier : 1.0f));    
	}
	if( IsKeyPressed(GLFW_KEY_PAGE_UP))
	{
		m_scene->Elevate( 0.01f*diff );
	}
	if( IsKeyPressed(GLFW_KEY_PAGE_DOWN))
	{
		m_scene->Elevate( -0.01f*diff );
	}

	if( WasKeyPressed(GLFW_KEY_R))
	{
		m_scene->reloadShaders();
	}
	if( WasKeyPressed(GLFW_KEY_L))
	{
		m_scene->reloadLights();
	}
	if (WasKeyPressed(GLFW_KEY_H))
	{
		m_scene->ReloadHDR() ;
	}
	if (WasKeyPressed(GLFW_KEY_I))
	{
		m_scene->ReloadExportedData();
	}
	if (WasKeyPressed(GLFW_KEY_J))
	{
		m_scene->ReloadExportedData(true);
	}
	if (WasKeyPressed(GLFW_KEY_M))
	{
		m_scene->SaveFreeCamTransform();
//        m_scene->m_azimuth += 10.0;
//        if(m_scene->m_azimuth > 360.0)
//            m_scene->m_azimuth = 0.0;
//
//        m_scene->m_light_dir = AzElDegToVec(m_scene->m_azimuth, m_scene->m_altitude);
	}
	if (WasKeyPressed(GLFW_KEY_O))
	{
		animationTime = m_scene->LoadFreeCamTransform();
//        m_scene->m_azimuth -= 10.0;
//        if(m_scene->m_azimuth < 0.0)
//            m_scene->m_azimuth = 360.0;
//
//        m_scene->m_light_dir = AzElDegToVec(m_scene->m_azimuth, m_scene->m_altitude);
	}
    if(WasKeyPressed(GLFW_KEY_C))
    {
#if defined(OPT_TEST_GFX40) && defined(USE_ANY_GL)
        //instant cubemap capture, at freecam location
        ((GLB_Scene4*)m_scene)->CaptureEnvmap( m_scene->m_camera_position, 100);
#endif
    }
	if (WasKeyPressed(GLFW_KEY_P))
	{
		m_scene->m_mblur_enabled ^= true ;
	}

	if (WasKeyPressed(GLFW_KEY_B))
	{
		m_scene->m_bloom ^= true ;



		printf("Bloom: %s\n",(m_scene->m_bloom?"enabled":"disabled")) ;
	}

    if( WasKeyPressed(GLFW_KEY_SPACE))
    {
        m_scene->m_featureToggle = !m_scene->m_featureToggle;
    }
#endif

#ifdef HAVE_GUI_FOLDER
    int X, Y;
    GetCursor(X, Y);
    m_scene->UpdateGUI(X, Y, IsButtonPressed(GLFW_MOUSE_BUTTON_1), WasButtonPressed(GLFW_MOUSE_BUTTON_1), IsButtonPressed(GLFW_MOUSE_BUTTON_2), WasButtonPressed(GLFW_MOUSE_BUTTON_2), m_keys_current);
#endif

    return animationTime;
}

//#define DEBUG_SHADERS_ON_DEVICE

bool Engine2::animate (const int itime)
{
#ifdef DEBUG_SHADERS_ON_DEVICE
    m_scene->reloadShaders();
#endif

	int time = itime;
	time = updateInput(time);

	SetAnimationTime(time);

	m_scene->m_animation_time = GetAnimationTime();
	m_scene->m_is_warmup = m_is_warmup;

	m_scene->Animate();

	int play_time = m_settings->m_play_time > -1 ? m_settings->m_play_time : m_scene->m_play_time;

    lastTime = itime;
    //return true;  // Animation doesnt loop if uncommented

	if( time >= play_time )
    {
        return false;
    }
    else
    {
        return true;
    }
}


bool Engine2::render ()
{
	m_scene->Render();
   	return true;
}


float Engine2::getScore () const
{
 	if( m_settings->m_frame_step_time > -1)
	{
        return (float)getElapsedTime();
	}
	else
	{
        return (float)m_frames;
	}
}


KCL::uint32 Engine2::indexCount() const
{
	return 0;
}

bool Engine2::isLowLevel() const
{
	return false;
}


bool Engine2::resize(int width, int height)
{
	m_window_height=height;
	m_window_width=width;
	if (m_settings->GetScreenMode()==SMode_Onscreen)
	{
		SetSetting().m_viewport_width=width;
		SetSetting().m_viewport_height=height;
		m_scene->Resize(m_settings->m_viewport_width, m_settings->m_viewport_height);
	}
	return true;
}


KCL::uint32 Engine2::getRenderedVerticesCount() const
{
	return m_scene->getRenderedVerticesCount();
}


KCL::uint32 Engine2::getRenderedTrianglesCount() const
{
	return m_scene->getRenderedTrianglesCount();
}


KCL::uint32 Engine2::getDrawCalls() const
{
	return m_scene->getDrawCalls();
}


KCL::int32 Engine2::getUsedTextureCount() const
{
	return m_scene->getUsedTextureCount();
}


KCL::int32 Engine2::getSamplesPassed() const
{
	return m_scene->getSamplesPassed();
}

float Engine2::getPixelCoverage() const
{
    return m_scene->getPixelCoverage();
}

KCL::int32 Engine2::getInstructionCount() const
{
	return m_scene->getInstructionCount();
}

