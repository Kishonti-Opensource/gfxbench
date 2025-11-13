/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_scene5_ovr_test.h"
#include "gfxb_scene5_ovr.h"
#include "gfxb_5_ovr_tools.h"
#include "components/input_component.h"
#include "components/time_component.h"
#include "components/screen_manager_component.h"

using namespace GFXB;

Gfxb5OvrTest::Gfxb5OvrTest() : SceneTestBase()
{
	//vr init
	m_ovr_graphics_context = nullptr;
	m_adapter = nullptr;

	m_ovr_texture_width = 0;
	m_ovr_texture_height = 0;
	m_ovr_fov_x = 0.0f;
	m_ovr_fov_y = 0.0f;

	m_ovr_projection_matrix = ovrMatrix4f_CreateIdentity();
	m_ovr_center_eye_view_matrix = ovrMatrix4f_CreateIdentity();

	for (KCL::uint32 eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++)
	{
		m_ovr_eye_view_matrix[eye] = ovrMatrix4f_CreateIdentity();
		m_ovr_swap_chain[eye] = nullptr;
	}
}

Gfxb5OvrTest::~Gfxb5OvrTest()
{
	delete m_adapter;

	for (KCL::uint32 eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++)
	{
#ifdef WITH_OVR_SDK
		vrapi_DestroyTextureSwapChain(m_ovr_swap_chain[eye]);
#endif
	}
}

void Gfxb5OvrTest::Animate()
{
}

double Gfxb5OvrTest::GetScore()
{
	GFXB::Scene5Ovr *scene = ( GFXB::Scene5Ovr* )GetScene();
	GFXB::OvrSettingsManager& osm = scene->GetOvrSettingsManager();
	return osm.GetScore();
}

void Gfxb5OvrTest::Render()
{
#ifndef WITH_OVR_SDK
#ifdef OVR_BENCHMARK
	{ //do optimization
		GFXB::Scene5Ovr *scene = ( GFXB::Scene5Ovr* )GetScene();
		GFXB::OvrSettingsManager& osm = scene->GetOvrSettingsManager();

		static int64_t timestamp = GetElapsedTime();
		static unsigned framecount = this->GetFrameCount();
		static OvrOptimizationResult last_result = OvrOptimizationResult::WARMUP_STAGE;
		static bool warmup = true;

		bool can_measure = m_screen_manager_component->NeedSwapbuffer();

		const int64_t measure_delay_ms = 10 * 1000;
		const int64_t warmup_time_ms = 15 * 1000;
		const unsigned min_frame_diff = 30;

		int64_t timediff = GetElapsedTime() - timestamp;
		unsigned framediff = this->GetFrameCount() - framecount;

		if( last_result > OvrOptimizationResult::FAILED && last_result < OvrOptimizationResult::OPTIMIZED && can_measure && ( !warmup || timediff > warmup_time_ms) && framediff > min_frame_diff )
		{
			float fps = float( framediff ) / ( float( timediff )*0.001f );
			INFO( "fps: %.2f", fps);
			last_result = osm.Optimize( fps );

			if( warmup )
			{
				INFO( "WARMUP END" );
				warmup = false;
				timestamp = GetElapsedTime();
				framecount = this->GetFrameCount();
			}

			if( last_result == OvrOptimizationResult::OPTIMIZED && osm.GetHasChanged() )
			{
				timestamp = GetElapsedTime();

				INFO( "OPTIMIZING START" );
				if( osm.getIntOption( "ENABLE_SHADOW_MAPPING" ).m_option == 1 && osm.getIntOption( "SHADOW_MAP_SIZE" ).m_option < 1024 )
				{
					INFO( "RESIZING SHADOW MAPS" );
					scene->ResizeShadowMap();
				}
				INFO( "RELOADING SHADERS" );
				scene->ReloadShaders();
				INFO( "OPTIMIZING END" );
			}
		}
		else if( last_result > OvrOptimizationResult::FAILED && last_result == OvrOptimizationResult::OPTIMIZED && can_measure && timediff > measure_delay_ms )
		{
			timestamp = GetElapsedTime();
			framecount = this->GetFrameCount();
			last_result = OvrOptimizationResult::MEASURED;
		}
	}
#endif
#endif

	//require(ctx_->makeCurrent());

	const int frame_index = GetFrameCount();

#ifdef WITH_OVR_SDK
	ovrMobile *mobile = m_ovr_graphics_context->getOvrMobile();

	const double ovr_current_time = vrapi_GetTimeInSeconds();
	const double predicted_display_time = vrapi_GetPredictedDisplayTime(mobile, frame_index);

	// Apply the head model and tracking
	// Set head-on-a-stick model if there is no positional tracking
	m_ovr_base_tracking = vrapi_GetPredictedTracking( mobile, predicted_display_time );
#else
	const double predicted_display_time = 0.0;

	m_ovr_base_tracking = vrapi_GetPredictedTracking( 0, predicted_display_time );
#endif

	m_ovr_head_model_params = vrapi_DefaultHeadModelParms();
	m_ovr_tracking = vrapi_ApplyHeadModel(&m_ovr_head_model_params, &m_ovr_base_tracking);

	// Comment out to disable head tracking
	//m_ovr_tracking = m_ovr_base_tracking;

	// Setup the scene camera
	SceneBase *scene = GetScene();
	KCL::Camera2 *camera = scene->m_fps_camera;
	scene->m_active_camera = camera;
	scene->m_animation_time = GetAnimationTime();

	// Set the perspective part
	camera->Perspective(m_ovr_fov_y, m_ovr_texture_width, m_ovr_texture_height, camera->GetNear(), camera->GetFar());
	static_cast<Scene5Ovr*>(SceneTestBase::GetScene())->SetWH(m_ovr_texture_width, m_ovr_texture_height);
	camera->Update();

	OvrTools::GetOVRMatrix(GetScene()->m_active_camera->GetProjection(), m_ovr_projection_matrix);
	const ovrMatrix4f ovr_tangent_matrix = ovrMatrix4f_TanAngleMatrixFromProjection(&m_ovr_projection_matrix);

	// Setup the position-orrientation part
	KCL::Matrix4x4 camera_pos_matrix;

	ovrMatrix4f ovr_camera_pos_matrix;
	OvrTools::GetOVRMatrix(camera_pos_matrix, ovr_camera_pos_matrix);
	m_ovr_center_eye_view_matrix = vrapi_GetCenterEyeViewMatrix(&m_ovr_head_model_params, &m_ovr_tracking, &ovr_camera_pos_matrix);

	// Init Ovr frame params
#ifdef WITH_OVR_SDK
	m_ovr_frame_params = vrapi_DefaultFrameParms(&m_ovr_graphics_context->getOvrModeParams().Java, VRAPI_FRAME_INIT_DEFAULT, ovr_current_time, NULL);
#endif
	m_ovr_frame_params.FrameIndex = frame_index;

	static const int CPU_LEVEL = 2;
	static const int GPU_LEVEL = 3;
	ovrPerformanceParms perfParms = vrapi_DefaultPerformanceParms();
	perfParms.CpuLevel = CPU_LEVEL;
	perfParms.GpuLevel = GPU_LEVEL;

	m_ovr_frame_params.PerformanceParms = perfParms;

	for (KCL::uint32 eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++)
	{
#ifdef WITH_OVR_SDK
		const KCL::uint32 swap_chain_index = KCL::uint32(frame_index % vrapi_GetTextureSwapChainLength(m_ovr_swap_chain[eye]));
#endif
		if (eye == VRAPI_FRAME_LAYER_EYE_LEFT)
		{
#ifdef WITH_OVR_SDK
			m_adapter->SetCurrentTextureIndex(swap_chain_index);
#endif
		}
		else
		{
#ifdef WITH_OVR_SDK
			m_adapter->SetCurrentTextureIndex(KCL::uint32(m_render_textures[VRAPI_FRAME_LAYER_EYE_LEFT].size()) + swap_chain_index);
#endif
		}

		m_ovr_frame_params.Layers[VRAPI_FRAME_LAYER_TYPE_WORLD].Textures[eye].ColorTextureSwapChain = m_ovr_swap_chain[eye];
#ifdef WITH_OVR_SDK
		m_ovr_frame_params.Layers[VRAPI_FRAME_LAYER_TYPE_WORLD].Textures[eye].TextureSwapChainIndex = swap_chain_index;
#endif
		m_ovr_frame_params.Layers[VRAPI_FRAME_LAYER_TYPE_WORLD].Textures[eye].TexCoordsFromTanAngles = ovr_tangent_matrix;
		m_ovr_frame_params.Layers[VRAPI_FRAME_LAYER_TYPE_WORLD].Textures[eye].HeadPose = m_ovr_tracking.HeadPose;

		// Set the camera for the current eye
		m_ovr_eye_view_matrix[eye] = vrapi_GetEyeViewMatrix(&m_ovr_head_model_params, &m_ovr_center_eye_view_matrix, eye);

		ovrMatrix4f mx = vrapi_GetEyeViewMatrix(&m_ovr_head_model_params, &m_ovr_center_eye_view_matrix, eye);
		static_cast<Scene5Ovr*>(SceneTestBase::GetScene())->SetUp(KCL::Vector3D(mx.M[1][0], mx.M[1][1], mx.M[1][2]).normalize());

		KCL::Matrix4x4 view_matrix;
		OvrTools::GetKCLMatrix(m_ovr_eye_view_matrix[eye], view_matrix);

		camera->LookAt(view_matrix);
		camera->Update();

		static_cast<Scene5Ovr*>(SceneTestBase::GetScene())->SetEye(eye);

		SceneTestBase::Animate();
		SceneTestBase::Render();
	}

#ifdef WITH_OVR_SDK
	vrapi_SubmitFrame(m_ovr_graphics_context->getOvrMobile(), &m_ovr_frame_params);
#endif
}

KCL::KCL_Status Gfxb5OvrTest::Init()
{
	INFO( "Gfxb5OvrTest::Init" );
	m_test_width = m_ovr_texture_width;
	m_test_height = m_ovr_texture_height;

	KCL::KCL_Status status = SceneTestBase::Init();

	return status;
}

GFXB::SceneBase *Gfxb5OvrTest::CreateScene()
{
	return new GFXB::Scene5Ovr();
}

std::string Gfxb5OvrTest::ChooseTextureType()
{
	//force using the 888 image dir
	//return "888";

	const std::string &texture_type = GetTestDescriptor().GetTextureType();

	if (texture_type != "Auto")
	{
		return texture_type;
	}

	//bool astc_supported = nglGetInteger(NGL_TEXTURE_COMPRESSION_ASTC) > 0;
	bool etc2_supported = nglGetInteger(NGL_TEXTURE_COMPRESSION_ETC2) > 0;
	bool dxt5_supported = nglGetInteger(NGL_TEXTURE_COMPRESSION_DXT5) > 0;

	switch (nglGetApi())
	{
	case NGL_OPENGL:
	case NGL_DIRECT3D_11:
	case NGL_DIRECT3D_12:
	case NGL_METAL_MACOS:
		if (dxt5_supported)
		{
			return "DXT5";
		}
		return "888";

	case NGL_OPENGL_ES:
	case NGL_METAL_IOS:
	{
		if (etc2_supported)
		{
			return "ETC2";
		}
		return "888";
	}
	case NGL_VULKAN:
	{
		if (dxt5_supported)
		{
			return "DXT5";
		}
		return "888";
	}

	default:
		return "888";
	}
}


void Gfxb5OvrTest::HandleUserInput(GLB::InputComponent *input_component, float frame_time_secs)
{
	SceneTestBase::HandleUserInput(input_component, frame_time_secs);

	GFXB::Scene5Ovr *scene = ( GFXB::Scene5Ovr* )GetScene();
	//if( input_component->IsKeyPressed( 84 ) ) //ascii codes

	GFXB::OvrSettingsManager& osm = scene->GetOvrSettingsManager();
	
	//ascii code
	int f0 = 289;

	if( input_component->IsKeyPressed( f0 + 1 ) ) //f1
	{
		osm.SetIntOption( "SSAO_ENABLED", !osm.getIntOption("SSAO_ENABLED").m_option );
	}
	if( input_component->IsKeyPressed( f0 + 2 ) ) //f2
	{
		osm.SetIntOption( "TONEMAPPER_FILMIC", !osm.getIntOption( "TONEMAPPER_FILMIC" ).m_option );
	}
	if( input_component->IsKeyPressed( f0 + 3 ) ) //f3
	{
		osm.SetIntOption( "EXPOSURE_MANUAL", !osm.getIntOption( "EXPOSURE_MANUAL" ).m_option );
	}
	if( input_component->IsKeyPressed( f0 + 4 ) ) //f4
	{
		osm.SetIntOption( "ENABLE_DIRECT_LIGHTING", !osm.getIntOption( "ENABLE_DIRECT_LIGHTING" ).m_option );
	}
	if( input_component->IsKeyPressed( f0 + 5 ) ) //f5
	{
		osm.SetIntOption( "ENABLE_SHADOW_MAPPING", !osm.getIntOption( "ENABLE_SHADOW_MAPPING" ).m_option );
	}
	if( input_component->IsKeyPressed( f0 + 6 ) ) //f6
	{
		int size = osm.getIntOption( "SHADOW_MAP_SIZE" ).m_option * 2;
		
		if( size > 2048 ) size = 512;

		osm.SetIntOption( "SHADOW_MAP_SIZE", size );
	}
	if( input_component->IsKeyPressed( f0 + 7 ) ) //f7
	{
		osm.SetIntOption( "ENABLE_CONSTANT_AMBIENT", !osm.getIntOption( "ENABLE_CONSTANT_AMBIENT" ).m_option );
	}
	if( input_component->IsKeyPressed( f0 + 8 ) ) //f8
	{
		osm.SetIntOption( "ENABLE_HEMISPHERIC_AMBIENT", !osm.getIntOption( "ENABLE_HEMISPHERIC_AMBIENT" ).m_option );
	}
	if( input_component->IsKeyPressed( f0 + 9 ) ) //f9
	{
		osm.SetIntOption( "ENABLE_IBL_AMBIENT", !osm.getIntOption( "ENABLE_IBL_AMBIENT" ).m_option );
	}
	if( input_component->IsKeyPressed( f0 + 10 ) ) //f10
	{
		osm.SetIntOption( "ENABLE_BLOOM", !osm.getIntOption( "ENABLE_BLOOM" ).m_option );
	}
}

bool Gfxb5OvrTest::InitRenderAPI()
{
	INFO( "Gfxb5OvrTest::InitRenderAPI" );

	INFO("Gfxb5Ovr::InitRenderAPI");
#ifdef ANDROID
#ifdef WITH_OVR_SDK
	if (ctx_->type() !=
		GraphicsContext::OVR_EGL
		)
	{
		INFO("Error! Gfxb5Ovr::InitRenderAPI - Graphics context is not OVR_EGL!");
		return false;
	}
#endif
#endif

	m_ovr_graphics_context = (OVRGraphicsContext*)ctx_;

	InitOvr();

	std::vector<void*> ngl_user_data;
	//ngl_user_data.push_back((void*)(size_t)m_ovr_texture_width);
	//ngl_user_data.push_back((void*)(size_t)m_ovr_texture_height);
#ifdef WITH_OVR_SDK
	ngl_user_data.push_back((void*)m_adapter );
#endif

	uint32_t major_version = 1;
	uint32_t minor_version = 0;

	NGL_context_descriptor cd;
	cd.m_enable_vsync = true;
	cd.m_user_data = ngl_user_data;
	cd.m_display_width = m_ovr_texture_width;
	cd.m_display_height = m_ovr_texture_height;
	cd.m_enable_validation = false;
	cd.m_selected_device_id = m_selected_device_id;
	cd.__logf = GPUApiLogFunction;

	switch (ctx_->type())
	{
	case GraphicsContext::OVR_EGL:
		INFO( "context: OVR_EGL" );
		major_version = ctx_->versionMajor();
		minor_version = ctx_->versionMinor();
		cd.m_api = NGL_OPENGL_ES;
		cd.m_major_version = major_version;
		cd.m_minor_version = minor_version;
		return nglCreateContext(cd); //Force gles even on desktop, so mobile is better debuggable...
	case GraphicsContext::OPENGL:
		INFO( "context: OPENGL" );
		major_version = ctx_->versionMajor();
		minor_version = ctx_->versionMinor();
		cd.m_api = NGL_OPENGL;
		cd.m_major_version = major_version;
		cd.m_minor_version = minor_version;
		return nglCreateContext(cd);
	default:
		throw new std::string("CONTEXT ERROR");
	}
}

void Gfxb5OvrTest::InitOvr()
{
	INFO( "Gfxb5OvrTest::InitOvr" );

#ifdef WITH_OVR_SDK
	//actually prefer rendering on native resolution as opposed to 1024x1024 that occulus defaults to...
	//TODO: test if this works VRAPI_SYS_PROP_DISPLAY_PIXELS_WIDE
	//m_ovr_texture_width = 1024;//GetNativeWidth() / 2;//1280;// (KCL::uint32)vrapi_GetSystemPropertyInt(&m_ovr_graphics_context->getOvrModeParams().Java, VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_WIDTH);
	m_ovr_texture_width = ( KCL::uint32 )vrapi_GetSystemPropertyInt( &m_ovr_graphics_context->getOvrModeParams().Java, VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_WIDTH );
	//m_ovr_texture_height = 1024;//GetNativeHeight();//1440;// (KCL::uint32)vrapi_GetSystemPropertyInt(&m_ovr_graphics_context->getOvrModeParams().Java, VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_HEIGHT);
	m_ovr_texture_height = ( KCL::uint32 )vrapi_GetSystemPropertyInt( &m_ovr_graphics_context->getOvrModeParams().Java, VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_HEIGHT );

	m_ovr_fov_x = vrapi_GetSystemPropertyFloat(&m_ovr_graphics_context->getOvrModeParams().Java, VRAPI_SYS_PROP_SUGGESTED_EYE_FOV_DEGREES_X);
	m_ovr_fov_y = vrapi_GetSystemPropertyFloat(&m_ovr_graphics_context->getOvrModeParams().Java, VRAPI_SYS_PROP_SUGGESTED_EYE_FOV_DEGREES_Y);
#else
	//float scaling_factor = 0.75f;
	float scaling_factor = 1.0f;
#ifndef ANDROID
	m_ovr_texture_width = KCL::uint32(float(GetNativeWidth() / 2) * scaling_factor ); //debug if no ovr
	m_ovr_texture_height = KCL::uint32( float( GetNativeHeight() ) * scaling_factor );
#else
	m_ovr_texture_width = KCL::uint32( float( GetNativeWidth() / 2 ) * scaling_factor );// 1280; //debug if no ovr
	m_ovr_texture_height = KCL::uint32( float( GetNativeHeight() ) * scaling_factor );// 1440;
#endif
	m_ovr_fov_x = 90.0f;
	m_ovr_fov_y = 90.0f;
#endif

	INFO("-OVR- Texture dims: %d %d", m_ovr_texture_width, m_ovr_texture_height);
	INFO("-OVR- Fov: %.2f %.2f", m_ovr_fov_x, m_ovr_fov_y);

	// Create the swap chains
	std::vector<KCL::uint32> textures;
	for (KCL::uint32 eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++)
	{
#ifdef WITH_OVR_SDK
		m_ovr_swap_chain[eye] = vrapi_CreateTextureSwapChain(VRAPI_TEXTURE_TYPE_2D, VRAPI_TEXTURE_FORMAT_8888, m_ovr_texture_width, m_ovr_texture_height, 1, true);

		const KCL::uint32 length = (KCL::uint32)vrapi_GetTextureSwapChainLength(m_ovr_swap_chain[eye]);

		for (KCL::uint32 i = 0; i < length; i++)
		{
			KCL::uint32 texture_id = vrapi_GetTextureSwapChainHandle(m_ovr_swap_chain[eye], i);
			m_render_textures[eye].push_back(texture_id);
			textures.push_back(texture_id);
		}
#else

#endif
	}
	INFO("-OVR- Swap chains: %d %d", m_render_textures[VRAPI_FRAME_LAYER_EYE_LEFT].size(), m_render_textures[VRAPI_FRAME_LAYER_EYE_RIGHT].size());

	// Initialize the adapter
	m_adapter = new OvrNGLAdapter(textures);
	m_adapter->SetCurrentTextureIndex(0);
}


CREATE_FACTORY(gl_5_ovr_2, Gfxb5OvrTest)
