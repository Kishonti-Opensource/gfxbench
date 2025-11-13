/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_SCENE_OPENGL3_H
#define GLB_SCENE_OPENGL3_H

#include "glb_scene.h"
#include "opengl/shader.h"
#include "gbuffer.h"
#include "vao_storage.h"

struct LightBufferObject
{
	KCL::uint32 m_vbo;
	KCL::uint32 m_ebo;
	KCL::uint32 m_num_indices;
	KCL::uint32 m_vao;

	KCL::int32 m_ubo_handle;
	
	LightBufferObject()
	{
		m_vbo = 0 ;
		m_ebo = 0 ;
		m_num_indices = 0 ;
		m_vao = 0 ;
		m_ubo_handle = -1;
	}
};

struct Filter
{
	Shader *m_shader;
	KCL::uint32 m_fbo;
	KCL::uint32 m_color_texture;
	KCL::uint32 m_width;
	KCL::uint32 m_height;
	int m_dir;
    KCL::uint32 m_max_mipmaps;
	bool m_is_mipmapped;
	KCL::Camera2 *m_active_camera;
	float m_focus_distance;
    bool m_onscreen;

	KCL::uint32 m_input_textures[8];

	Filter();
	virtual ~Filter() {};
	void Clear();
    void Create( KCL::uint32 depth_attachment, KCL::uint32 w, KCL::uint32 h, bool onscreen, KCL::uint32 maxmipcount, int dir, bool fp = false);	

#if defined OCCLUSION_QUERY_BASED_STAT
	void Render( KCL::uint32 &num_draw_calls, KCL::uint32 &num_triangles , KCL::uint32 &num_vertices , std::set<KCL::uint32> &textureCounter , KCL::int32 &num_samples_passed , KCL::int32 &num_instruction, GLSamplesPassedQuery *glGLSamplesPassedQuery);
#else
	void Render( KCL::uint32 &num_draw_calls, KCL::uint32 &num_triangles , KCL::uint32 &num_vertices , std::set<KCL::uint32> &textureCounter , KCL::int32 &num_samples_passed , KCL::int32 &num_instruction);
#endif	
	virtual void SetUniforms();
};

class GLB_Scene_ES3 : public GLB_Scene_ES2
{
public:

    struct RenderBits
    {
        enum 
        {
            ERB_ShadowDepthRender   = 1,
            ERB_GBufferSolids       = 2,
            ERB_LensFlareQuery      = 4,
            ERB_Lighting            = 8,
            ERB_Sky                 = 16,
            ERB_ShadowDecal         = 32,
            ERB_Decals              = 64,
            ERB_Particles           = 128,
            ERB_LightShafts         = 256,
            ERB_Transparents        = 512,
            ERB_LensFlares          = 1024,
            ERB_Post                = 2048,
			ERB_Compute_Lightning   = 4096, //manhattan31 only
			ERB_Compute_DOF			= 8192  //manhattan31 only
        };
    };

	enum Effect
	{
		BlurEffect,
		LightningEffect,
		PostEffect
	};

	struct shaftVert
	{
		KCL::Vector3D pos;
		//KCL::Vector3D col;
		float scrollStrength;
	};

	GLB_Scene_ES3();
	virtual ~GLB_Scene_ES3();
	KCL::KCL_Status Process_GL( GLB::FBO_COLORMODE color_mode, GLB::FBO_DEPTHMODE depth_mode, int samples);
	virtual KCL::KCL_Status CreateBuffers();
	virtual void DeleteBuffers() ;
	virtual void Render();

	virtual KCL::KCL_Status reloadShaders();

    void Debug_RenderLights();
    void Debug_RenderStatics();

	KCL::uint32 m_UBO_ids[Shader::UNIFORM_BLOCK_COUNT];

	KCL::uint32 m_VolumeLightTex;
	KCL::uint8 m_VolumeLightData[64][64][3]; //rgb color
	KCL::Vector3D m_VolumeLightMin;
	KCL::Vector3D m_VolumeLightExtents;

protected:
	static const KCL::uint32 FILTER_COUNT = 32;
	PP pp;
	Filter *filters[FILTER_COUNT];

	int m_samples;

	KCL::Texture* m_particle_noise;	
	KCL::Texture* m_fire_texid;
	KCL::Texture* m_smoke_texid;
	KCL::Texture* m_fog_texid;

	KCL::Texture* m_light_noise;

	LightBufferObject m_lbos[2];
	KCL::uint32 m_lightshaft_vbo;
	KCL::uint32 m_lightshaft_ebo;
	KCL::uint32 m_lightshaft_vao;

	KCL::uint32 m_occlusion_query_vbo;
	KCL::uint32 m_occlusion_query_vao;

	KCL::uint32 m_fullscreen_quad_vao;

	VaoStorage *m_vao_storage ;

	bool m_skip_create_shadow_decal;

	virtual void Render( KCL::Camera2* camera, std::vector<KCL::Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light, int pass_type);

	virtual void DoLightingPass();
	virtual void RunEffect(Effect effect);
	virtual void RenderEffect(Effect effect);
	
	virtual void RenderLight( KCL::Light *l);
	virtual void MoveParticles();
	virtual void RenderTFParticles();
	virtual void RenderLightshafts(); 
	virtual void QueryLensFlare();

	virtual bool UseEnvmapMipmaps();
		
	virtual Filter * CreateFilter();
    void CollectInstances( std::vector<KCL::Mesh*> &visible_meshes);
private:
	void CreateLBOs();
};


class GLB_Scene_ES2_FrameCapture : public GLB_Scene_ES2
{
public:
	virtual void Render( KCL::Camera2* camera, std::vector<KCL::Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light, int pass_type) {}
};

#endif