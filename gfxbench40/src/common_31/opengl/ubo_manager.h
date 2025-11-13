/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef UBO_MANAGER_H
#define UBO_MANAGER_H

#include <kcl_base.h>
#include <kcl_mesh.h>

#include <krl_scene.h>

#include "opengl/glb_particlesystem.h"
#include "opengl/glb_material.h"
#include "opengl/glb_light.h"

#include <vector>

struct Filter31;
struct LightBufferObject;

//4-float alignment!/
//MUST keep in sync with all shaders!
//--> shader.h/UNIFORM_BLOCK_COUNT
struct VectorI4D
{
	union
	{
		int v[4];
		struct
		{
			int x, y, z, w;
		};
	};

	VectorI4D () : x(0), y(0), z(0), w(0) {}
	explicit VectorI4D (const VectorI4D &v) : x(v.x), y(v.x), z(v.z), w(v.w) {}
	VectorI4D (int nx, int ny, int nz, int nw) : x(nx), y(ny), z(nz), w(nw) {}
};

template<typename T>
struct UBO_Container
{
    T m_ubo;
    KCL::int32 m_handle;   

    UBO_Container()
    {
        m_handle = -1;
    }
};

//TODO: move these structs to text files the shaders and code can both include
struct UBOFrame
{
	KCL::Matrix4x4 shadow_matrix0;
    KCL::Vector4D time_dt_pad2; // time, delta time in seconds
    
     //Post process constants
    KCL::Vector4D ABCD; //ShoulderStrength, LinearStrength, LinearAngle, ToeStrength
    KCL::Vector4D EFW_tau; //ToeNumerator, ToeDenominator, LinearWhite, AdaptationRange
    KCL::Vector4D exposure_bloomthreshold_tone_map_white_pad;
};

struct UBOCamera
{
	KCL::Vector4D depth_parameters;
	KCL::Vector4D view_dirXYZ_pad;
	KCL::Vector4D view_posXYZ_normalized_time;
	KCL::Vector4D inv_resolutionXY_pad;
	KCL::Matrix4x4 vp;
};

struct UBOMaterial
{
	KCL::Vector4D fresnelXYZ_transp;
	KCL::Vector4D matparams_disiseri;
};

struct UBOMesh
{
	KCL::Matrix4x4 mvp;
	KCL::Matrix4x4 mv;
	KCL::Matrix4x4 inv_modelview;
};

struct UBOStaticMesh
{
	KCL::Matrix4x4 model;
	KCL::Matrix4x4 inv_model;
	KCL::Vector4D color_pad;
};

struct UBOEmitterAdvect
{
	KCL::Vector4D emitter_apertureXYZ_focusdist;
	KCL::Matrix4x4 emitter_worldmat;

	KCL::Vector4D emitter_min_freqXYZ_speed;
	KCL::Vector4D emitter_max_freqXYZ_speed;
	KCL::Vector4D emitter_min_ampXYZ_accel;
	KCL::Vector4D emitter_max_ampXYZ_accel;

	KCL::Vector4D emitter_externalVel_gravityFactor;

	KCL::Vector4D emitter_maxlifeX_sizeYZ_numSubsteps;
};

struct UBOEmitterRender
{
	KCL::Matrix4x4 mvp;
	KCL::Matrix4x4 mv;
	KCL::Vector4D emitter_maxlifeX_sizeYZ_pad;
	KCL::Vector4D color_pad;
};

struct UBOLightShaft
{
	KCL::Matrix4x4 mvp;
	KCL::Matrix4x4 mv;
	KCL::Matrix4x4 shadow_matrix0;
	
	KCL::Vector4D light_color_pad;
	KCL::Vector4D light_pos_pad;
	KCL::Vector4D light_x;
	KCL::Vector4D spot_cos_attenuation_parameter_pad;
};

struct UBOLightLensFlare
{
	KCL::Vector4D light_color_pad;
	KCL::Vector4D light_pos_pad;
};

struct UBOFilter
{
	KCL::Vector4D offset_2d_pad2;
};

struct UBOTranslateUV
{
	KCL::Vector4D translate_uv_pad2;
};

struct UBOEnvmapsInterpolator
{
	KCL::Vector4D envmaps_interpolator_pad3;
};

class UBOManager
{
public:	
	struct Camera
	{
		enum CameraType
		{
			NORMAL = 0,
			SHADOW = 1,
			CAMERA_COUNT = 2,
		};
	};

	UBOManager();
	~UBOManager();

	void Precache(KRL_Scene * scene_handler);
	
	void BeginFrame();
	void EndFrame();

	void SetFrame(const UBOFrame * data);
	void SetCamera(KCL::uint32 camera_index, const UBOCamera * data);
	void SetMesh(KCL::Mesh * mesh, const UBOMesh * data, const UBOStaticMesh * static_data);
	void SetTranslateUV(KCL::Mesh * mesh,const UBOTranslateUV * data);
	void SetEnvmapsInterpolator(KCL::Mesh * mesh, const UBOEnvmapsInterpolator * data);
	void SetLBO(LightBufferObject * light, const UBOMesh * data, const UBOStaticMesh * static_data);
	void SetEmitter(GLB::TF_emitter * emitter, const UBOEmitterAdvect * data, const UBOEmitterRender * data2);
	void SetLightShaft(GLB::Light * light, const UBOLightShaft * data);
	void SetLightLensFlare(GLB::Light * light, const UBOLightLensFlare * data);
	void SetFilter(Filter31 * filter, const UBOFilter * data);

    template<typename T> void SetUBO(KCL::int32& ubo_handle, const T * data);

	void Upload();

	void BindCamera(KCL::uint32 camera_index);
	void BindMesh(KCL::Mesh * mesh, GLB::Material * material);
	void BindLBO(LightBufferObject * light);	
	void BindEmitterAdvect(GLB::TF_emitter * emitter);
	void BindEmitterRender(GLB::TF_emitter * emitter);
	void BindLightShaft(GLB::Light * light);
	void BindLightLensFlare(GLB::Light * light);
	void BindFilter(Filter31 * filter);
	void BindTranslateUV(KCL::Mesh * mesh);
	void BindEnvmapsInterpolator(KCL::Mesh * mesh);
    
    void BindUBO(KCL::int32 ubo_handle, KCL::int32 slot);

private:
	static bool log_enabled;
	KCL::int32 m_ubo_usage;
	KCL::int32 m_offset_alignment;
	KCL::int32 m_ubo_max_size;

	KCL::uint32 m_prev_camera_bind;
	KCL::Mesh * m_prev_mesh_bind;
	GLB::Material * m_prev_material_bind;
	
	std::vector<KCL::uint32> m_constant_ubos;

	KCL::uint32 m_current_camera;
	struct BufferBind
	{
		KCL::uint32 m_id;
		KCL::uint32 m_offset;
        KCL::uint32 m_structSize;
        KCL::uint32 m_alignedSize;
        
        //TODO: remove these below
		KCL::uint32 m_id2;		// UBOEmitterRender, UBOLightLensFlare, UBOTranslateUV
		KCL::uint32 m_offset2;

		KCL::uint32 m_id3;		// UBOEnvmapsInterpolator
		KCL::uint32 m_offset3;

		KCL::uint32 m_static_id;	// UBOStaticMesh
		KCL::uint32 m_static_offset;

		bool precached;
		
		BufferBind()
		{
            m_structSize = 0;
            m_alignedSize = 0;
			precached = false;
		}

	};
	std::vector<BufferBind> m_ubo_handles[Camera::CAMERA_COUNT];
	KCL::uint32 m_ubo_handle_counter;

	struct Buffer
	{
		KCL::uint32 m_id;
		char * mem_ptr;
	};	
	std::vector<Buffer> m_ubos;
	
	KCL::uint32 m_ubo_index;
	KCL::uint32 m_offset;
	
	KCL::uint32 m_frame_asize;
	KCL::uint32 m_camera_asize;
	KCL::uint32 m_mat_asize;
	KCL::uint32 m_mesh_asize;
	KCL::uint32 m_static_mesh_asize;
	KCL::uint32 m_emitter_advect_asize;
	KCL::uint32 m_emitter_render_asize;
	KCL::uint32 m_light_shaft_asize;
	KCL::uint32 m_light_lens_flare_asize;
	KCL::uint32 m_filter_asize;
	KCL::uint32 m_translate_uv_asize;
	KCL::uint32 m_envmaps_interpolator_asize;
	
	KCL::uint32 CreateNewBuffer();
	KCL::uint32 StoreData(const void * data, int size, int asize);
	KCL::int32 CreateHandle();
	static KCL::uint32 AlignedSize(KCL::uint32 size, KCL::uint32 alignment);
};

template<typename T>
void UBOManager::SetUBO(KCL::int32& ubo_handle, const T * data)
{
    BufferBind* bind = NULL;
	if (ubo_handle == -1)
	{
		ubo_handle = CreateHandle();
        bind = &m_ubo_handles[m_current_camera][ubo_handle];
        bind->m_structSize = sizeof(T);
        bind->m_alignedSize = AlignedSize(sizeof(T), m_offset_alignment);
	}
    else
	{
        bind = &m_ubo_handles[m_current_camera][ubo_handle];	
    }

    bind->m_offset = StoreData((void*)data, bind->m_structSize, bind->m_alignedSize);
	bind->m_id =  m_ubos[m_ubo_index].m_id;
}

#endif