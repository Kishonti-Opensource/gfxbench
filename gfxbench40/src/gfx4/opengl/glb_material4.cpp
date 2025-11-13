/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glb_material4.h"

#include <set>
#include "opengl/glb_scene_opengl4.h"
#include "opengl/glb_shader2.h"
#include "opengl/glb_opengl_state_manager.h"
#include "glb_cascaded_shadow_map.h"
#include "ng/log.h"

using namespace GLB;

KCL::Texture* Material4::m_texture_overrides[KCL::Material::MAX_IMAGE_TYPE];

Material4::Material4( const char *name) : 
	KRL::Material(name)
{	
	SetDefaults();
}

void Material4::SetDefaults()
{
	KRL::Material::SetDefaults();	
	ClearShaders();
	m_sort_order = 0;
}

void Material4::ClearShaders()
{
	for (KCL::uint32 i = 0; i < KRL_Scene::PassType::PASS_TYPE_COUNT; i++)
	{
		for (KCL::uint32 j = 0; j < KRL_Scene::ShaderVariant::SHADER_VARIANT_COUNT; j++)	
		{
			m_shaders4[i][j] = NULL;
		}
	}
}

KCL::KCL_Status Material4::InitShaders(GLB_Scene4 *scene)
{
    m_scene = scene;

    ClearShaders();

	// Set the default shaders for the techniques
	std::string default_shaders[KRL_Scene::PassType::PASS_TYPE_COUNT];
	default_shaders[KRL_Scene::PassType::NORMAL] = "default.shader";
	default_shaders[KRL_Scene::PassType::REFLECTION] = "simple_lightmapped.shader"; // TODO: use simple.shader, and setup exceptions in .cfg files
    default_shaders[KRL_Scene::PassType::SHADOW] = "shadow_caster.shader";

    // Collect the defines
    std::set<std::string> resolved_defines[KRL_Scene::PassType::PASS_TYPE_COUNT];
    resolved_defines[KRL_Scene::PassType::NORMAL] = m_shader_defs[KRL_Scene::PassType::NORMAL];
    resolved_defines[KRL_Scene::PassType::REFLECTION] = m_shader_defs[KRL_Scene::PassType::REFLECTION];
    resolved_defines[KRL_Scene::PassType::SHADOW] = m_shader_defs[KRL_Scene::PassType::SHADOW];
    ResolveDefines(resolved_defines);

	std::string shader_name;
	KCL::KCL_Status result = KCL::KCL_TESTERROR_NOERROR;
	GLB::GLBShaderBuilder sb;
	for (KCL::uint32 pass_type = 0; pass_type < KRL_Scene::PassType::PASS_TYPE_COUNT; pass_type++)
	{
		// Set the name of the shader		
		if(m_shader_names[pass_type].empty())
		{
			shader_name = default_shaders[pass_type];			
		}
		else
		{
			shader_name = m_shader_names[pass_type];
		}

		// Hard code the shadow caster shader NOTE: We could remove this if we only use cascaded shadows
		if (m_name.find("shadow_caster_depth_depth") != std::string::npos)
		{
			m_material_type = SHADOWCASTER1;
			shader_name = "shadow_caster0.shader";
            return result;
		}

        std::set<std::string> defines = resolved_defines[pass_type];

		for (KCL::uint32 shader_variant = 0; shader_variant < KRL_Scene::ShaderVariant::SHADER_VARIANT_COUNT; shader_variant++)
		{
            // GFXBench 4.0 does not use skeletal now, so it is pointless to compile the shaders
            if (shader_variant == KRL_Scene::ShaderVariant::SKELETAL)
            {
                continue;
            }

            //so each variant gets the predefined shader defs for the appropriate pass type
            //m_defines_set gets cleared after each compilation!
            sb.AddDefines(&resolved_defines[pass_type]);		

          	if (shader_variant == KRL_Scene::ShaderVariant::INSTANCED)
			{
				sb.AddDefine("INSTANCING");
			}
			if (shader_variant == KRL_Scene::ShaderVariant::SKELETAL)
			{
				sb.AddDefine("SKELETAL");
			}
			
#ifdef _DEBUG
            if(scene->GetWireframeRenderEnabled() && scene->GetWireframeMode() == GLB_Scene4::WireframeGS)
            {
                sb.AddDefine("USE_GEOMSHADER");
            }
#endif
            // Compile the shader
			m_shaders4[pass_type][shader_variant] = sb.ShaderFile(shader_name.c_str()).Build(result);
			if (result != KCL::KCL_TESTERROR_NOERROR)
			{
				return result;
			}
			CheckUniforms(m_shaders4[pass_type][shader_variant], shader_name.c_str());
		}
	}

	SetSortOrder();

	return KCL::KCL_TESTERROR_NOERROR;
}

void GLB::Material4::ResolveDefines(std::set<std::string> resolved_defines[KRL_Scene::PassType::PASS_TYPE_COUNT]) const
{
    if (g_extension->isES() == false)
    {
        resolved_defines[KRL_Scene::PassType::NORMAL].insert("UBYTE_NORMAL_TANGENT");
        resolved_defines[KRL_Scene::PassType::REFLECTION].insert("UBYTE_NORMAL_TANGENT");
        resolved_defines[KRL_Scene::PassType::SHADOW].insert("UBYTE_NORMAL_TANGENT");
    }
    
    resolved_defines[KRL_Scene::PassType::NORMAL].insert("VELOCITY_BUFFER");

    if(!m_is_transparent)
    {
        resolved_defines[KRL_Scene::PassType::NORMAL].insert("G_BUFFER_PASS");
    }
    
    if (m_has_emissive_channel)
    {
        resolved_defines[KRL_Scene::PassType::NORMAL].insert("HAS_LOCAL_EMISSIVE");
        resolved_defines[KRL_Scene::PassType::REFLECTION].insert("HAS_LOCAL_EMISSIVE");
    }

    if (m_opacity_mode == ALPHA_TEST)
    {
         resolved_defines[KRL_Scene::PassType::NORMAL].insert("HAS_LOCAL_OPACITY");
         resolved_defines[KRL_Scene::PassType::NORMAL].insert("ALPHA_TEST");

		 resolved_defines[KRL_Scene::PassType::SHADOW].insert("HAS_LOCAL_OPACITY");
         resolved_defines[KRL_Scene::PassType::SHADOW].insert("ALPHA_TEST");
    }

    if (m_opacity_mode == ALPHA_BLEND)
    {
         resolved_defines[KRL_Scene::PassType::NORMAL].insert("HAS_LOCAL_OPACITY");
    }

    if (m_is_tesselated)
    {
        if(GLB::g_extension->hasExtension( GLB::GLBEXT_primitive_bounding_box))
        {
            resolved_defines[KRL_Scene::PassType::NORMAL].insert("USE_PBB_EXT");
            resolved_defines[KRL_Scene::PassType::SHADOW].insert("USE_PBB_EXT");
	    }

        if (m_displacement_mode == DISPLACEMENT_LOCAL)
        {
            resolved_defines[KRL_Scene::PassType::NORMAL].insert("HAS_LOCAL_DISPLACEMENT");
            resolved_defines[KRL_Scene::PassType::NORMAL].insert("USE_TESSELLATION");
            resolved_defines[KRL_Scene::PassType::SHADOW].insert("USE_TESSELLATION");
        }

        if (m_displacement_mode == DISPLACEMENT_ABS)
        {
            resolved_defines[KRL_Scene::PassType::NORMAL].insert("HAS_ABS_DISPLACEMENT");
            resolved_defines[KRL_Scene::PassType::NORMAL].insert("USE_TESSELLATION");
            resolved_defines[KRL_Scene::PassType::SHADOW].insert("USE_TESSELLATION");
        }
    }    

    if (m_is_billboard)
    {
        resolved_defines[KRL_Scene::PassType::NORMAL].insert("IS_BILLBOARD");
        resolved_defines[KRL_Scene::PassType::SHADOW].insert("IS_BILLBOARD");
    }

    if (m_has_car_ao)
    {
        resolved_defines[KRL_Scene::PassType::NORMAL].insert("HAS_CAR_AO");
    }

    if (m_is_car_paint)
    {
        resolved_defines[KRL_Scene::PassType::NORMAL].insert("HAS_CAR_PAINT");
    }

    if (m_is_two_sided)
    {
        resolved_defines[KRL_Scene::PassType::NORMAL].insert("IS_TWO_SIDED");
    }

    if(m_use_world_ao)
    {
        resolved_defines[KRL_Scene::PassType::NORMAL].insert("USE_WORLD_AO");
    }
}

void GLB::Material4::CheckUniforms(GLBShader2 *shader, const char *shader_name)
{
	if (!shader)
	{
		// This should not happen
		return;
	}

	// Check the textures
	for (unsigned int i = 0; i < 8; i++)
	{
		if (shader->m_uniform_locations[GLB::uniforms::texture_unit0 + i] > -1 && !m_textures[i])
		{
			INFO("Warning! Shader %s requires texture_unit%d but not set in material: %s", shader_name, i, m_name.c_str());
		}
	}
}

void Material4::SetTextureOverrides(KCL::Texture* ovrds[])
{
    memcpy(&m_texture_overrides[0], &ovrds[0], sizeof(m_texture_overrides));
}

void Material4::Bind(KRL_Scene::PassType::Enum pass_type, KRL_Scene::ShaderVariant::Enum shader_variant, KCL::uint32 &texture_num)
{
	int i = 8;

	GLBShader2 * shader = m_shaders4[pass_type][shader_variant];

	OpenGLStateManager::GlUseProgram( shader->m_p ) ;

	while( i--)
	{
		if (shader->m_uniform_locations[GLB::uniforms::texture_unit0 + i] > -1 && m_textures[i])
		{
            if(m_texture_overrides[i])
            {
                m_texture_overrides[i]->bind(texture_num);

                if (m_texture_overrides[i] == m_scene->GetTopdownShadowMap())
                {
                    glBindSampler(texture_num, m_scene->GetTopdownShadowSampler());
                }
            }
            else
            {
                m_textures[i]->bind(texture_num);
            }

            if (!g_extension->isES())
            {
                //upload gamma uniform on desktop, see common.h
                if(i == 0) //albedo
                {
                    if (shader->m_uniform_locations[GLB::uniforms::gamma_exp] > -1)
		            {
                        float val = 1.0f;
                        if(m_textures[COLOR]->isDataInSRGB())
                        {
                            val = 2.2f;
                        }

                        glUniform4fv(shader->m_uniform_locations[GLB::uniforms::gamma_exp], 1, KCL::Vector4D(val,val,val,val));
		            }
                }
            }

			glUniform1i(shader->m_uniform_locations[GLB::uniforms::texture_unit0 + i], texture_num);
			texture_num++;
		}

		if (shader->m_uniform_locations[GLB::uniforms::texture_unit0 + i] > -1 && (!m_textures[i]))
		{
			//NGLOG_ERROR("Try to bind undefined material texture attribute: %s %s   %s %s",m_name,i,__FILE__,__LINE__) ;
		}

	}	

	switch( m_material_type)
	{
	case SKY:
		{
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlDepthFunc( GL_LEQUAL);
			OpenGLStateManager::GlDepthMask( 0);
			glDepthRangef( 1, 1);
			break;
		}
	case WATER:
		{
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlEnable( GL_BLEND);
			OpenGLStateManager::GlBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		}
	case GLASS:
		{
			OpenGLStateManager::GlDisable( GL_CULL_FACE);
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlEnable( GL_BLEND);

#if ENABLE_LIGHTCOMBINE_FP_RENDER_TARGET
			OpenGLStateManager::GlBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#else
			// Special blending for offscreen target: premultiplied alpha
            OpenGLStateManager::GlBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#endif
			//OpenGLStateManager::GlDepthMask( 0);  // TODO: velocity buffer not correct with transparent objects 
			break;
		}
	case LIGHTSHAFT:
		{
			OpenGLStateManager::GlDepthMask( 0);
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlEnable( GL_BLEND);
			OpenGLStateManager::GlBlendFunc( 1, 1);
			break;
		}
	case FOLIAGE:
		{
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			break;
		}
	case FLAME:
		{
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlEnable( GL_BLEND);
			OpenGLStateManager::GlDepthMask( 0);
			OpenGLStateManager::GlBlendFunc( 1, 1);
			break;
		}
	case FIRE:
		{			
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlEnable( GL_BLEND);
			OpenGLStateManager::GlDepthMask( 0);
			OpenGLStateManager::GlBlendFunc( 1, GL_ONE_MINUS_SRC_ALPHA);
			break;
		}
	case SMOKE:
		{			
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlEnable( GL_BLEND);
			OpenGLStateManager::GlDepthMask( 0);
			OpenGLStateManager::GlBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		}
	case STEAM:
		{			
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlEnable( GL_BLEND);
			OpenGLStateManager::GlDepthMask( 0);
			OpenGLStateManager::GlBlendFunc( 1, GL_ONE_MINUS_SRC_ALPHA);
			break;
		}
	case DECAL:
		{			
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlEnable( GL_BLEND);
			OpenGLStateManager::GlDepthMask( 0);
			OpenGLStateManager::GlBlendFunc( GL_DST_COLOR, GL_SRC_COLOR);
			break;
		}
	case OMNILIGHT:
		{
			break;
		}
	case SHADOWCASTER0:
		{
			break;
		}
	case SHADOWRECEIVER0:
		{
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlEnable( GL_BLEND);
			OpenGLStateManager::GlDepthMask( 0);
			OpenGLStateManager::GlDepthFunc( GL_EQUAL);
			OpenGLStateManager::GlBlendFunc( GL_DST_COLOR, 0);
			break;
		}
	case SHADOWCASTER1:
		{			
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlEnable( GL_CULL_FACE);
			OpenGLStateManager::GlCullFace( GL_FRONT);
			break;
		}
	case SHADOWRECEIVER1:
		{			
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlEnable( GL_BLEND);
			OpenGLStateManager::GlDepthMask( 0);
			OpenGLStateManager::GlDepthFunc( GL_EQUAL);
			OpenGLStateManager::GlBlendFunc( GL_DST_COLOR, 0);
			break;
		}
	case PLANAR_REFLECTION:
		{
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlEnable( GL_BLEND);
			OpenGLStateManager::GlDepthMask( 0);
			OpenGLStateManager::GlDepthFunc( GL_EQUAL);
			OpenGLStateManager::GlBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		}
	default:
		{
			OpenGLStateManager::GlEnable( GL_CULL_FACE);
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
		}
	}

    if(m_is_two_sided)
    {
        OpenGLStateManager::GlDisable( GL_CULL_FACE);
    }
}

void Material4::Unbind()
{
	OpenGLStateManager::GlDepthFunc( GL_LESS);
	OpenGLStateManager::GlDepthMask( 1);
	OpenGLStateManager::GlCullFace( GL_BACK);
	OpenGLStateManager::GlDisable( GL_DEPTH_TEST);
	OpenGLStateManager::GlDisable( GL_CULL_FACE);
	OpenGLStateManager::GlDisable( GL_BLEND);
	glDepthRangef( 0, 1);
}


KCL::KCL_Status Material4::InitShaders( const char* path,  const std::string &max_joint_num_per_mesh)
{		
	INFO("Warning! GLB::Material4::InitShaders is not implemented! Use InitShader(GLB_Scene4*)");
    return KCL::KCL_TESTERROR_NOERROR;
}

void GLB::Material4::preInit( KCL::uint32 &texture_num, int type, int pass_type)
{
	INFO("Warning! GLB::Material4::preInit4 is not implemented! Use Bind()");
}

void GLB::Material4::postInit()
{
	INFO("Warning! GLB::Material4::postInit is not implemented! Use Unbind()");
}

void GLB::Material4::SetSortOrder()
{
	bool tesselated = m_displacement_mode != KCL::Material::NO_DISPLACEMENT;
	bool alpha_tested = m_opacity_mode == ALPHA_TEST;
	bool alpha_blended = m_opacity_mode == ALPHA_BLEND;
	bool opaque = !alpha_tested  && !alpha_blended;
	
	if (m_material_type == SKY)
	{
		// Render the sky last
		m_sort_order = 8;
	}
	else if (tesselated)
	{
		m_sort_order = 0;
	}
	else
	{
		if (opaque)
		{
			m_sort_order = 1;
		}
		else if (alpha_tested)
		{
			m_sort_order = 2;
		}
		else if (alpha_blended)
		{
			m_sort_order = 4;
		}
		else
		{
			INFO("Can not define sort order for material: %s", m_name.c_str());
			m_sort_order = 0;
		}
	}
}

Shader* GLB::Material4::CreateShader(const char* vsfile, const char* fsfile, const std::set<std::string> *defines, KCL::KCL_Status& error)
{
	return Shader::CreateShader(vsfile, fsfile, defines, error);
}
