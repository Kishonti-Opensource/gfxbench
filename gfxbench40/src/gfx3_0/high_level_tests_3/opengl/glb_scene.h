/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_SCENE_H
#define GLB_SCENE_H

namespace GLB
{
	class Texture2D;
#ifdef DUMMY_FBO_FOR_PLANAR_FLUSH
	class FBO;
#endif
}

class CubeEnvMap;
class ShadowMap;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DRIVER WORKAROUND STARTS HERE: 
// On certain platforms when running T-Rex and rendering planar reflections the command buffer could overflow 
// without this. This code was present in GFXBench 2.7.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DUMMY_FBO_FOR_PLANAR_FLUSH
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DRIVER WORKAROUND ENDS HERE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "glb_scene_.h"
#include "opengl/fbo.h"


class GLB_Scene_ES2 : public GLB_Scene_ES2_
{
public:
	GLB_Scene_ES2();
	~GLB_Scene_ES2();

	void Release_GLResources();

	KCL::KCL_Status Process_GL( GLB::FBO_COLORMODE color_mode, GLB::FBO_DEPTHMODE depth_mode, int samples);

	void Render();
	void Render( KCL::Camera2* camera, std::vector<KCL::Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light);
	void RenderPrepass( KCL::Camera2* camera, std::vector<KCL::Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light);
	virtual void Render( KCL::Camera2* camera, std::vector<KCL::Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light, int pass_type)
	{
		GLB_Scene_ES2_::Render( camera, visible_meshes, _override_material, pm, lod, light, pass_type);
	}

	void OcclusionCullLights();
	void RenderShadow( KRL_ShadowMap* sm);
	void RenderPlanar( KCL::PlanarMap* pm);
	void RenderVisibleAABBs( bool actors, bool rooms, bool room_meshes);
	void RenderVisibleAABBs(std::vector<void*>* objectsToHighlight);
	void RenderPortals();
	void renderSkeleton( KCL::Node* node);

	CubeEnvMap *CreateEnvMap( const KCL::Vector3D &pos, KCL::uint32 idx, bool use_mipmaps);
	void GeneratePVS();


#ifdef DUMMY_FBO_FOR_PLANAR_FLUSH
	GLB::FBO *m_dummyFbo;
	KCL::uint32 m_dummy_program;
	KCL::int32 m_dummy_texture_unif_loc;
	KCL::uint32 m_dummy_vbo;
	KCL::uint32 m_dummy_ebo;
#endif


	KCL::uint32 m_particles_vbo;         //for per particle data
	KCL::uint32 m_fullscreen_quad_vbo;

	GLB::FBO *m_main_fbo;
	GLB::FBO *m_mblur_fbo;

	void SavePNG(GLenum read_buffer, std::string const &desc);

protected:
	virtual bool UseEnvmapMipmaps();
};

#endif
