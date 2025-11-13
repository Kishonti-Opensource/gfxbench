/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_SCENE_OPENGL31_H
#define GLB_SCENE_OPENGL31_H

#include "../gfx3_0/high_level_tests_3/opengl/glb_scene_opengl.h"
#include "opengl/glb_shader2.h"
#include "opengl/glb_filter.h"
#include "opengl/ubo_manager.h"
#include <memory>

// Forward declarations
class InstancedLightRenderer;
class ComputeLightning;
class ComputeHDR31;
class GUIInterface;

namespace GLB
{
    class FragmentBlur;
}

struct Filter31 : public Filter
{
	Filter31()
	{
		m_ubo_handle = -1;
	}

	virtual void SetUniforms();	

	KCL::int32 m_ubo_handle;
	UBOManager * m_ubo_manager;
};

class GLB_Scene_ES31 : public GLB_Scene_ES3
{
public:
	GLB_Scene_ES31();
	virtual ~GLB_Scene_ES31();
	virtual KCL::KCL_Status Process_GL( GLB::FBO_COLORMODE color_mode, GLB::FBO_DEPTHMODE depth_mode, int samples);
	virtual KCL::KCL_Status CreateBuffers();
	virtual void DeleteBuffers() ;
	virtual KCL::KCL_Status reloadShaders();

protected:			
	virtual void RunEffect(Effect effect);
	virtual void RenderEffect(Effect effect);	
	virtual void DoLightingPass();
	virtual bool UseEnvmapMipmaps();

#ifdef UBO31
	virtual Filter * CreateFilter();
	virtual void Render();
	void UploadMeshes(const KCL::Camera2* camera, std::vector<KCL::Mesh*> & meshes, KCL::Material *_override_material);
    virtual void Animate();
	virtual void Render( KCL::Camera2* camera, std::vector<KCL::Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light, int pass_type);
	virtual void RenderLight( KCL::Light *l);
	virtual void MoveParticles();
	virtual void RenderTFParticles();
	virtual void RenderLightshafts(); 
	virtual void QueryLensFlare();	
	virtual void CollectInstances( std::vector<KCL::Mesh*> &visible_meshes);
#endif

private:
    KCL::int32 m_last_animation_time;
    
    ComputeHDR31 *m_compute_hdr;
	ComputeLightning *m_compute_lightning;
	InstancedLightRenderer *m_InstancedLightRenderer;
    GLB::FragmentBlur *m_dof_blur;
     
	GLB::GLBFilter m_pp2_filter;
	GLB::GLBFilter m_hdr_filter;
	GLB::GLBShader2 *m_pp2;

	UBOManager *m_ubo_manager;
	UBOFrame m_ubo_frame;
	UBOCamera m_ubo_camera;
	UBOMesh m_ubo_mesh;
	UBOStaticMesh m_ubo_static_mesh;
	UBOEnvmapsInterpolator m_ubo_envmaps;
	UBOTranslateUV m_ubo_translate_uv;
	UBOEmitterAdvect m_ubo_emitter_advect;
	UBOEmitterRender m_ubo_emitter_render;
	UBOLightShaft m_ubo_light_shaft;
	UBOLightLensFlare m_ubo_light_lens_flare;
	UBOFilter m_ubo_filter;

	KCL::Matrix4x4 m_shadow_decal_light_matrix;

	//TODO: KCL::uint32 m_fullscreen_quad_vao;
	KCL::uint32 m_quad_vao;
	KCL::uint32 m_quad_vbo;

	std::vector<KCL::Mesh*> lfm;

    // GUI
    friend class GFXGui31;
	std::unique_ptr<GUIInterface> m_gui;
    virtual void UpdateGUI(KCL::uint32 cursorX, KCL::uint32 cursorY, bool mouseLPressed, bool mouseLClicked, bool mouseRPressed, bool mouseRClicked, const bool *downKeys);
};

#endif