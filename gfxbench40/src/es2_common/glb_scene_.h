/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_SCENE_ES2_H
#define GLB_SCENE_ES2_H

#include "krl_scene.h"
#include "kcl_base.h"

#include "render_statistics_defines.h"
#include "glb_mesh.h"
#include "opengl/glb_texture.h"

#ifndef NOT_USE_GFXB
#include "opengl/glb_particlesystem.h"
#include "opengl/glb_light.h"
#endif

#ifdef OPT_TEST_GFX40
#include "opengl/gfxb4_mesh.h"
#endif

#ifdef OCCLUSION_QUERY_BASED_STAT
class GLSamplesPassedQuery;
#endif


class GLB_Scene_ES2_ : public KRL_Scene
{
protected:
	KCL::FactoryBase* m_animatedFactory;
	KCL::FactoryBase* m_tf_emitterFactory;
	KCL::FactoryBase* m_lightFactory;
	KCL::FactoryBase* m_materialFactory;


	GLB_Scene_ES2_();
	~GLB_Scene_ES2_();

	virtual void Render( KCL::Camera2* camera, std::vector<KCL::Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light, int pass_type);

	void CreateVBOPool();
	void DeleteVBOPool();
	void DeleteShaders();
	virtual KCL::KCL_Status reloadShaders();

////////////////////////////////////////////////////////////////
//                        FOR STATISTICS                      //
////////////////////////////////////////////////////////////////
public:
	virtual void InitFactories();
	std::set<KCL::uint32> m_textureCounter;

#ifdef OCCLUSION_QUERY_BASED_STAT
	GLSamplesPassedQuery *m_glGLSamplesPassedQuery;
#endif

protected:
#ifdef MEASURE_PIXEL_PER_PRIMITIVE_COVERAGE
    FILE *m_coverageFile;
#endif
    bool m_measurePixelCoverage;

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

	virtual KCL::TextureFactory &TextureFactory()
	{
		return m_texture_factory;
	}
	
	virtual KCL::Mesh3Factory &Mesh3Factory()
	{
#ifdef OPT_TEST_GFX40
		if (m_scene_version == KCL::SV_40)
		{
			static GFXB4::Mesh3Factory m_mesh3_factory;
			return m_mesh3_factory;
		}
#endif
		return m_mesh3_factory;
	}


private:
	GLB::GLBTextureFactory m_texture_factory;
	GLB::Mesh3Factory m_mesh3_factory;
};

#endif
