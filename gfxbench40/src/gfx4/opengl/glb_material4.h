/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_MATERIAL4_H
#define GLB_MATERIAL4_H

#include "opengl/glb_material.h"
#include "opengl/glb_shader2.h"
#include "krl_scene.h"

#include <string>
#include <set>

class GLB_Scene4;

namespace GLB
{

class Material4 : public KRL::Material 
{
public:
	static const KCL::uint32 INSTANCE_BUFFER_BINDING = 0;

	Material4( const char *name);

	virtual void SetDefaults();	

    KCL::KCL_Status InitShaders(GLB_Scene4 *scene);
	Shader* CreateShader(const char* vsfile, const char* fsfile, const std::set<std::string> *defines, KCL::KCL_Status& error);

	void Bind(KRL_Scene::PassType::Enum pass_type, KRL_Scene::ShaderVariant::Enum shader_variant, KCL::uint32 &texture_num);
	void Unbind();
	
	GLB::GLBShader2 *m_shaders4[KRL_Scene::PassType::PASS_TYPE_COUNT][KRL_Scene::ShaderVariant::SHADER_VARIANT_COUNT];

	KCL::int32 m_sort_order;

    static void SetTextureOverrides(KCL::Texture* ovrds[]);

    // Old API from KRL::Material, should be never called
    virtual KCL::KCL_Status InitShaders( const char* path,  const std::string &max_joint_num_per_mesh);
	virtual void preInit( KCL::uint32 &texture_num, int type, int pass_type);
	virtual void postInit();

private:
	GLB_Scene4 *m_scene;

    void ResolveDefines(std::set<std::string> resolved_defines[KRL_Scene::PassType::PASS_TYPE_COUNT]) const;
	void ClearShaders();
	void CheckUniforms(GLB::GLBShader2 * shader, const char * shader_name);

	void SetSortOrder();

    static KCL::Texture* m_texture_overrides[KCL::Material::MAX_IMAGE_TYPE];
};

}

#endif //GLB_MATERAIL4_H
