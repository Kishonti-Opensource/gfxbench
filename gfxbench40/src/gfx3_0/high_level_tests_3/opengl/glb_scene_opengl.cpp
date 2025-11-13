/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "platform.h"

#include "glb_scene_opengl.h"
#include "opengl/glb_texture.h"
#include <kcl_camera2.h>

#if defined HAVE_GLES3 || defined HAVE_GLEW

#include <kcl_planarmap.h>
#include <kcl_room.h>
#include <kcl_actor.h>
#include <kcl_animation4.h>
#include "glb_particlesystem.h"

#include "glb_kcl_adapter.h"
#include "opengl/glb_image.h"
#include "glb_mesh.h"
#include "glb_material.h"
#include "glb_light.h"

#include "opengl/shader.h"
#include "platform.h"
#include "cubemap.h"
#include "shadowmap.h"

#include "opengl/misc2_opengl.h"
#include "opengl/fbo.h"
#include "opengl/glbshader.h"
#include "opengl/texture.h"
#include "opengl/vbopool.h"

#include "misc2.h"
#include "kcl_io.h"

#include "opengl/glb_opengl_state_manager.h"

#include <cmath>
#include <cstddef>
#include <algorithm> //std::sort

using namespace GLB;

const bool occlusion_query_enable = true;
const bool ssao_enabled = false;


#define VAO_enabled GLB::g_extension->hasFeature(GLB::GLBFEATURE_vertex_array_object)

extern bool do_not_skip_samples_passed;

void LoadHudTextures( const std::string &dir);
uint32 Create2DTexture( KCL::uint32 max_miplevels, bool linear, uint32 w, uint32 h, GLint format);
uint32 CreateRenderbuffer( int samples, uint32 w, uint32 h, GLint format);

Filter::Filter()
{
	m_shader = NULL;

	m_fbo = 0;
	m_color_texture = 0;
	m_width = 0;
	m_height = 0;

	//int m_dir;

	m_max_mipmaps = 1;
	m_is_mipmapped = false;

	KCL::Camera2 *m_active_camera = NULL;

	m_onscreen = false;


	for( int i=0; i<8; i++)
	{
		m_input_textures[i] = 0;
	}
}

void Filter::Clear()
{
	GLenum e;
	glBindTexture(GL_TEXTURE_2D, 0);

	if (!m_onscreen)
	{
		glDeleteTextures(1, &m_color_texture);
		e = glGetError();

		GLB::FBO::bind(0);
		glDeleteFramebuffers( 1, &m_fbo);
	}

	e = glGetError();
}

//maxmipcount: 0 means complete mipchain
void Filter::Create( KCL::uint32 depth_attachment, KCL::uint32 w, KCL::uint32 h, bool onscreen, KCL::uint32 maxmipcount, int dir, bool fp)
{
    m_max_mipmaps = maxmipcount;
	m_is_mipmapped = (maxmipcount == 0) || (maxmipcount > 1);

	m_dir = dir;
	m_shader = 0;

	m_width = w;
	m_height = h;

    m_onscreen = onscreen;

	if( !onscreen)
	{
		m_color_texture = Create2DTexture( maxmipcount, true, m_width, m_height, fp ? GL_RGB10_A2 : GL_RGBA8);

		glGenFramebuffers( 1, &m_fbo);

		glBindFramebuffer( GL_FRAMEBUFFER, m_fbo);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_texture, 0);

        if(depth_attachment != 0)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_attachment, 0);
        }

		FBO::bind(0);
	}
	else
	{
		m_fbo = 0;
	}
}

bool g_ForceScreenRender = false; //helper for post

void Filter::SetUniforms()
{
	if (m_shader->m_uniform_locations[GLB::uniforms::offset_2d] > -1)
	{
		if( m_dir)
		{
			glUniform2f(m_shader->m_uniform_locations[GLB::uniforms::offset_2d], 1.0f / m_width, 0.0f);
		}
		else
		{
			glUniform2f(m_shader->m_uniform_locations[GLB::uniforms::offset_2d], 0.0f, 1.0f / m_height);
		}
	}

	if (m_shader->m_uniform_locations[GLB::uniforms::inv_resolution] > -1)
	{
		glUniform2f(m_shader->m_uniform_locations[GLB::uniforms::inv_resolution], 1.0f / m_width, 1.0f / m_height);
	}

	if (m_shader->m_uniform_locations[GLB::uniforms::depth_parameters] > -1 && m_active_camera)
	{
		glUniform4fv(m_shader->m_uniform_locations[GLB::uniforms::depth_parameters], 1, m_active_camera->m_depth_linearize_factors.v);
	}

	if (m_shader->m_uniform_locations[GLB::uniforms::camera_focus] > -1)
	{
		glUniform1f(m_shader->m_uniform_locations[GLB::uniforms::camera_focus], m_focus_distance);
	}

	if (m_shader->m_uniform_locations[GLB::uniforms::dof_strength] > -1 && m_active_camera)
	{
		glUniform1f(m_shader->m_uniform_locations[GLB::uniforms::dof_strength], m_active_camera->GetFov());
	}
}

#if defined OCCLUSION_QUERY_BASED_STAT
void Filter::Render( KCL::uint32 &num_draw_calls, KCL::uint32 &num_triangles , KCL::uint32 &num_vertices , std::set<KCL::uint32> &textureCounter , KCL::int32 &num_samples_passed , KCL::int32 &num_instruction, GLSamplesPassedQuery *glGLSamplesPassedQuery)
#else
void Filter::Render( KCL::uint32 &num_draw_calls, KCL::uint32 &num_triangles , KCL::uint32 &num_vertices , std::set<KCL::uint32> &textureCounter , KCL::int32 &num_samples_passed , KCL::int32 &num_instruction)
#endif
{
	GLB::OpenGLStateManager::GlUseProgram( m_shader->m_p);


	if(!m_fbo || g_ForceScreenRender) FBO::bind(NULL); else
		glBindFramebuffer( GL_FRAMEBUFFER, m_fbo);

    if (m_onscreen)
    {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    else
    {
        glClear( GL_COLOR_BUFFER_BIT);
    }

	glViewport( 0, 0, m_width, m_height);

	int i = 8;
	while( i--)
	{
		if (m_shader->m_uniform_locations[GLB::uniforms::texture_unit0 + i] > -1 && m_input_textures[i])
		{
			GLB::OpenGLStateManager::GlActiveTexture( GL_TEXTURE0 + i);
			glUniform1i(m_shader->m_uniform_locations[GLB::uniforms::texture_unit0 + i], i);
			glBindTexture( GL_TEXTURE_2D, m_input_textures[i]);
            glBindSampler(i, 0);

#ifdef TEXTURE_COUNTING
			textureCounter.insert( m_input_textures[i] );
#endif
		}
	}

	SetUniforms();

	if( !VAO_enabled)
	{
		GLB::OpenGLStateManager::GlEnableVertexAttribArray( m_shader->m_attrib_locations[attribs::in_position]);
		glVertexAttribPointer(m_shader->m_attrib_locations[attribs::in_position], 2, GL_FLOAT, GL_FALSE, sizeof(Vector4D), 0);
	}

	GLB::OpenGLStateManager::Commit();
#ifdef OCCLUSION_QUERY_BASED_STAT
	glGLSamplesPassedQuery->Begin();
#endif
	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4);

	if( !VAO_enabled)
	{
		GLB::OpenGLStateManager::GlDisableVertexAttribArray(m_shader->m_attrib_locations[attribs::in_position]);
	}

	num_draw_calls++;
	num_triangles += 2;
	num_vertices += 4;

#ifdef OCCLUSION_QUERY_BASED_STAT
	glGLSamplesPassedQuery->End();
	num_samples_passed += glGLSamplesPassedQuery->Result();

#ifdef CALCULATE_SHADER_INSTRUCTION_COUNT
	num_instruction += m_shader->m_instruction_count_v * 4 + m_shader->m_instruction_count_f * glGLSamplesPassedQuery->Result();
#endif

#endif

#ifdef DEBUG
	if( m_is_mipmapped)
	{
		//printf("!warning: filter m_is_mipmapped, call glGenerateMipmap.\n");
	}
#endif
}



static GLenum b[] =
{
	GL_COLOR_ATTACHMENT0,
	GL_COLOR_ATTACHMENT1,
	GL_COLOR_ATTACHMENT2,
	GL_COLOR_ATTACHMENT3
};

KCL::uint16 frustum_indices[] =
{
	0, 1, 4,
	1, 5, 4,

	1, 3, 5,
	3, 7, 5,

	3, 2, 7,
	2, 6, 7,

	2, 0, 6,
	0, 4, 6,

	2, 3, 0,
	3, 1, 0,
	4, 5, 6,
	5, 7, 6
};

KCL::uint32 hud_texids[16];

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

typedef GLB::Matrix4x4 mat4;
typedef KCL::Vector4D vec4;
typedef KCL::Vector3D vec3;
typedef KCL::Vector2D vec2;
typedef VectorI4D ivec4;
#define layout(std140) struct
#define uniform
#include "ubo_frameConsts.h"
#include "ubo_matConsts.h"
#include "ubo_meshConsts.h"
#include "ubo_lightConsts.h"
#include "ubo_emitterAdvectConsts.h"

frameConsts uFrameConsts;
matConsts uMatConsts;
meshConsts uMeshConsts;
lightConsts uLightConsts;
emitterAdvectConsts uAdvectConsts;

Filter * GLB_Scene_ES3::CreateFilter()
{
	return new Filter;
}

void GLB_Scene_ES3::CreateLBOs()
{
	std::vector<Vector3D> sphere_vertices;
	std::vector<Vector2D> sphere_tcs;
	std::vector<KCL::uint16> sphere_indices;

	std::vector<Vector3D> cone_vertices;
	std::vector<Vector2D> cone_tcs;
	std::vector<KCL::uint16> cone_indices;

	KCL::Mesh3::CreateSphere( sphere_vertices, sphere_tcs, sphere_indices, 10, 10);
	KCL::Mesh3::CreateCone( cone_vertices, cone_tcs, cone_indices, 15, 1);

	glGenBuffers(1, &m_lbos[0].m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_lbos[0].m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(KCL::Vector3D) * sphere_vertices.size(), sphere_vertices[0].v, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_lbos[0].m_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_lbos[0].m_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(KCL::uint16) * sphere_indices.size(), &sphere_indices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_lbos[0].m_num_indices = static_cast<uint32>(sphere_indices.size());

	glGenBuffers(1, &m_lbos[1].m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_lbos[1].m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(KCL::Vector3D) * cone_vertices.size(), cone_vertices[0].v, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_lbos[1].m_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_lbos[1].m_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(KCL::uint16) * cone_indices.size(), &cone_indices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_lbos[1].m_num_indices = static_cast<uint32>(cone_indices.size());

	for( int i=0; i<2; i++)
	{
		glGenVertexArrays( 1, &m_lbos[i].m_vao);

		glBindVertexArray( m_lbos[i].m_vao);
		glBindBuffer( GL_ARRAY_BUFFER, m_lbos[i].m_vbo);
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_lbos[i].m_ebo);

		glEnableVertexAttribArray( 0);
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindVertexArray( 0);
		glBindBuffer( GL_ARRAY_BUFFER, 0);
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}

KCL::KCL_Status GLB_Scene_ES3::reloadShaders()
{
	return GLB_Scene_ES2::reloadShaders();
}

bool GLB_Scene_ES3::UseEnvmapMipmaps()
{
	return false;
}

KCL::KCL_Status GLB_Scene_ES3::Process_GL( GLB::FBO_COLORMODE color_mode, GLB::FBO_DEPTHMODE depth_mode, int samples)
{
#ifdef USE_UBOs
	//create and fill uniform buffers
	glGenBuffers(COUNT_OF(m_UBO_ids), m_UBO_ids);

	glBindBuffer(GL_UNIFORM_BUFFER, m_UBO_ids[Shader::sUBOnames::Frame]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(uFrameConsts), NULL, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_UBO_ids[Shader::sUBOnames::Frame]);

	glBindBuffer(GL_UNIFORM_BUFFER, m_UBO_ids[Shader::sUBOnames::Mat]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(uMatConsts), NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_UBO_ids[Shader::sUBOnames::Mat]);

	glBindBuffer(GL_UNIFORM_BUFFER, m_UBO_ids[Shader::sUBOnames::Mesh]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(uMeshConsts), NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_UBO_ids[Shader::sUBOnames::Mesh]);

	glBindBuffer(GL_UNIFORM_BUFFER, m_UBO_ids[Shader::sUBOnames::Light]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(uLightConsts), NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 3, m_UBO_ids[Shader::sUBOnames::Light]);

	glBindBuffer(GL_UNIFORM_BUFFER, m_UBO_ids[Shader::sUBOnames::EmitterAdvect]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(uAdvectConsts), NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 4, m_UBO_ids[Shader::sUBOnames::EmitterAdvect]);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
#endif

	for (unsigned int i = 0; i < FILTER_COUNT; i++)
	{
		filters[i] = CreateFilter();
	}

    KCL::uint32 light_noise_create_flags = KCL::TC_Commit;
    if (m_scene_version == KCL::SV_31)
    {
        light_noise_create_flags |= KCL::TC_NoMipmap;
    }
    m_light_noise = TextureFactory().CreateAndSetup( KCL::Texture_2D, "common/light.png", light_noise_create_flags);
    m_particle_noise = TextureFactory().CreateAndSetup( KCL::Texture_2D, "common/noise2_color.png", KCL::TC_Commit | KCL::TC_NearestFilter | KCL::TC_NoMipmap);

	//LoadHudTextures( m_path + std::string(ImagesDirectory()) + std::string("hud/"));
    m_fire_texid = TextureFactory().CreateAndSetup( KCL::Texture_3D,  (ImagesDirectory() + std::string("anims/fire")).c_str(), KCL::TC_Commit | KCL::TC_Clamp);
	m_fog_texid = TextureFactory().CreateAndSetup(KCL::Texture_3D, (ImagesDirectory() + std::string("anims/fog")).c_str(), KCL::TC_Commit);
	//smoke_texid = LoadTexture3D( m_path + ImagesDirectory() + std::string("anims/caustic") );

	m_samples = samples ;
	KCL::KCL_Status pp_init_ret = CreateBuffers() ;

	if( pp_init_ret != KCL::KCL_TESTERROR_NOERROR)
	{
		return KCL::KCL_TESTERROR_REQUIRED_FSAA_LEVEL_NOT_SUPPORTED;
	}

	CreateLBOs();
	glGenBuffers(1, &m_particles_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_particles_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(KCL::ParticleRenderAttrib2) * sizeof(float) * MAX_PARTICLES_PER_EMITTER, 0, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	{
	glGenBuffers(1, &m_lightshaft_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_lightshaft_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(KCL::Vector3D) * 1024, 0, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_lightshaft_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_lightshaft_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16) * 1024, 0, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glGenVertexArrays( 1, &m_lightshaft_vao);

		glBindVertexArray( m_lightshaft_vao);
		glBindBuffer( GL_ARRAY_BUFFER, m_lightshaft_vbo);
		glVertexAttribPointer( 0, 3, GL_FLOAT, 0, sizeof(KCL::Vector3D), 0);
		glEnableVertexAttribArray( 0);

		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_lightshaft_ebo);

		glBindVertexArray( 0);
		glBindBuffer( GL_ARRAY_BUFFER, 0);
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	{
		glGenBuffers(1, &m_occlusion_query_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, m_occlusion_query_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(KCL::Vector3D) * 128, 0, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenVertexArrays( 1, &m_occlusion_query_vao);

		glBindVertexArray( m_occlusion_query_vao);
		glBindBuffer( GL_ARRAY_BUFFER, m_occlusion_query_vbo);
		glVertexAttribPointer( 0, 3, GL_FLOAT, 0, sizeof(KCL::Vector3D), 0);
		glEnableVertexAttribArray( 0);

		glBindVertexArray( 0);
		glBindBuffer( GL_ARRAY_BUFFER, 0);
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	//create buffers for TF_emitters
	for(std::vector<KCL::Actor*>::size_type i=0; i<m_actors.size(); ++i)
	{
#if defined ENABLE_FRAME_CAPTURE
#pragma message("Framecapture: ReadEmitter disabled")
		m_actors[i]->m_emitters.clear();
#endif

		for(std::vector<KCL::AnimatedEmitter*>::size_type j=0; j<m_actors[i]->m_emitters.size(); ++j)
		{
			KCL::_emitter* em = dynamic_cast<KCL::_emitter*>(m_actors[i]->m_emitters[j]);
			em->Process();
		}
	}
/*
	//fill volume light data with static lights
	memset(m_VolumeLightData, 0x0, sizeof(m_VolumeLightData));

	AABB lightBounds;
	std::vector<KCL::Light*> staticLights;
	{
		for(std::vector<KCL::Actor*>::size_type i=0; i<m_actors.size(); ++i)
		{
			for(std::vector<Light*>::size_type j=0; j<m_actors[i]->m_lights.size(); ++j)
			{
				if(m_actors[i]->m_lights[j]->m_intensity_track == NULL)
				{
					KCL::Light* l = m_actors[i]->m_lights[j];
					staticLights.push_back(l);

					KCL::Vector3D lpos(l->m_world_pom.getTranslation());
					KCL::Vector3D lrad(l->m_radius, l->m_radius, l->m_radius);
					AABB lbounds(lpos - lrad, lpos + lrad);
					lightBounds.Merge(lpos); //do we have a valid world pom here???
				}
			}
		}
	}

	m_VolumeLightMin = lightBounds.GetMinVertex();
	m_VolumeLightExtents = lightBounds.GetMaxVertex() - lightBounds.GetMinVertex();

	float maxDim = std::max<float>(m_VolumeLightExtents.x, m_VolumeLightExtents.z);
	m_VolumeLightExtents = KCL::Vector3D(maxDim,0.0f,maxDim);

	float tileSize = std::min<float>(m_VolumeLightExtents.x, m_VolumeLightExtents.z) / 64.0f;

	for(int x=0; x<64; ++x)
	{
		for(int y=0; y<64; ++y)
		{
			KCL::Vector3D tileCenter = m_VolumeLightMin;
			tileCenter.x += float(x)/64.0f*m_VolumeLightExtents.x;
			tileCenter.z += float(y)/64.0f*m_VolumeLightExtents.z;

			for(std::vector<Light*>::size_type l=0; l<staticLights.size(); ++l)
			{
				if( (staticLights[l]->m_radius > tileSize * 2.0f) && (staticLights[l]->m_light_type == Light::OMNI) )
				{
					KCL::Vector3D lightPos = staticLights[l]->m_world_pom.getTranslation();
					KCL::Vector3D color = staticLights[l]->m_diffuse_color / 2.0f;//.normalize();
					//float intensity = staticLights[l]->m_diffuse_color.length();
					float dist = (lightPos - tileCenter).length();
					if( dist < staticLights[l]->m_radius) //TODO attenuation
					{
						float atten = 1.0f - sqrtf(dist / staticLights[l]->m_radius);
						m_VolumeLightData[x][y][0] = std::min<KCL::uint8>(255, m_VolumeLightData[x][y][0] + int(color.x * atten * 255.0f));
						m_VolumeLightData[x][y][1] = std::min<KCL::uint8>(255, m_VolumeLightData[x][y][1] + int(color.y * atten * 255.0f));
						m_VolumeLightData[x][y][2] = std::min<KCL::uint8>(255, m_VolumeLightData[x][y][2] + int(color.z * atten * 255.0f));
					}
				}
			}
		}
	}

	glGenTextures(1, &m_VolumeLightTex);
	glBindTexture( GL_TEXTURE_2D, m_VolumeLightTex);

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//glTexImage2D( GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, 64, 64, 0, GL_RGB, GL_FLOAT, m_VolumeLightData);
	glTexStorage2D( GL_TEXTURE_2D, 1, GL_RGB8, 64, 64);
	glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 64, 64, GL_RGB, GL_UNSIGNED_BYTE, m_VolumeLightData);

	//for (unsigned int i = 0; i < images.size(); i++)
	//{
	//	glTexSubImage2D( GL_TEXTURE_3D, 0, 0, 0, i, images[i]->getWidth(), images[i]->getHeight(), 1, GL_RGBA, GL_UNSIGNED_BYTE, images[i]->data());
	//}
	glBindTexture( GL_TEXTURE_2D, 0);
*/

	/*
		init light oqs and results
		should be in the light constructor, but that's cross platform code.
	*/
	for(size_t i=0; i<m_actors.size(); i++)
	{
		Actor* a = m_actors[i];
		for(size_t j=0; j<a->m_lights.size(); j++)
		{
			GLB::Light* l = static_cast<GLB::Light*>(a->m_lights[j]);

			if (l->m_has_lensflare)
			{
				glGenQueries(4, l->m_query_objects);
			}
		}
	}

	{
		Vector4D v[4] =
		{
			Vector4D( -1, -1, 0, 0),
			Vector4D( 1, -1, 1, 0),
			Vector4D( -1, 1, 0, 1),
			Vector4D( 1, 1, 1, 1)
		};

		glGenBuffers( 1, &m_fullscreen_quad_vbo);
		glBindBuffer( GL_ARRAY_BUFFER, m_fullscreen_quad_vbo);
		glBufferData_chunked( GL_ARRAY_BUFFER, 4 * 4 * sizeof(float), v[0].v, GL_STATIC_DRAW);
		glBindBuffer( GL_ARRAY_BUFFER, 0);

		glGenVertexArrays( 1, &m_fullscreen_quad_vao);

		glBindVertexArray( m_fullscreen_quad_vao);
		glBindBuffer( GL_ARRAY_BUFFER, m_fullscreen_quad_vbo);
		glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( Vector4D), 0);
		glEnableVertexAttribArray( 0);

		glBindVertexArray( 0);
		glBindBuffer( GL_ARRAY_BUFFER, 0);
	}

	KCL::KCL_Status result = GLB_Scene_ES2::Process_GL( color_mode, depth_mode, samples);

	if( result != KCL::KCL_TESTERROR_NOERROR)
	{
		return result;
	}

	if (VAO_enabled)
	{
		m_vao_storage = new VaoStorage() ;
	}

	return KCL::KCL_TESTERROR_NOERROR;
}

GLB_Scene_ES3::GLB_Scene_ES3()
{
	for (unsigned int i = 0; i < FILTER_COUNT; i++)
	{
		filters[i] = NULL;
	}

	m_skip_create_shadow_decal = false;

	m_fire_texid = NULL;
	m_smoke_texid = NULL;
	m_fog_texid = NULL;

	m_particle_noise = NULL;
	m_light_noise = NULL;

  	m_lightshaft_vbo = 0;
	m_lightshaft_ebo = 0;
	m_lightshaft_vao = 0;

	m_occlusion_query_vbo = 0;
	m_occlusion_query_vao = 0;

	m_fullscreen_quad_vao = 0;

	m_vao_storage = 0;
}

GLB_Scene_ES3::~GLB_Scene_ES3()
{
	for(size_t i=0; i<m_actors.size(); i++)
	{
		Actor* a = m_actors[i];
		for(size_t j=0; j<a->m_lights.size(); j++)
		{
            GLB::Light* l = static_cast<GLB::Light*>(a->m_lights[j]);

			if (l->m_has_lensflare)
			{
				glDeleteQueries(4, l->m_query_objects);
			}
		}
	}

	DeleteBuffers() ;

#ifdef USE_UBOs
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	for(int i=0; i<COUNT_OF(m_UBO_ids); ++i)
	{
		if(m_UBO_ids[i])
		{
			glDeleteBuffers(1, &m_UBO_ids[i]);
		}
	}
#endif

	delete m_light_noise;
	delete m_particle_noise;
	delete m_fire_texid;
	delete m_fog_texid;

	glDeleteVertexArrays( 1, &m_fullscreen_quad_vao);

	glDeleteBuffers( 1, &m_occlusion_query_vbo);
	glDeleteVertexArrays( 1, &m_occlusion_query_vao);

	glDeleteBuffers( 1, &m_lbos[0].m_vbo);
	glDeleteBuffers( 1, &m_lbos[0].m_ebo);
	glDeleteVertexArrays( 1, &m_lbos[0].m_vao);
	glDeleteBuffers( 1, &m_lbos[1].m_vbo);
	glDeleteBuffers( 1, &m_lbos[1].m_ebo);
	glDeleteVertexArrays( 1, &m_lbos[1].m_vao);

	if( m_lightshaft_vbo)
	{
		glDeleteBuffers(1, &m_lightshaft_vbo);
		m_lightshaft_vbo = 0;
	}
	if( m_lightshaft_ebo)
	{
		glDeleteBuffers(1, &m_lightshaft_ebo);
		m_lightshaft_ebo = 0;
	}
	if( m_lightshaft_vao)
	{
		glDeleteVertexArrays(1, &m_lightshaft_vao);
		m_lightshaft_vao = 0;
	}

	for (unsigned int i = 0; i < FILTER_COUNT; i++)
	{
		delete filters[i];
	}

	delete m_vao_storage ;
}

KCL::KCL_Status GLB_Scene_ES3::CreateBuffers()
{
	bool pp_init_ret = pp.Init( m_viewport_width, m_viewport_height, m_samples, m_scene_version == KCL::SV_31);

	if( !pp_init_ret)
	{
		return KCL::KCL_TESTERROR_REQUIRED_FSAA_LEVEL_NOT_SUPPORTED;
	}

	filters[0]->Create( 0, m_viewport_width, m_viewport_height, true, 1, 0);
	filters[1]->Create( 0, m_viewport_width / 2.0f, m_viewport_height / 2.0f, false, 1, 0);
	filters[2]->Create( 0, m_viewport_width / 2.0f, m_viewport_height / 2.0f, false, 1, 1);
	filters[3]->Create( 0, m_viewport_width / 4.0f, m_viewport_height / 4.0f, false, 1, 0);
	filters[4]->Create( 0, m_viewport_width / 4.0f, m_viewport_height / 4.0f, false, 1, 1);
	filters[5]->Create( 0, m_viewport_width / 8.0f, m_viewport_height / 8.0f, false, 1, 0);
	filters[6]->Create( 0, m_viewport_width / 8.0f, m_viewport_height / 8.0f, false, 1, 1);
	filters[7]->Create( 0, m_viewport_width / 16.0f, m_viewport_height / 16.0f, false, 1, 0);
	filters[8]->Create( 0, m_viewport_width / 16.0f, m_viewport_height / 16.0f, false, 1, 1);

    if(m_disabledRenderBits & RenderBits::ERB_Post)
    {
        filters[10]->Create( pp.m_depth_texture, m_viewport_width, m_viewport_height, true, 1, 0);
    }
    else //default behaviour
    {
        filters[10]->Create( pp.m_depth_texture, m_viewport_width, m_viewport_height, false, 4, 0);
    }

	return KCL::KCL_TESTERROR_NOERROR ;
}

void GLB_Scene_ES3::DeleteBuffers()
{
	pp.Clear();

	for(int i=0; i<COUNT_OF(filters); ++i)
	{
		if (filters[i])
		{
			filters[i]->Clear();
		}
	}
}


void Render_Frustum_Of_Camera_From_Eye(const Camera2& camera, const Camera2& eye, Shader *s);

//#define MATERIAL_SORT_DRAWCALLS
#define DEPTH_SORT_DRAWCALLS

static bool material_compare(Mesh* A, Mesh* B)
{
	return (A->m_material->m_name < B->m_material->m_name);
}

struct mesh_depth_sort_info
{
	KCL::Mesh *mesh;
	Vector2D extents;
    float vertexCenterDist;
};

static bool depth_compare(const mesh_depth_sort_info &A, const mesh_depth_sort_info &B)
{
    return A.vertexCenterDist < B.vertexCenterDist;
}

static void sort_vis_list(std::vector<KCL::Mesh*> &visible_meshes, KCL::Camera2 *camera)
{
	std::vector<mesh_depth_sort_info> sorted_visible_meshes;

	for (uint32 i = 0; i < visible_meshes.size(); i++)
	{
		mesh_depth_sort_info mesh_info;

		mesh_info.mesh = visible_meshes[i];
		mesh_info.extents = visible_meshes[i]->m_aabb.DistanceFromPlane(camera->GetCullPlane(KCL::CULLPLANE_NEAR));
        mesh_info.vertexCenterDist = Vector4D::dot( KCL::Vector4D(visible_meshes[i]->m_vertexCenter), camera->GetCullPlane(KCL::CULLPLANE_NEAR));

		// force objects with infinite bounds (actors) to draw first
		if (mesh_info.extents.x < -FLT_MAX || mesh_info.extents.y > FLT_MAX)
		{
			mesh_info.extents.x = 0.0f;
			mesh_info.extents.y = 0.0f;
            mesh_info.vertexCenterDist = 0.0f;
		}
		sorted_visible_meshes.push_back(mesh_info);
	}

	// depth sort
	std::sort (sorted_visible_meshes.begin(), sorted_visible_meshes.end(), &depth_compare);

	// remap original visible meshes
	for (uint32 i = 0; i < visible_meshes.size(); i++)
		visible_meshes[i] = sorted_visible_meshes[i].mesh;
}


void GLB_Scene_ES3::CollectInstances( std::vector<KCL::Mesh*> &visible_meshes)
{
#if defined ENABLE_FRAME_CAPTURE
#pragma message("Framecapture: CollectInstances disabled")
	return;
#endif

    KRL_Scene::CollectInstances(visible_meshes);

    for( uint32 j=0; j<visible_meshes.size(); j++)
	{
		KCL::Mesh* m = visible_meshes[j];
        GLB::Mesh3 *gm = static_cast<GLB::Mesh3*>(m->m_mesh);
        size_t dataCount = sizeof(gm->m_instances[m->m_material][0]) * gm->m_instances[m->m_material].size();
        gm->UpdateInstanceVBO(gm->m_instances[m->m_material][0].mv.v, static_cast<int>(dataCount));
	}
}

void GLB_Scene_ES3::Render( Camera2* camera, std::vector<Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light, int pass_type)
{
	GLB::Material *override_material = dynamic_cast<GLB::Material*>(_override_material);
	GLB::Material *last_material = NULL;
	int last_mesh_type = -1;
	KCL::uint32 texture_num_from_material = 0;

	float normalized_time = (float) (m_animation_time % 100000) / 100000.0f;

#ifdef PER_FRAME_ALU_INFO
	uint32 samplesPassed = 0;
#endif

	if(visible_meshes.size())
	{
#ifdef DEPTH_SORT_DRAWCALLS
		/* depth sort */
		sort_vis_list(visible_meshes, camera);
#elif defined MATERIAL_SORT_DRAWCALLS
		/* material sort */
		std::sort (visible_meshes.begin(), visible_meshes.end(), &material_compare);
		//qsort( &visible_meshes[0], visible_meshes.size(), sizeof( Mesh*), &material_compare);
#endif
#ifdef USE_UBOs
		//upload per-frame constants
		uFrameConsts.global_light_dirXYZ_pad = KCL::Vector4D(m_light_dir);
		uFrameConsts.global_light_colorXYZ_pad = KCL::Vector4D(m_light_color);
		uFrameConsts.view_dirXYZ_pad = KCL::Vector4D( -camera->GetView().v[2], -camera->GetView().v[6], -camera->GetView().v[10], 0.0f);
		uFrameConsts.view_posXYZ_time = KCL::Vector4D(camera->GetEye().x, camera->GetEye().y, camera->GetEye().z, normalized_time);
		uFrameConsts.background_colorXYZ_fogdensity = KCL::Vector4D(m_background_color.x, m_background_color.y, m_background_color.z, m_fog_density);
		uFrameConsts.depth_parameters = m_active_camera->m_depth_linearize_factors;
		uFrameConsts.inv_resolutionXY_pad = KCL::Vector4D(1.0f / m_viewport_width, 1.0f / m_viewport_height, 0.0f, 0.0f);
		glBindBuffer(GL_UNIFORM_BUFFER, m_UBO_ids[Shader::sUBOnames::Frame]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(uFrameConsts), (const void*)(&uFrameConsts), GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_UBO_ids[Shader::sUBOnames::Frame]);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
#endif

		if( &visible_meshes == &m_visible_meshes[0])
		CollectInstances( visible_meshes);
	}

	for( uint32 j=0; j<visible_meshes.size(); j++)
	{
#ifdef PER_FRAME_ALU_INFO
		samplesPassed = 0;
		glBeginQuery(GL_SAMPLES_PASSED, Shader::QueryId());
#endif

		Mesh* sm = (Mesh*)visible_meshes[j];
		KRL::Mesh3 *krl_mesh  = (KRL::Mesh3 *)sm->m_mesh;

		if( !sm->m_mesh)
		{
			continue;
		}

		int mesh_type = sm->m_mesh->m_vertex_matrix_indices.size() != 0;

		if( krl_mesh->m_instances[sm->m_material].size() > 1)
		{
			std::set<KCL::Material*>::iterator iter = krl_mesh->m_is_rendered.find( sm->m_material);

			if( iter != krl_mesh->m_is_rendered.end())
			{
				continue;
			}

			mesh_type = 2;
		}

		GLB::Material *material = dynamic_cast<GLB::Material*>(sm->m_material);

		if( override_material)
		{
			material = override_material;
		}

		/* force use of shader[1][?] for depth prepass */
		int shader_bank = (pass_type == -1) ? 1 : 0;

		Shader *s = material->m_shaders[shader_bank][mesh_type];
		KRL_CubeEnvMap *envmaps[2];
		float envmaps_interpolator = 0.0f;
		GLB::Matrix4x4 mvp;
		GLB::Matrix4x4 mv;
		GLB::Matrix4x4 model;
		GLB::Matrix4x4 inv_model;
		GLB::Matrix4x4 inv_modelview;
		Vector3D pos( sm->m_world_pom.v[12], sm->m_world_pom.v[13], sm->m_world_pom.v[14]);

		if( last_material != material || last_mesh_type != mesh_type)
		{
			if( last_material)
			{
				last_material->postInit();
			}

			texture_num_from_material = 0;


			if( material->m_ogg_decoder)
			{
				if( material->m_frame_when_animated != m_frame)
				{
					material->DecodeVideo();
					material->m_frame_when_animated = m_frame;
				}
			}


			material->preInit( texture_num_from_material, mesh_type, pass_type);
#ifdef USE_UBOs
            uMatConsts.matparams_disiseri = KCL::Vector4D(material->m_diffuse_intensity, material->m_specular_intensity, material->m_specular_exponent, material->m_reflect_intensity);
            uMatConsts.fresnelXYZ_transp = KCL::Vector4D(material->m_fresnel_params.x, material->m_fresnel_params.y, material->m_fresnel_params.z, material->m_transparency);
			glBindBuffer(GL_UNIFORM_BUFFER, m_UBO_ids[Shader::sUBOnames::Mat]);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(uMatConsts), (const void*)(&uMatConsts), GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_UBO_ids[Shader::sUBOnames::Mat]);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
#endif
		}

		KCL::uint32 texture_num = texture_num_from_material;

		last_material = material;

		last_mesh_type = mesh_type;

		switch( mesh_type)
		{
		case 0:
			{
				if( material->m_material_type == KCL::Material::SKY)
				{
					mvp = sm->m_world_pom * camera->GetViewProjectionOrigo();
				}
				else
				{
					mvp = sm->m_world_pom * camera->GetViewProjection();
				}

				mv = sm->m_world_pom * camera->GetView();
				GLB::Matrix4x4::InvertModelView(mv, inv_modelview);

				model = sm->m_world_pom;
				inv_model = Matrix4x4::Invert4x3( sm->m_world_pom);
				break;
			}
		case 2:
		case 1:
			{
				mvp = camera->GetViewProjection();
				mv = camera->GetView();
				GLB::Matrix4x4::InvertModelView(mv, inv_modelview);
				model.identity();
				inv_model.identity();
				break;
			}
		}
#ifdef USE_UBOs
		uMeshConsts.mvp = mvp;
		uMeshConsts.mv = mv;
		uMeshConsts.model = model;
		uMeshConsts.inv_model = inv_model;
		uMeshConsts.inv_modelview = inv_modelview;
		glBindBuffer(GL_UNIFORM_BUFFER, m_UBO_ids[Shader::sUBOnames::Mesh]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(uMeshConsts), (const void*)(&uMeshConsts), GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_UBO_ids[Shader::sUBOnames::Mesh]);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
#else //meshconsts
		glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::mvp], 1, GL_FALSE, mvp.v);

		if (s->m_uniform_locations[GLB::uniforms::mv] > -1)
		{
			glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::mv], 1, GL_FALSE, mv.v);
		}

		if (s->m_uniform_locations[GLB::uniforms::model] > -1)
		{
			glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::model], 1, GL_FALSE, model.v);
		}

		if (s->m_uniform_locations[GLB::uniforms::inv_model] > -1)
		{
			glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::inv_model], 1, GL_FALSE, inv_model.v);
		}

		if (s->m_uniform_locations[GLB::uniforms::inv_modelview] > -1)
		{
			glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::inv_modelview], 1, GL_FALSE, inv_modelview.v);
		}
#endif
		if (s->m_uniform_locations[GLB::uniforms::bones] > -1 && sm->m_mesh->m_node_matrices.size())
		{
			glUniform4fv(s->m_uniform_locations[GLB::uniforms::bones], static_cast<GLsizei>(3 * sm->m_mesh->m_nodes.size()), sm->m_mesh->m_node_matrices.data());
		}

#ifndef USE_UBOs //frameconsts
		if (s->m_uniform_locations[GLB::uniforms::global_light_dir] > -1)
		{
			glUniform3fv(s->m_uniform_locations[GLB::uniforms::global_light_dir], 1, m_light_dir.v);
		}

		if (s->m_uniform_locations[GLB::uniforms::global_light_color] > -1)
		{
			glUniform3fv(s->m_uniform_locations[GLB::uniforms::global_light_color], 1, m_light_color.v);
		}

		if (s->m_uniform_locations[GLB::uniforms::view_dir] > -1)
		{
			Vector3D view_dir( -camera->GetView().v[2], -camera->GetView().v[6], -camera->GetView().v[10]);
			glUniform3fv(s->m_uniform_locations[GLB::uniforms::view_dir], 1, view_dir.v);
		}

		if (s->m_uniform_locations[GLB::uniforms::view_pos] > -1)
		{
			glUniform3fv(s->m_uniform_locations[GLB::uniforms::view_pos], 1, camera->GetEye().v);
		}

		if (s->m_uniform_locations[GLB::uniforms::time] > -1)
		{
			glUniform1f(s->m_uniform_locations[GLB::uniforms::time], normalized_time);
		}

		if (s->m_uniform_locations[GLB::uniforms::background_color] > -1)
		{
			glUniform3fv(s->m_uniform_locations[GLB::uniforms::background_color], 1, m_background_color.v);
		}

		if (s->m_uniform_locations[GLB::uniforms::fog_density] > -1)
		{
			glUniform1f(s->m_uniform_locations[GLB::uniforms::fog_density], m_fog_density);
		}
#endif

#ifndef USE_UBOs //matconsts
		if (s->m_uniform_locations[GLB::uniforms::diffuse_intensity] > -1)
		{
			glUniform1f(s->m_uniform_locations[GLB::uniforms::diffuse_intensity], material->m_diffuse_intensity);
		}

		if (s->m_uniform_locations[GLB::uniforms::specular_intensity] > -1)
		{
			glUniform1f(s->m_uniform_locations[GLB::uniforms::specular_intensity], material->m_specular_intensity);
		}

		if (s->m_uniform_locations[GLB::uniforms::reflect_intensity] > -1)
		{
			glUniform1f(s->m_uniform_locations[GLB::uniforms::reflect_intensity], material->m_reflect_intensity);
		}

		if (s->m_uniform_locations[GLB::uniforms::specular_exponent] > -1)
		{
			glUniform1f(s->m_uniform_locations[GLB::uniforms::specular_exponent], material->m_specular_exponent);
		}

		if (s->m_uniform_locations[GLB::uniforms::transparency] > -1)
		{
			glUniform1f(s->m_uniform_locations[GLB::uniforms::transparency], material->m_transparency);
		}

		if (s->m_uniform_locations[GLB::uniforms::fresnel_params] > -1)
		{
			glUniform3fv(s->m_uniform_locations[GLB::uniforms::fresnel_params], 1, material->m_fresnel_params.v);
		}
#endif

		if (s->m_uniform_locations[GLB::uniforms::envmaps_interpolator] > -1 && m_cubemaps.size())
		{
			Get2Closest( pos, envmaps[0], envmaps[1], envmaps_interpolator);

			glUniform1f(s->m_uniform_locations[GLB::uniforms::envmaps_interpolator], envmaps_interpolator);

			envmaps[0]->GetTexture()->bind( texture_num);
			glUniform1i(s->m_uniform_locations[GLB::uniforms::envmap0], texture_num++);

			envmaps[1]->GetTexture()->bind( texture_num);
			glUniform1i(s->m_uniform_locations[GLB::uniforms::envmap1], texture_num++);

			OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);

#ifdef TEXTURE_COUNTING
            m_textureCounter.insert( ((GLBTexture*)envmaps[0]->GetTexture())->textureObject() );
			m_textureCounter.insert( ((GLBTexture*)envmaps[1]->GetTexture())->textureObject() );
#endif
		}

		if (s->m_uniform_locations[GLB::uniforms::light_pos] > -1 && light)
		{
			glUniform3fv(s->m_uniform_locations[GLB::uniforms::light_pos], 1, &light->m_world_pom.v[12]);
		}

		if (s->m_uniform_locations[GLB::uniforms::shadow_matrix0] > -1 && m_global_shadowmaps[0])
		{
			glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::shadow_matrix0], 1, GL_FALSE, m_global_shadowmaps[0]->m_matrix.v);

			OpenGLStateManager::GlActiveTexture(GL_TEXTURE0 + texture_num);
			glBindTexture( GL_TEXTURE_2D, m_global_shadowmaps[0]->GetTextureId() );
            glBindSampler(texture_num, 0);
			glUniform1i(s->m_uniform_locations[GLB::uniforms::shadow_unit0], texture_num++);

			OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);

#ifdef TEXTURE_COUNTING
			m_textureCounter.insert( m_global_shadowmaps[0]->GetTextureId() );
#endif
		}

		if (s->m_uniform_locations[GLB::uniforms::shadow_matrix1] > -1 && m_global_shadowmaps[1])
		{
			glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::shadow_matrix1], 1, GL_FALSE, m_global_shadowmaps[1]->m_matrix.v);

			OpenGLStateManager::GlActiveTexture(GL_TEXTURE0 + texture_num);
			glBindTexture( GL_TEXTURE_2D, m_global_shadowmaps[1]->GetTextureId() );
            glBindSampler(texture_num, 0);
			glUniform1i(s->m_uniform_locations[GLB::uniforms::shadow_unit1], texture_num++);

			OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);

#ifdef TEXTURE_COUNTING
			m_textureCounter.insert( m_global_shadowmaps[1]->GetTextureId() );
#endif
		}

#ifndef USE_UBOs //frameconsts
		if (s->m_uniform_locations[GLB::uniforms::inv_resolution] > -1)
		{
			glUniform2f(s->m_uniform_locations[GLB::uniforms::inv_resolution], 1.0f / m_viewport_width, 1.0f / m_viewport_height);
		}
#endif
		if (s->m_uniform_locations[GLB::uniforms::color] > -1)
		{
			glUniform3fv(s->m_uniform_locations[GLB::uniforms::color], 1, sm->m_color.v);
		}

		if (s->m_uniform_locations[GLB::uniforms::translate_uv] > -1)
		{
			if( sm->m_material->m_frame_when_animated != m_frame)
			{
                //if the test has looped, reset animation offsets
                if(sm->m_material->m_animation_time > m_animation_time / 1000.0f)
                {
                    sm->m_material->m_animation_time_base = 0.0f;
                }

				sm->m_material->m_animation_time = m_animation_time / 1000.0f;

				if( sm->m_material->m_translate_u_track)
				{
					Vector4D r;

					_key_node::Get( r, sm->m_material->m_translate_u_track, sm->m_material->m_animation_time, sm->m_material->m_animation_time_base, true);

					sm->m_material->m_uv_offset.x = -r.x;
				}
				if( sm->m_material->m_translate_v_track)
				{
					Vector4D r;

					_key_node::Get( r, sm->m_material->m_translate_v_track, sm->m_material->m_animation_time, sm->m_material->m_animation_time_base, true);

					sm->m_material->m_uv_offset.y = -r.x;
				}
				sm->m_material->m_frame_when_animated = m_frame;
			}

			glUniform2fv(s->m_uniform_locations[GLB::uniforms::translate_uv], 1, sm->m_material->m_uv_offset.v);
		}

		if (s->m_uniform_locations[GLB::uniforms::particle_data] > -1)
		{
			glUniform4fv(s->m_uniform_locations[GLB::uniforms::particle_data], MAX_PARTICLE_PER_MESH, m_particle_data[sm->m_offset1].v);
		}
		if (s->m_uniform_locations[GLB::uniforms::particle_color] > -1)
		{
			glUniform4fv(s->m_uniform_locations[GLB::uniforms::particle_color], MAX_PARTICLE_PER_MESH, m_particle_color[sm->m_offset1].v);
		}
		if (s->m_uniform_locations[GLB::uniforms::alpha_threshold] > -1)
		{
			glUniform1f(s->m_uniform_locations[GLB::uniforms::alpha_threshold], sm->m_alpha);
		}

		if (s->m_uniform_locations[GLB::uniforms::light_color] > -1 && light)
		{
			glUniform3fv(s->m_uniform_locations[GLB::uniforms::light_color], 1, light->m_diffuse_color.v);
		}
		if (s->m_uniform_locations[GLB::uniforms::world_fit_matrix] > -1)
		{
			glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::world_fit_matrix], 1, GL_FALSE, m_world_fit_matrix.v);
		}

		if (override_material != 0 && override_material->m_planar_map != 0 && s->m_uniform_locations[GLB::uniforms::planar_reflection] > -1)
		{
			OpenGLStateManager::GlActiveTexture(GL_TEXTURE0 + texture_num);
			glBindTexture( GL_TEXTURE_2D, override_material->m_planar_map->GetTextureId() );
            glBindSampler(texture_num, 0);
			glUniform1i(s->m_uniform_locations[GLB::uniforms::planar_reflection], texture_num++);

			OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);

#ifdef TEXTURE_COUNTING
			m_textureCounter.insert( override_material->m_planar_map->GetTextureId() );
#endif
		}


		if (s->m_uniform_locations[GLB::uniforms::mvp2] > -1)
		{
			switch( mesh_type)
			{
			case 0:
				{
					KCL::Matrix4x4 mvp2;

					mvp2 = sm->m_prev_world_pom * m_prev_vp;

					glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::mvp2], 1, GL_FALSE, mvp2.v);

					break;
				}
			case 1:
				{
				glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::mvp2], 1, GL_FALSE, m_prev_vp.v);
					break;
				}
			}
		}

		if (s->m_uniform_locations[GLB::uniforms::prev_bones] > -1 && sm->m_mesh->m_prev_node_matrices.size())
		{
			glUniform4fv(s->m_uniform_locations[GLB::uniforms::prev_bones], 3 * m_max_joint_num_per_mesh, sm->m_mesh->m_prev_node_matrices.data());
		}

		if (s->m_uniform_locations[GLB::uniforms::mblur_mask] > -1)
		{
			glUniform1f(s->m_uniform_locations[GLB::uniforms::mblur_mask], sm->m_is_motion_blurred);
		}


		if( !VAO_enabled)
		{
#ifdef USE_VBO
			VboPool::Instance()->BindBuffer(sm->m_mesh->m_vbo);
			IndexBufferPool::Instance()->BindBuffer(sm->m_mesh->m_ebo[lod].m_buffer);
#endif

			for( uint32 l=0; l<14; l++)
			{
				if( s->m_attrib_locations[l] > -1 && sm->m_mesh->m_vertex_attribs[l].m_size != 0)
				{
					OpenGLStateManager::GlEnableVertexAttribArrayInstantCommit( s->m_attrib_locations[l]);

					glVertexAttribPointer(
						s->m_attrib_locations[l],
						sm->m_mesh->m_vertex_attribs[l].m_size,
						sm->m_mesh->m_vertex_attribs[l].m_type,
						sm->m_mesh->m_vertex_attribs[l].m_normalized,
						sm->m_mesh->m_vertex_attribs[l].m_stride,
						sm->m_mesh->m_vertex_attribs[l].m_data
						);
				}
			}
		}
#ifdef OCCLUSION_QUERY_BASED_STAT
        if(do_not_skip_samples_passed || m_measurePixelCoverage)
			m_glGLSamplesPassedQuery->Begin();
#endif

        KCL::uint32 instanceCount = 0;

#ifdef MEASURE_PIXEL_PER_PRIMITIVE_COVERAGE
        if(m_measurePixelCoverage)
	    {
            //turn off depth testing - otherwise covered samples are not taken into account but covered polys are
            GLB::OpenGLStateManager::GlDisable(GL_DEPTH_TEST);
            GLB::OpenGLStateManager::GlDepthFunc(GL_ALWAYS);
        }
#endif

#ifdef USE_VBO
		//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
		if( krl_mesh->m_instances[sm->m_material].size() < 2)
		{
			GLB::OpenGLStateManager::Commit();

			if( VAO_enabled)
			{

			glBindVertexArray(m_vao_storage->get(sm->m_mesh,s)) ;

			}

			glDrawElements(GL_TRIANGLES, sm->m_primitive_count ? sm->m_primitive_count : KCL::uint32(sm->m_mesh->getIndexCount(lod)), GL_UNSIGNED_SHORT, sm->m_mesh->m_ebo[lod].m_offset);

			if( VAO_enabled)
			{
			glBindVertexArray( 0);
			}

            instanceCount = 1;
		}
		else
		{
			krl_mesh->m_is_rendered.insert( sm->m_material);

			if( VAO_enabled)
			{
			glBindVertexArray(m_vao_storage->get(sm->m_mesh,s)) ;

			}
			else
			{
				VboPool::Instance()->BindBuffer( 0);
	            (static_cast<GLB::Mesh3*>(krl_mesh))->BindInstanceVBO();
			}

			for( int i=0; i<4; i++)
			{
				if( !VAO_enabled)
				{

					//GLB::OpenGLStateManager::GlEnableVertexAttribArray( s->m_attrib_locations[attribs::in_instance_mv0]+i); //instance pom
					//GLB::OpenGLStateManager::GlEnableVertexAttribArray( s->m_attrib_locations[attribs::in_instance_mv1]+i); //instance pom
					GLB::OpenGLStateManager::GlEnableVertexAttribArray(s->m_attrib_locations[attribs::in_instance_mv0 + i]); //instance pom
					GLB::OpenGLStateManager::GlEnableVertexAttribArray(s->m_attrib_locations[attribs::in_instance_inv_mv] + i); //instance pom

					//glVertexAttribPointer( s->m_attrib_locations[attribs::in_instance_mv0]+i, 4, GL_FLOAT, 0, sizeof( KRL::Mesh3::InstanceData), krl_mesh->m_instances[sm->m_material][0].mv.v + i*4);
					//glVertexAttribPointer( s->m_attrib_locations[attribs::in_instance_mv1]+i, 4, GL_FLOAT, 0, sizeof( KRL::Mesh3::InstanceData), krl_mesh->m_instances[sm->m_material][0].inv_mv.v + i*4);
					glVertexAttribPointer(s->m_attrib_locations[attribs::in_instance_mv0 + i], 4, GL_FLOAT, 0, sizeof(KRL::Mesh3::InstanceData), (const void*)(i * 4 * sizeof(float)));
					glVertexAttribPointer(s->m_attrib_locations[attribs::in_instance_inv_mv] + i, 4, GL_FLOAT, 0, sizeof(KRL::Mesh3::InstanceData), (const void*)(sizeof(KCL::Matrix4x4) + i * 4 * sizeof(float)));
				}

				//glVertexAttribDivisor( s->m_attrib_locations[attribs::in_instance_mv0]+i, 1);
				//glVertexAttribDivisor( s->m_attrib_locations[attribs::in_instance_mv1]+i, 1);
				glVertexAttribDivisor(s->m_attrib_locations[attribs::in_instance_mv0 + i], 1);
				glVertexAttribDivisor(s->m_attrib_locations[attribs::in_instance_inv_mv] + i, 1);
			}

			GLB::OpenGLStateManager::Commit();

			glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(sm->m_mesh->getIndexCount(lod)), GL_UNSIGNED_SHORT, sm->m_mesh->m_ebo[lod].m_offset, static_cast<GLsizei>(krl_mesh->m_instances[sm->m_material].size()));

			for( int i=0; i<4; i++)
			{
				if( !VAO_enabled)
				{
					//GLB::OpenGLStateManager::GlDisableVertexAttribArray( s->m_attrib_locations[attribs::in_instance_mv0]+i); //instance pom
					//GLB::OpenGLStateManager::GlDisableVertexAttribArray( s->m_attrib_locations[attribs::in_instance_mv1]+i); //instance pom
					GLB::OpenGLStateManager::GlDisableVertexAttribArray(s->m_attrib_locations[attribs::in_instance_mv0 + i]); //instance pom
					GLB::OpenGLStateManager::GlDisableVertexAttribArray(s->m_attrib_locations[attribs::in_instance_inv_mv] + i); //instance pom
				}
				//glVertexAttribDivisor( s->m_attrib_locations[attribs::in_instance_mv0]+i, 0);
				//glVertexAttribDivisor( s->m_attrib_locations[attribs::in_instance_mv1]+i, 0);
				glVertexAttribDivisor(s->m_attrib_locations[attribs::in_instance_mv0 + i], 0);
				glVertexAttribDivisor(s->m_attrib_locations[attribs::in_instance_inv_mv] + i, 0);
			}


			if( VAO_enabled)
			{
			glBindVertexArray( 0);
			}

            instanceCount = (uint32)krl_mesh->m_instances[sm->m_material].size();
		}
#endif
		//glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);

		m_num_draw_calls++;
		m_num_triangles += KCL::uint32(sm->m_mesh->getIndexCount(lod)) / 3 * instanceCount;
		m_num_vertices += (uint32)sm->m_mesh->getIndexCount(lod) * instanceCount;

#ifdef OCCLUSION_QUERY_BASED_STAT
		if(do_not_skip_samples_passed || m_measurePixelCoverage)
		{
			m_glGLSamplesPassedQuery->End();
            size_t sampleCount = m_glGLSamplesPassedQuery->Result();
            if(do_not_skip_samples_passed)
			{
                m_num_samples_passed += sampleCount;

#ifdef CALCULATE_SHADER_INSTRUCTION_COUNT
			    m_num_instruction += s->m_instruction_count_v * sm->m_mesh->getIndexCount(lod) * instanceCount + s->m_instruction_count_f * sampleCount;
#endif
            }

#ifdef MEASURE_PIXEL_PER_PRIMITIVE_COVERAGE
            if(m_measurePixelCoverage && sampleCount)
		    {
                KCL::uint32 primCount = sm->m_mesh->getIndexCount(lod) / 3 / 2 * instanceCount;

		        float vis = float(sampleCount) / float(primCount);
                fprintf( m_coverageFile, "%.0f;%s;%d;%d;%.1f\n", m_animation_time * m_animation_multiplier, sm->m_name.c_str(), primCount, sampleCount, vis);

                m_pixel_coverage_sampleCount += sampleCount;
                m_pixel_coverage_primCount += primCount;
		    }
#endif
		}


#ifdef MEASURE_PIXEL_PER_PRIMITIVE_COVERAGE
        if(m_measurePixelCoverage)
	    {
            //turn depth testing back on
            OpenGLStateManager::GlEnable(GL_DEPTH_TEST);
            OpenGLStateManager::GlDepthFunc(GL_LESS);
        }
#endif

#endif

		if( !VAO_enabled)
		{
			for( uint32 l=0; l<14; l++)
			{
				if( s->m_attrib_locations[l] > -1)
				{
					OpenGLStateManager::GlDisableVertexAttribArray( s->m_attrib_locations[l]);
				}
			}
		}

#ifdef PER_FRAME_ALU_INFO
		glEndQuery(GL_SAMPLES_PASSED);

		glGetQueryObjectuiv( Shader::QueryId(), GL_QUERY_RESULT, &samplesPassed);

		Shader::Add_FrameAluInfo(s, samplesPassed, sm->m_mesh->m_vertex_indices[lod].size() / 3);
#endif

	}

	if(last_material)
	{
		last_material->postInit();
	}

#ifdef USE_VBO
	VboPool::Instance()->BindBuffer(0);
	IndexBufferPool::Instance()->BindBuffer(0);
#endif
}

void GLB_Scene_ES3::RenderLight( KCL::Light *l)
{
	Matrix4x4 model;
	KCL::uint32 input_textures[] =
	{
		pp.m_color_map,
		pp.m_normal_map,
		pp.m_reflection_map,
		pp.m_depth_texture,
		pp.m_param_map
	};
	int light_shader_index = 3;
	int light_buffer_index = -1;

	if( l->m_light_type == KCL::Light::SSAO)
	{
		light_shader_index = 4;
	}
	else if( l->m_light_type == KCL::Light::DIRECTIONAL)
	{
		light_shader_index = 0;
	}
	else if( l->m_light_type == KCL::Light::OMNI)
	{
		light_shader_index = 1;
		light_buffer_index = 0;

		model.translate( Vector3D( l->m_world_pom.v[12], l->m_world_pom.v[13], l->m_world_pom.v[14]));
		model.scale( Vector3D( l->m_radius * 1.25f, l->m_radius * 1.25f, l->m_radius * 1.25f) );
	}
	else if( l->m_light_type == KCL::Light::SPOT)
	{
		light_shader_index = 2;
		light_buffer_index = 1;

		float halfSpotAngle = Math::Rad(0.5f * l->m_spotAngle);

        float scalingFactorX = KCL::Vector3D(l->m_world_pom.v[0], l->m_world_pom.v[1], l->m_world_pom.v[2]).length();
        float scalingFactorY = KCL::Vector3D(l->m_world_pom.v[4], l->m_world_pom.v[5], l->m_world_pom.v[6]).length();
        float scalingFactorZ = KCL::Vector3D(l->m_world_pom.v[8], l->m_world_pom.v[9], l->m_world_pom.v[10]).length();

		assert(fabs(scalingFactorX - scalingFactorY) < 0.001f);
        assert(fabs(scalingFactorY - scalingFactorZ) < 0.001f);
        assert(fabs(scalingFactorX - scalingFactorZ) < 0.001f);

		model.zero();
        model.v33 = l->m_radius * (1.0f / scalingFactorZ);
		model.v11 = model.v22 = model.v33 * tanf(halfSpotAngle) * 1.2f; //1.2 is: extra opening to counter low tess-factor of the cone
		model.v43 = -model.v33;	// Translate so the top is at the origo
		model.v44 = 1;
		model *= l->m_world_pom;
	}
	else if( l->m_light_type == KCL::Light::SHADOW_DECAL)
	{
		light_shader_index = 15;
		light_buffer_index = 0;

		model.translate( Vector3D( l->m_world_pom.v[12], l->m_world_pom.v[13], l->m_world_pom.v[14]));
		model.scale( Vector3D( l->m_radius * 1.25f, l->m_radius * 1.25f, l->m_radius * 1.25f) );
	}
	else
	{
		model.translate( Vector3D( l->m_world_pom.v[12], l->m_world_pom.v[13], l->m_world_pom.v[14]));
		model.scale( Vector3D( l->m_radius * 1.25f, l->m_radius * 1.25f, l->m_radius * 1.25f) );
	}

	if( light_buffer_index == -1)
	{
		return;
	}

	if( l->m_is_shadow_caster)
	{
		//light_shader_index += 10;
	}

	Shader *s = m_lighting_shaders[light_shader_index];

	glUseProgram( s->m_p);

	if( !VAO_enabled)
	{
		GLB::OpenGLStateManager::GlEnableVertexAttribArray(s->m_attrib_locations[attribs::in_position]);
	}

	int j = 5;
	while( j--)
	{
		if (s->m_uniform_locations[GLB::uniforms::texture_unit0 + j] > -1)
		{
			GLB::OpenGLStateManager::GlActiveTexture( GL_TEXTURE0 + j);
			glUniform1i(s->m_uniform_locations[GLB::uniforms::texture_unit0 + j], j);
			glBindTexture( GL_TEXTURE_2D, input_textures[j]);
            glBindSampler(j, 0);

#ifdef TEXTURE_COUNTING
			m_textureCounter.insert( input_textures[j] );
#endif
		}
	}

#ifdef USE_UBOs
	uMeshConsts.mvp = model * m_active_camera->GetViewProjection();
	uMeshConsts.mv.identity();
	uMeshConsts.model = model;
	uMeshConsts.inv_model.identity();
	uMeshConsts.inv_modelview.identity();
	glBindBuffer(GL_UNIFORM_BUFFER, m_UBO_ids[Shader::sUBOnames::Mesh]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(uMeshConsts), (const void*)(&uMeshConsts), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_UBO_ids[Shader::sUBOnames::Mesh]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
#else //meshconsts
	if (s->m_uniform_locations[GLB::uniforms::mvp] > -1)
	{
		glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::mvp], 1, GL_FALSE, model * m_active_camera->GetViewProjection());
	}

	if (s->m_uniform_locations[GLB::uniforms::mv] > -1)
	{
		glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::mv], 1, GL_FALSE, model * m_active_camera->GetView());
	}

	if (s->m_uniform_locations[GLB::uniforms::model] > -1)
	{
		glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::model], 1, GL_FALSE, model.v);
	}
#endif

#ifndef USE_UBOs //frameconsts
	if (s->m_uniform_locations[GLB::uniforms::view_dir] > -1)
	{
		Vector3D view_dir( -m_active_camera->GetView().v[2], -m_active_camera->GetView().v[6], -m_active_camera->GetView().v[10]);
		glUniform3fv(s->m_uniform_locations[GLB::uniforms::view_dir], 1, view_dir.v);
	}

	if (s->m_uniform_locations[GLB::uniforms::view_pos] > -1)
	{
		glUniform3fv(s->m_uniform_locations[GLB::uniforms::view_pos], 1, m_active_camera->GetEye().v);
	}

	if (s->m_uniform_locations[GLB::uniforms::inv_resolution] > -1)
	{
		glUniform2f(s->m_uniform_locations[GLB::uniforms::inv_resolution], 1.0f / m_viewport_width, 1.0f / m_viewport_height);
	}

	if (s->m_uniform_locations[GLB::uniforms::depth_parameters] > -1)
	{
		glUniform4fv(s->m_uniform_locations[GLB::uniforms::depth_parameters], 1, m_active_camera->m_depth_linearize_factors.v);
	}
#endif

#ifdef USE_UBOs
	float i = 1.0f;
	if( l->m_intensity_track)
	{
		Vector4D v;

		l->t = m_animation_time / 1000.0f;

		_key_node::Get( v, l->m_intensity_track, l->t, l->tb, true);

		//i = v.x / l->m_intensity;
		i = 0.01 * v.x;// / l->m_intensity;
	}
	uLightConsts.light_colorXYZ_pad = KCL::Vector4D(l->m_diffuse_color.x * i, l->m_diffuse_color.y * i, l->m_diffuse_color.z * i, 0.0f);

	uLightConsts.light_posXYZ_pad = KCL::Vector4D( l->m_world_pom.v[12], l->m_world_pom.v[13], l->m_world_pom.v[14], 0.0f);
	uLightConsts.light_x = KCL::Vector4D( -l->m_world_pom.v[8], -l->m_world_pom.v[9], -l->m_world_pom.v[10], l->m_world_pom[14]);

	Vector2D spot_sin;
	float fov = l->m_spotAngle;
	spot_sin.x = cosf( Math::Rad( fov / 2.0f));
	spot_sin.y = 1.0f / (1.0f - spot_sin.x);
	uLightConsts.spotcosXY_attenZ_pad = KCL::Vector4D(spot_sin.x, spot_sin.y, -1.0f / (l->m_radius * l->m_radius), 0.0f);

	glBindBuffer(GL_UNIFORM_BUFFER, m_UBO_ids[Shader::sUBOnames::Light]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(uLightConsts), (const void*)(&uLightConsts), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, m_UBO_ids[Shader::sUBOnames::Light]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
#else
	if (s->m_uniform_locations[GLB::uniforms::light_x] > -1)
	{
		Vector3D dir( -l->m_world_pom.v[8], -l->m_world_pom.v[9], -l->m_world_pom.v[10]);
		dir.normalize();
		glUniform4f(s->m_uniform_locations[GLB::uniforms::light_x],
			dir.x,
			dir.y,
			dir.z,
			l->m_world_pom[14]);
	}

	if (s->m_uniform_locations[GLB::uniforms::light_pos] > -1)
	{
		Vector3D light_pos( l->m_world_pom.v[12], l->m_world_pom.v[13], l->m_world_pom.v[14]);

		glUniform3fv(s->m_uniform_locations[GLB::uniforms::light_pos], 1, light_pos.v);
	}

	if (s->m_uniform_locations[GLB::uniforms::light_color] > -1)
	{
		float i = 1.0f;
		if( l->m_intensity_track)
		{
			Vector4D v;

			l->t = m_animation_time / 1000.0f;

			_key_node::Get( v, l->m_intensity_track, l->t, l->tb, true);

			//i = v.x / l->m_intensity;
			i = 0.01 * v.x;// / l->m_intensity;
		}

		glUniform3f(s->m_uniform_locations[GLB::uniforms::light_color],
			l->m_diffuse_color.x * i,
			l->m_diffuse_color.y * i,
			l->m_diffuse_color.z * i
			);
	}

	if (s->m_uniform_locations[GLB::uniforms::attenuation_parameter] > -1)
	{
		float r = l->m_radius;

		glUniform1f(s->m_uniform_locations[GLB::uniforms::attenuation_parameter], -1.0f / (r * r));
	}

	if (s->m_uniform_locations[GLB::uniforms::spot_cos] > -1)
	{
		Vector2D spot_sin;

		float fov = l->m_spotAngle;

		spot_sin.x = cosf( Math::Rad( fov / 2.0f));
		spot_sin.y = 1.0f / (1.0f - spot_sin.x);

		glUniform2fv(s->m_uniform_locations[GLB::uniforms::spot_cos], 1, spot_sin.v);
	}
#endif

	if (s->m_uniform_locations[GLB::uniforms::shadow_matrix0] > -1 && m_global_shadowmaps[0])
	{
		glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::shadow_matrix0], 1, GL_FALSE, m_global_shadowmaps[0]->m_matrix.v);

		GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0 + 7);
		glBindTexture( GL_TEXTURE_2D, m_global_shadowmaps[0]->GetTextureId() );
        glBindSampler(7, 0);
		glUniform1i(s->m_uniform_locations[GLB::uniforms::shadow_unit0], 7);
#ifdef TEXTURE_COUNTING
		m_textureCounter.insert( m_global_shadowmaps[0]->GetTextureId() );
#endif

		GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
	}

	if( !VAO_enabled)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_lbos[light_buffer_index].m_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_lbos[light_buffer_index].m_ebo);

		glVertexAttribPointer(s->m_attrib_locations[attribs::in_position], 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	GLB::OpenGLStateManager::Commit();
#ifdef OCCLUSION_QUERY_BASED_STAT
	m_glGLSamplesPassedQuery->Begin();
#endif

	if( VAO_enabled)
	{
	glBindVertexArray( m_lbos[light_buffer_index].m_vao);
	}

	glDrawElements( GL_TRIANGLES, m_lbos[light_buffer_index].m_num_indices, GL_UNSIGNED_SHORT, 0);

	if( VAO_enabled)
	{
	glBindVertexArray( 0);
	}

	m_num_draw_calls++;
	m_num_triangles += m_lbos[light_buffer_index].m_num_indices / 3;
	m_num_vertices += m_lbos[light_buffer_index].m_num_indices;


#ifdef OCCLUSION_QUERY_BASED_STAT
	m_glGLSamplesPassedQuery->End();
	m_num_samples_passed += m_glGLSamplesPassedQuery->Result();

#ifdef CALCULATE_SHADER_INSTRUCTION_COUNT
	m_num_instruction += s->m_instruction_count_v * m_lbos[light_buffer_index].m_num_indices + s->m_instruction_count_f * m_glGLSamplesPassedQuery->Result();
#endif

#endif
	if( !VAO_enabled)
	{
		GLB::OpenGLStateManager::GlDisableVertexAttribArray(s->m_attrib_locations[attribs::in_position]);
	}
}

void GLB_Scene_ES3::MoveParticles()
{
    GLB::OpenGLStateManager::Reset();
	//GLB::OpenGLStateManager::DisableAllVertexAttribsInstantCommit();
	//bool done = false;

	KCL::uint32 advectVAO, renderBuffer, renderVAO;

	advectVAO = 0 ;
	renderBuffer = 0 ;
	renderVAO = 0 ;

	for( KCL::uint32 i=0; i<m_actors.size(); i++)
	{
		Actor *actor = m_actors[i];

		for( KCL::uint32 i=0; i<actor->m_emitters.size(); i++)
		{
			GLB::TF_emitter *emitter = static_cast<GLB::TF_emitter*>(actor->m_emitters[i]);

			KCL::Vector4D emitter_apertureXYZ_focusdist;
			KCL::Matrix4x4 emitter_worldmat;
			KCL::Vector4D emitter_min_freqXYZ_speed;
			KCL::Vector4D emitter_max_freqXYZ_speed;
			KCL::Vector4D emitter_min_ampXYZ_accel;
			KCL::Vector4D emitter_max_ampXYZ_accel;
			KCL::Vector4D emitter_color;
			KCL::Vector4D emitter_externalVel_gravityFactor;
			KCL::Vector3D emitter_maxlife_sizeXY;
			emitter->GetEmitterParams(	emitter_apertureXYZ_focusdist,
										emitter_worldmat,
										emitter_min_freqXYZ_speed,
										emitter_max_freqXYZ_speed,
										emitter_min_ampXYZ_accel,
										emitter_max_ampXYZ_accel,
										emitter_color,
										emitter_externalVel_gravityFactor,
										emitter_maxlife_sizeXY);

			//Advect + birth particles in VS
			glEnable(GL_RASTERIZER_DISCARD);
			{
				Shader* s = m_particleAdvect_shader;

				// Set up the advection shader:
				GLB::OpenGLStateManager::GlUseProgram(s->m_p);
				Shader::m_last_shader = s;
				//glUseProgram(s->m_p);

				//noise map for per instance rnd emission values
                m_particle_noise->bind(0);
				#ifdef TEXTURE_COUNTING
					m_textureCounter.insert( ((GLBTexture*)m_particle_noise)->textureObject() );
				#endif
					glUniform1i(s->m_uniform_locations[GLB::uniforms::texture_unit0], 0);


					if (s->m_uniform_locations[GLB::uniforms::emitter_apertureXYZ_focusdist] > -1)
				{
						glUniform4fv(s->m_uniform_locations[GLB::uniforms::emitter_apertureXYZ_focusdist], 1, emitter_apertureXYZ_focusdist.v);
					}
					if (s->m_uniform_locations[GLB::uniforms::emitter_worldmat] > -1)
					{
						glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::emitter_worldmat], 1, GL_FALSE, emitter_worldmat.v);
					}
					if (s->m_uniform_locations[GLB::uniforms::emitter_min_freqXYZ_speed] > -1)
					{
						glUniform4fv(s->m_uniform_locations[GLB::uniforms::emitter_min_freqXYZ_speed], 1, emitter_min_freqXYZ_speed.v);
					}
					if (s->m_uniform_locations[GLB::uniforms::emitter_max_freqXYZ_speed] > -1)
					{
						glUniform4fv(s->m_uniform_locations[GLB::uniforms::emitter_max_freqXYZ_speed], 1, emitter_max_freqXYZ_speed.v);
					}
					if (s->m_uniform_locations[GLB::uniforms::emitter_min_ampXYZ_accel] > -1)
					{
						glUniform4fv(s->m_uniform_locations[GLB::uniforms::emitter_min_ampXYZ_accel], 1, emitter_min_ampXYZ_accel.v);
					}
					if (s->m_uniform_locations[GLB::uniforms::emitter_max_ampXYZ_accel] > -1)
					{
						glUniform4fv(s->m_uniform_locations[GLB::uniforms::emitter_max_ampXYZ_accel], 1, emitter_max_ampXYZ_accel.v);
					}
					if (s->m_uniform_locations[GLB::uniforms::emitter_externalVel_gravityFactor] > -1)
					{
						glUniform4fv(s->m_uniform_locations[GLB::uniforms::emitter_externalVel_gravityFactor], 1, emitter_externalVel_gravityFactor.v);
					}
					if (s->m_uniform_locations[GLB::uniforms::emitter_maxlifeX_sizeYZ_pad] > -1)
					{
						glUniform4fv(s->m_uniform_locations[GLB::uniforms::emitter_maxlifeX_sizeYZ_pad], 1, KCL::Vector4D(emitter_maxlife_sizeXY.x, emitter_maxlife_sizeXY.y, emitter_maxlife_sizeXY.z, 0.0f));
					}


				//get buffers - simulatesubstep might decide to skip advection, but we still need to render
				emitter->GetBuffers(advectVAO, renderBuffer, renderVAO);

				const unsigned int numSubsteps = emitter->GetNumSubsteps();

                KCL::int32 subStepData[ TF_emitter::max_num_substeps * 4 ];


				for (int i = 0 ; i < TF_emitter::max_num_substeps * 4 ; i++)
				{
					subStepData[i] = 0 ;
				}

				for( unsigned int substep = 0; substep < numSubsteps; substep++ )
				{
					emitter->SimulateSubStep();

					float dummy;
					emitter->GetBufferData( subStepData[ substep * 4 ], subStepData[ substep * 4 + 1 ], subStepData[ substep * 4 + 2 ], dummy );
				}

				//advect all substeps in the shader in a single draw
				{
					emitter->SwitchBuffers();
					emitter->GetBuffers(advectVAO, renderBuffer, renderVAO);

					if (s->m_uniform_locations[GLB::uniforms::particleBufferParamsXYZ_pad] > -1)
					{
						glUniform4iv(s->m_uniform_locations[GLB::uniforms::particleBufferParamsXYZ_pad], numSubsteps, subStepData);
					}

					if (s->m_uniform_locations[GLB::uniforms::emitter_numSubsteps] > -1)
					{
						glUniform1i(s->m_uniform_locations[GLB::uniforms::emitter_numSubsteps], numSubsteps);
					}

					// Specify the source buffer:
					glBindVertexArray(advectVAO);

					// Specify the target buffer:
                    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, renderBuffer);
					glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, renderBuffer);

					// Perform GPU advection:
					glBeginTransformFeedback(GL_POINTS);
					GLB::OpenGLStateManager::Commit();
					glDrawArrays(GL_POINTS, 0, emitter->VisibleParticleCount());

                    m_num_draw_calls++;
	                m_num_vertices += numSubsteps * emitter->VisibleParticleCount();
					#ifdef CALCULATE_SHADER_INSTRUCTION_COUNT
						m_num_instruction += s->m_instruction_count_v * numSubsteps * emitter->GetMaxParticleCount();
					#endif

					glEndTransformFeedback();

					glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
					glBindVertexArray(0);
				}


			}
		}
	}

	glDisable(GL_RASTERIZER_DISCARD);
}


void GLB_Scene_ES3::RenderTFParticles()
{
	GLB::OpenGLStateManager::Reset();
	//GLB::OpenGLStateManager::DisableAllVertexAttribsInstantCommit();
	//bool done = false;

	KCL::uint32 advectVAO, renderBuffer, renderVAO;

	for( KCL::uint32 i=0; i<m_actors.size(); i++)
	{
		Actor *actor = m_actors[i];

		for( KCL::uint32 i=0; i<actor->m_emitters.size(); i++)
		{
			GLB::TF_emitter *emitter = static_cast<GLB::TF_emitter*>(actor->m_emitters[i]);

			KCL::Vector4D emitter_apertureXYZ_focusdist;
			KCL::Matrix4x4 emitter_worldmat;
			KCL::Vector4D emitter_min_freqXYZ_speed;
			KCL::Vector4D emitter_max_freqXYZ_speed;
			KCL::Vector4D emitter_min_ampXYZ_accel;
			KCL::Vector4D emitter_max_ampXYZ_accel;
			KCL::Vector4D emitter_color;
			KCL::Vector4D emitter_externalVel_gravityFactor;
			KCL::Vector3D emitter_maxlife_sizeXY;
			emitter->GetEmitterParams(	emitter_apertureXYZ_focusdist,
										emitter_worldmat,
										emitter_min_freqXYZ_speed,
										emitter_max_freqXYZ_speed,
										emitter_min_ampXYZ_accel,
										emitter_max_ampXYZ_accel,
										emitter_color,
										emitter_externalVel_gravityFactor,
										emitter_maxlife_sizeXY);

			glDisable(GL_RASTERIZER_DISCARD);
			glDisable( GL_CULL_FACE);

            //get buffers - simulatesubstep might decide to skip advection, but we still need to render
            emitter->GetBuffers(advectVAO, renderBuffer, renderVAO);

			//Render particles
			{
				KCL::uint32 texture_num = 0;
				GLB::Material *material = NULL;
				if( emitter->m_name.find( "smoke") != std::string::npos)
				{
					material = dynamic_cast<GLB::Material*>(m_instanced_smoke_material);
				}
				if( emitter->m_name.find( "fire") != std::string::npos)
				{
					material = dynamic_cast<GLB::Material*>(m_instanced_fire_material);
				}
				if( emitter->m_name.find( "spark") != std::string::npos)
				{
					material = dynamic_cast<GLB::Material*>(m_instanced_spark_material);
				}
				if( emitter->m_name.find( "soot") != std::string::npos)
				{
					material = dynamic_cast<GLB::Material*>(m_instanced_spark_material);
				}
				else
				{
					material = dynamic_cast<GLB::Material*>(m_instanced_fire_material);
				}

				Shader* s = material->m_shaders[0][0];

				GLB::OpenGLStateManager::GlUseProgram(s->m_p);
				Shader::m_last_shader = s;

				material->preInit( texture_num, 0, 0);



				if (s->m_uniform_locations[GLB::uniforms::texture_3D_unit0] > -1)
				{
					//if( emitter->m_emitter_type == 1)
                    m_fire_texid->bind(texture_num);
#ifdef TEXTURE_COUNTING
						m_textureCounter.insert( ((GLBTexture*)m_fire_texid)->textureObject());
#endif

					//if( emitter->m_emitter_type == 2)
					//{
						//glBindTexture( GL_TEXTURE_3D, smoke_texid);
						//#ifdef TEXTURE_COUNTING
						//	m_textureCounter.insert( smoke_texid );
						//#endif
					//}
						glUniform1i(s->m_uniform_locations[GLB::uniforms::texture_3D_unit0], texture_num++);
				}

				if (s->m_uniform_locations[GLB::uniforms::texture_unit0] > -1)
				{
					glUniform1i(s->m_uniform_locations[GLB::uniforms::texture_unit0], 0);
				}
				if (s->m_uniform_locations[GLB::uniforms::emitter_maxlifeX_sizeYZ_pad] > -1)
				{
					glUniform4fv(s->m_uniform_locations[GLB::uniforms::emitter_maxlifeX_sizeYZ_pad], 1, KCL::Vector4D(emitter_maxlife_sizeXY.x, emitter_maxlife_sizeXY.y, emitter_maxlife_sizeXY.z, 0.0f));
				}
				if (s->m_uniform_locations[GLB::uniforms::color] > -1)
				{
					glUniform3fv(s->m_uniform_locations[GLB::uniforms::color], 1, emitter->GetColor().v);
				}
				if (s->m_uniform_locations[GLB::uniforms::mvp] > -1)
				{
					glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::mvp], 1, GL_FALSE, m_active_camera->GetViewProjection().v);
				}
				if (s->m_uniform_locations[GLB::uniforms::mv] > -1)
				{
					glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::mv], 1, GL_FALSE, m_active_camera->GetView().v);
				}

				glBindVertexArray(renderVAO);

#ifdef OCCLUSION_QUERY_BASED_STAT
				m_glGLSamplesPassedQuery->Begin();
#endif
				glVertexAttribDivisor(0, 1);
				glVertexAttribDivisor(1, 1);
				glVertexAttribDivisor(2, 1);
				glVertexAttribDivisor(3, 1);
				glVertexAttribDivisor(4, 1);
				glVertexAttribDivisor(5, 1);
				glVertexAttribDivisor(6, 1);
				glVertexAttribDivisor(7, 1);
				glVertexAttribDivisor(8, 1);

				GLB::OpenGLStateManager::Commit();
				glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, emitter->VisibleParticleCount() );
				m_num_draw_calls++;
				m_num_triangles += 2 * (emitter->VisibleParticleCount());
				m_num_vertices += 6 * (emitter->VisibleParticleCount());

				glVertexAttribDivisor(8, 0);
				glVertexAttribDivisor(7, 0);
				glVertexAttribDivisor(6, 0);
				glVertexAttribDivisor(5, 0);
				glVertexAttribDivisor(4, 0);
				glVertexAttribDivisor(3, 0);
				glVertexAttribDivisor(2, 0);
				glVertexAttribDivisor(1, 0);
				glVertexAttribDivisor(0, 0);

#ifdef OCCLUSION_QUERY_BASED_STAT
				m_glGLSamplesPassedQuery->End();
				m_num_samples_passed += m_glGLSamplesPassedQuery->Result();

#ifdef CALCULATE_SHADER_INSTRUCTION_COUNT
				m_num_instruction += s->m_instruction_count_v * (6 * emitter->VisibleParticleCount()) + s->m_instruction_count_f * m_glGLSamplesPassedQuery->Result();
#endif

#endif
				glBindVertexArray(0);
				material->postInit();
			}
		}
	}
}

void GLB_Scene_ES3::RenderLightshafts()
{
	//Add Camfogshaft
	//Light* l = new Light( "CameraFogShaft", 0, 0);
	//l->m_light_type = Light::SPOT;
	//l->m_diffuse_color = Vector3D(1.0f, 1.0f, 1.0f);
	//l->m_intensity = 1.0f;
	//l->m_radius = 150.0f;
	//l->m_spotAngle = m_active_camera->GetFov();
	//KCL::Matrix4x4::Invert4x4(m_active_camera->GetView(), l->m_world_pom);
	//l->m_light_projection.identity();
	//l->m_light_projection.perspective( l->m_spotAngle, m_active_camera->GetAspectRatio(), 0.01f, l->m_radius);
	//KCL::Matrix4x4::Invert4x4( l->m_light_projection, l->m_inv_light_projection);
	//KCL::Matrix4x4 m2 = l->m_inv_light_projection * l->m_world_pom;
	//for( KCL::uint32 i=0; i<8; i++)
	//{
	//	mult4x4( m2, KCL::Vector3D( m_box_vertices[i].v), l->m_frustum_vertices[i]);
	//}

	//m_lightshafts.push_back(l);
	LightShaft ls;

	for( uint32 j=0; j<m_lightshafts.size(); j++)
	{
		KCL::Light *l = m_lightshafts[j];

		for( uint32 i=0; i<8; i++)
		{
			ls.m_corners[i].set( l->m_frustum_vertices[i].v);
		}

		bool isCamShaft = false; //(j == m_lightshafts.size() - 1);

		//ls.CreateSlices( m_active_camera->GetCullPlane( KCL::CULLPLANE_NEAR), isCamShaft);
		KCL::Matrix4x4 m2 = l->m_inv_light_projection * l->m_world_pom;
		ls.CreateCone( m2, m_active_camera->GetCullPlane( KCL::CULLPLANE_NEAR), isCamShaft);
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_lightshaft_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_lightshaft_ebo);

	if( ls.m_vertices.size() && ls.m_indices.size())
	{
        glBufferData( GL_ARRAY_BUFFER, ls.m_vertices.size() * sizeof(KCL::Vector3D), ls.m_vertices[0].v, GL_DYNAMIC_DRAW);
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, ls.m_indices.size() * sizeof(uint16), &ls.m_indices[0], GL_DYNAMIC_DRAW);
	}

	GLB::OpenGLStateManager::GlEnable( GL_CULL_FACE);
	GLB::OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
	GLB::OpenGLStateManager::GlDepthMask( 0);
	GLB::OpenGLStateManager::GlEnable( GL_BLEND);
	GLB::OpenGLStateManager::GlBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if( !VAO_enabled)
	{
		GLB::OpenGLStateManager::GlEnableVertexAttribArray( 0);
		glVertexAttribPointer( 0, 3, GL_FLOAT, 0, sizeof(KCL::Vector3D), 0);
	}

	for( uint32 j=0; j<m_lightshafts.size(); j++)
	{
		KCL::Light *l = m_lightshafts[j];

		float normalized_time = (float) (m_animation_time % 100000) / 100000.0f;

		bool isCamShaft = false; //(j == m_lightshafts.size() - 1);

		if( !ls.m_num_indices[j])
		{
			continue;
		}

		Shader *s;
		if(!isCamShaft)
		{
			s = m_fog_shader;

			glUseProgram( s->m_p);

			KCL::uint32 textureNum = 1;
			if (s->m_uniform_locations[GLB::uniforms::texture_unit0+ 1] > -1)
			{
				GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0 + textureNum);
				glUniform1i(s->m_uniform_locations[GLB::uniforms::texture_unit0 + 1], textureNum);
				glBindTexture( GL_TEXTURE_2D, pp.m_depth_texture);
                glBindSampler(textureNum, 0);
				#ifdef TEXTURE_COUNTING
					m_textureCounter.insert( pp.m_depth_texture );
				#endif
				++textureNum;
			}
		}
		else //if camera shaft
		{
			s = m_camera_fog_shader;

			glUseProgram( s->m_p);

			KCL::uint32 textureNum = 0;
			if (s->m_uniform_locations[GLB::uniforms::texture_unit0] > -1)
			{
				GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0 + textureNum);
				glUniform1i(s->m_uniform_locations[GLB::uniforms::texture_unit0], textureNum);
				glBindTexture( GL_TEXTURE_2D, pp.m_depth_texture);
                glBindSampler(textureNum, 0);
				#ifdef TEXTURE_COUNTING
					m_textureCounter.insert( pp.m_depth_texture );
				#endif
				++textureNum;
			}
            /*
			if(s->m_uniform_locations[GLB::uniforms::texture_unit0 + 1] > -1)
			{
				GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0 + textureNum);
				glUniform1i( s->m_uniform_locations[GLB::uniforms::texture_unit0 + 1], textureNum);
				glBindTexture( GL_TEXTURE_2D, m_VolumeLightTex);
				#ifdef TEXTURE_COUNTING
					m_textureCounter.insert( m_VolumeLightTex );
				#endif
				++textureNum;
			}
            */
			if (s->m_uniform_locations[GLB::uniforms::texture_3D_unit0] > -1)
			{
				glUniform1i(s->m_uniform_locations[GLB::uniforms::texture_3D_unit0], textureNum);
                m_fog_texid->bind(textureNum);
				#ifdef TEXTURE_COUNTING
					m_textureCounter.insert( ((GLBTexture*)m_fog_texid)->textureObject() );
				#endif
				++textureNum;
			}
		}

		glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::mvp], 1, GL_FALSE, m_active_camera->GetViewProjection().v);
		glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::mv], 1, GL_FALSE, m_active_camera->GetView().v);

		if( s->m_uniform_locations[GLB::uniforms::view_dir] > -1)
		{
			Vector3D view_dir( -m_active_camera->GetView().v[2], -m_active_camera->GetView().v[6], -m_active_camera->GetView().v[10]);
			glUniform3fv(s->m_uniform_locations[GLB::uniforms::view_dir], 1, view_dir.v);
		}

		if (s->m_uniform_locations[GLB::uniforms::view_pos] > -1)
		{
			glUniform3fv(s->m_uniform_locations[GLB::uniforms::view_pos], 1, m_active_camera->GetEye().v);
		}
		if (s->m_uniform_locations[GLB::uniforms::inv_resolution] > -1) //for depth-fade
		{
			glUniform2f(s->m_uniform_locations[GLB::uniforms::inv_resolution], 1.0f / m_viewport_width, 1.0f / m_viewport_height);
		}
		if (s->m_uniform_locations[GLB::uniforms::depth_parameters] > -1) //for depth-fade
		{
			glUniform4fv(s->m_uniform_locations[GLB::uniforms::depth_parameters], 1, m_active_camera->m_depth_linearize_factors.v);
		}

		glUniform3f(s->m_uniform_locations[GLB::uniforms::background_color], 1.0, 0, 0);

		if ((s->m_uniform_locations[GLB::uniforms::shadow_matrix0] > -1) && (!isCamShaft))
		{
			static const Matrix4x4 shadowM (0.5f, 0, 0, 0,
				0, 0.5f, 0, 0,
				0, 0, 0.5f, 0,
				0.5f, 0.5f, 0.5f, 1);

			KCL::Matrix4x4 m0;

			KCL::Matrix4x4::Invert4x4( l->m_world_pom, m0);
			KCL::Matrix4x4 m;

			m = m0 * l->m_light_projection * shadowM;

			glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::shadow_matrix0], 1, GL_FALSE, m.v);

            m_light_noise->bind(0);
			glUniform1i(s->m_uniform_locations[GLB::uniforms::texture_unit0], 0);
#ifdef TEXTURE_COUNTING
			m_textureCounter.insert( ((GLBTexture*)m_light_noise)->textureObject() );
#endif

			GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
		}

		glUniform1f(s->m_uniform_locations[GLB::uniforms::time], normalized_time);

		if (s->m_uniform_locations[GLB::uniforms::light_x] > -1)
		{
			Vector3D dir( -l->m_world_pom.v[8], -l->m_world_pom.v[9], -l->m_world_pom.v[10]);
			dir.normalize();
			glUniform4f(s->m_uniform_locations[GLB::uniforms::light_x],
				dir.x,
				dir.y,
				dir.z,
				l->m_world_pom[14]);
		}

		if (s->m_uniform_locations[GLB::uniforms::light_pos] > -1)
		{
			Vector3D light_pos( l->m_world_pom.v[12], l->m_world_pom.v[13], l->m_world_pom.v[14]);

			glUniform3fv(s->m_uniform_locations[GLB::uniforms::light_pos], 1, light_pos.v);
		}

		if (s->m_uniform_locations[GLB::uniforms::light_color] > -1)
		{
			float i = 1.0f;
			if( l->m_intensity_track)
			{
				Vector4D v;

				l->t = m_animation_time / 1000.0f;

				_key_node::Get( v, l->m_intensity_track, l->t, l->tb, true);

				i = v.x / l->m_intensity;
			}

			glUniform3f(s->m_uniform_locations[GLB::uniforms::light_color],
				l->m_diffuse_color.x * i,
				l->m_diffuse_color.y * i,
				l->m_diffuse_color.z * i
				);
		}

		if (s->m_uniform_locations[GLB::uniforms::attenuation_parameter] > -1)
		{
			float r = l->m_radius;

			glUniform1f(s->m_uniform_locations[GLB::uniforms::attenuation_parameter], -1.0f / (r * r));
		}

		if (s->m_uniform_locations[GLB::uniforms::spot_cos] > -1)
		{
			Vector2D spot_sin;

			float fov = l->m_spotAngle;

			spot_sin.x = cosf( Math::Rad( fov / 2.0f));
			spot_sin.y = 1.0f / (1.0f - spot_sin.x);

			glUniform2fv(s->m_uniform_locations[GLB::uniforms::spot_cos], 1, spot_sin.v);
		}

#ifdef TEXTURE_COUNTING
		m_textureCounter.insert( ((GLBTexture*)m_fog_texid)->textureObject() );
#endif

		GLB::OpenGLStateManager::Commit();
#ifdef OCCLUSION_QUERY_BASED_STAT
		m_glGLSamplesPassedQuery->Begin();
#endif
		if( VAO_enabled)
		{
		glBindVertexArray( m_lightshaft_vao);
		}
		glDrawElements( GL_TRIANGLES, ls.m_num_indices[j], GL_UNSIGNED_SHORT, (void*) (ls.m_index_offsets[j] * sizeof( uint16)));

		if( VAO_enabled)
		{
		glBindVertexArray( 0);
		}

		m_num_draw_calls++;
		m_num_triangles += ls.m_num_indices[j] / 3;
		m_num_vertices += ls.m_num_indices[j];

#ifdef OCCLUSION_QUERY_BASED_STAT
		m_glGLSamplesPassedQuery->End();
		m_num_samples_passed += m_glGLSamplesPassedQuery->Result();

#ifdef CALCULATE_SHADER_INSTRUCTION_COUNT
		m_num_instruction += s->m_instruction_count_v * ls.m_num_indices[j] + s->m_instruction_count_f * m_glGLSamplesPassedQuery->Result();
#endif

#endif
	}
}

void GLB_Scene_ES3::QueryLensFlare()
{
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // DRIVER WORKAROUND STARTS HERE: Avoids crash related to transform feedback on some devices
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    glFlush();
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // DRIVER WORKAROUND ENDS HERE
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GLenum c[] =
	{
		GL_NONE,
		GL_NONE,
		GL_NONE,
		GL_NONE
	};
	glDrawBuffers(4, c);

#ifdef __glew_h__
	//GLB::OpenGLStateManager::GlEnable( GL_PROGRAM_POINT_SIZE);
	glEnable( GL_PROGRAM_POINT_SIZE);
#endif

	glColorMask( 0,0,0,0);
	GLB::OpenGLStateManager::GlDepthMask( 0);
	GLB::OpenGLStateManager::GlEnable( GL_DEPTH_TEST);

	Shader::m_last_shader = 0;
	GLB::OpenGLStateManager::GlUseProgram( m_occlusion_query_shader->m_p);

	glUniformMatrix4fv(m_occlusion_query_shader->m_uniform_locations[GLB::uniforms::vp], 1, GL_FALSE, m_active_camera->GetViewProjection().v);

	if( !VAO_enabled)
	{
		GLB::OpenGLStateManager::GlEnableVertexAttribArray(m_occlusion_query_shader->m_attrib_locations[attribs::in_position]);
	}

	for( KCL::uint32 i=0; i<m_visible_lights.size(); i++)
	{
		GLB::Light *l = static_cast<GLB::Light*>(m_visible_lights[i]);

		if( !l->m_has_lensflare)
		{
			continue;
		}

		if( l->m_light_type == KCL::Light::AMBIENT || l->m_light_type == KCL::Light::DIRECTIONAL || l->m_diffuse_color == Vector3D() )
		{
			continue;
		}


		l->NextQueryObject();

		glBeginQuery( GL_ANY_SAMPLES_PASSED, l->GetCurrentQueryObject());

		glBindBuffer( GL_ARRAY_BUFFER, m_occlusion_query_vbo);
		glBufferData( GL_ARRAY_BUFFER, 1 * sizeof(KCL::Vector3D), &l->m_world_pom.v[12], GL_DYNAMIC_DRAW);
		glBindBuffer( GL_ARRAY_BUFFER, 0);


		GLB::OpenGLStateManager::Commit();

		if( VAO_enabled)
		{
		glBindVertexArray( m_occlusion_query_vao);
		}
		else
		{
			glBindBuffer( GL_ARRAY_BUFFER, m_occlusion_query_vbo);
			glVertexAttribPointer(m_occlusion_query_shader->m_attrib_locations[attribs::in_position], 3, GL_FLOAT, 0, sizeof(KCL::Vector3D), 0);
		}

		glDrawArrays( GL_POINTS, 0, 1);

		if( VAO_enabled)
		{
		glBindVertexArray( 0);
		}
		else
		{
			glBindBuffer( GL_ARRAY_BUFFER, 0);
		}

        m_num_draw_calls++;
	    m_num_vertices += 1;
		#ifdef CALCULATE_SHADER_INSTRUCTION_COUNT
			m_num_instruction += m_occlusion_query_shader->m_instruction_count_v * 1;
		#endif

		glEndQuery( GL_ANY_SAMPLES_PASSED);
	}

	if( !VAO_enabled)
	{
		GLB::OpenGLStateManager::GlDisableVertexAttribArray(m_occlusion_query_shader->m_attrib_locations[attribs::in_position]);
	}

	GLB::OpenGLStateManager::GlDepthMask( 1);
	glColorMask( 1,1,1,1);
	GLB::OpenGLStateManager::GlDisable( GL_DEPTH_TEST);
    glDrawBuffers(1, b);
}

void GLB_Scene_ES3::Render()
{
	KCL::Vector3D fullscreen_vertices[4];

#ifdef LOG_SHADERS

#ifdef PER_FRAME_ALU_INFO
	Shader::Push_New_FrameAluInfo();
#endif

#endif

	std::vector<KCL::MeshInstanceOwner*>::iterator mioi = m_mios.begin();

	while( mioi != m_mios.end())
	{
		KCL::MeshInstanceOwner *mio = *mioi;

		if( mio->IsNeedUpdate() )
		{
			for( KCL::uint32 j = 0; j < 2; j++)
			{
				IndexBufferPool::Instance()->SubData(
					mio->m_current_vertex_indices[j].size() * 2,
					&mio->m_current_vertex_indices[j][0],
					mio->m_mesh->m_mesh->m_ebo[j].m_buffer,
					(intptr_t)mio->m_mesh->m_mesh->m_ebo[j].m_offset
					);
			}
		}

		mioi++;
	}


	m_num_draw_calls = 0;
	m_num_triangles = 0;
	m_num_vertices = 0;

    GLB::OpenGLStateManager::Reset();

#ifdef MEASURE_PIXEL_PER_PRIMITIVE_COVERAGE
    m_pixel_coverage_sampleCount = 0;
    m_pixel_coverage_primCount = 0;
#endif

#ifdef TEXTURE_COUNTING
	m_textureCounter.clear();
	Material::TextureCounter(m_textureCounter);
#endif

#ifdef OCCLUSION_QUERY_BASED_STAT
	m_num_samples_passed = 0;
#ifdef CALCULATE_SHADER_INSTRUCTION_COUNT
	m_num_instruction = 0;
#endif
#endif
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //              RENDER SHADOW MAPS
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
if(!(m_disabledRenderBits & RenderBits::ERB_ShadowDepthRender))
{
	for( KCL::uint32 i=0; i<m_num_shadow_maps; i++)
	{
		if( m_global_shadowmaps[i])
		{
            // 3.1: Do not render shadow if there is no visible shadow caster
            if( m_scene_version != KCL::SV_31 || !m_global_shadowmaps[i]->m_caster_meshes[0].empty() || !m_global_shadowmaps[i]->m_caster_meshes[1].empty())
            {
			    RenderShadow( m_global_shadowmaps[i]);
            }
		}
	}
}


	for( uint32 i=0; i<m_visible_planar_maps.size(); i++)
	{
#ifndef DUMMY_RENDER_FOR_PLANAR_FLUSH
		RenderPlanar( m_visible_planar_maps[i]);
#else
		if(m_visible_planar_maps.size() != i+1)
		{
			RenderPlanar( m_visible_planar_maps[i], m_visible_planar_maps[i+1]);
		}
		else
		{
			RenderPlanar( m_visible_planar_maps[i], 0);
		}
#endif
	}

	pp.BindGBuffer();

	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef OCCLUSION_QUERY_BASED_STAT
	//do_not_skip_samples_passed = true;
#endif

#ifdef MEASURE_PIXEL_PER_PRIMITIVE_COVERAGE
	m_measurePixelCoverage = true;
#endif

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //              RENDER SOLIDS TO G-BUFFER
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
if(!(m_disabledRenderBits & RenderBits::ERB_GBufferSolids))
{
	Render(m_active_camera, m_visible_meshes[0], 0, 0, 0, 0, 0);
}

#ifdef MEASURE_PIXEL_PER_PRIMITIVE_COVERAGE
	m_measurePixelCoverage = false;
#endif

#ifdef OCCLUSION_QUERY_BASED_STAT
	//do_not_skip_samples_passed = false;
#endif

	for( uint32 i=0; i<m_visible_planar_maps.size(); i++)
	{
		dynamic_cast<GLB::Material*>(m_planarReflectionMaterial)->m_planar_map = dynamic_cast<GLB::PlanarMap*>(m_visible_planar_maps[i]);
		m_planarReflectionMaterial->m_transparency = m_visible_planar_maps[i]->m_receiver_meshes[0]->m_material->m_reflect_intensity;
		m_planarReflectionMaterial->m_textures[2] = m_visible_planar_maps[i]->m_receiver_meshes[0]->m_material->m_textures[2];
		m_planarReflectionMaterial->m_textures[3] = m_visible_planar_maps[i]->m_receiver_meshes[0]->m_material->m_textures[3];

		GLB_Scene_ES2::Render(m_active_camera, m_visible_planar_maps[i]->m_receiver_meshes, m_planarReflectionMaterial, 0, 0, 0);

		m_planarReflectionMaterial->m_textures[2] = 0;
		m_planarReflectionMaterial->m_textures[3] = 0;
	}

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //              ADVECT PARTICLES ON GPU
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
if(!(m_disabledRenderBits & RenderBits::ERB_Particles))
{
    MoveParticles();
}

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //              COMPUTE LIGHTNING (GFXBENCH 3.1 ONLY)
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    RunEffect(LightningEffect);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //              ISSUE LENSFLARE QUERIES
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
if(!(m_disabledRenderBits & RenderBits::ERB_LensFlareQuery))
{
	if( occlusion_query_enable)
	{
		QueryLensFlare();
	}
}

    m_active_camera->CalculateFullscreenBillboard( fullscreen_vertices);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //              RUN LIGHTING PASS
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	DoLightingPass();

	Shader::m_last_shader = 0;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //              DRAW SKY
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
if(!(m_disabledRenderBits & RenderBits::ERB_Sky))
{
    GLB_Scene_ES2::Render(m_active_camera, m_sky_mesh, 0, 0, 0, 0);
}

	//re
	if( VAO_enabled)
	{
	glBindVertexArray( m_fullscreen_quad_vao);
	}
	else
	{
		glBindBuffer( GL_ARRAY_BUFFER, m_fullscreen_quad_vbo);
	}

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //              COMBINE LIT SCENE WITH REFLECTIONS AND EMISSIVE PARTS
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	filters[10]->m_shader = m_reflection_emission_shader;
	filters[10]->m_input_textures[0] = pp.m_color_map;
	filters[10]->m_input_textures[1] = pp.m_normal_map;
	filters[10]->m_input_textures[2] = pp.m_final_texture;
	filters[10]->m_input_textures[3] = pp.m_reflection_map;
	filters[10]->m_input_textures[4] = pp.m_depth_texture;

    if(m_disabledRenderBits & RenderBits::ERB_Post)
    {
        g_ForceScreenRender = true;
    }

#ifdef OCCLUSION_QUERY_BASED_STAT
	filters[10]->Render(m_num_draw_calls, m_num_triangles , m_num_vertices, m_textureCounter, m_num_samples_passed , m_num_instruction, m_glGLSamplesPassedQuery);
#else
	filters[10]->Render(m_num_draw_calls, m_num_triangles , m_num_vertices, m_textureCounter, m_num_samples_passed , m_num_instruction);
#endif

    if(m_disabledRenderBits & RenderBits::ERB_Post)
    {
        g_ForceScreenRender = false;

        glBindFramebuffer(GL_READ_FRAMEBUFFER, pp.m_fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, pp.m_viewport_width, pp.m_viewport_height, 0, 0, pp.m_viewport_width, pp.m_viewport_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    }

	if( VAO_enabled)
	{
	glBindVertexArray( 0);
	}
	else
	{
		glBindBuffer( GL_ARRAY_BUFFER, 0);
	}

	Shader::m_last_shader = 0;

	GLB::OpenGLStateManager::GlEnable( GL_CULL_FACE);
	GLB::OpenGLStateManager::GlDisable( GL_DEPTH_TEST);
	GLB::OpenGLStateManager::GlCullFace( GL_FRONT);
	GLB::OpenGLStateManager::GlEnable( GL_BLEND);
	GLB::OpenGLStateManager::GlBlendFunc( GL_DST_COLOR, 0);


    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //              BLEND SHADOW DECAL OVER
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
if(!(m_disabledRenderBits & RenderBits::ERB_ShadowDepthRender) && !(m_disabledRenderBits & RenderBits::ERB_ShadowDecal))
{
    if (m_skip_create_shadow_decal)
	{
		// RenderLight for 3.1
		RenderLight( NULL);
	}
	else
	{
		KCL::Light* ll = (KCL::Light*)m_factory.GetFactory(KCL::LIGHT)->Create( "", 0, 0);

		ll->m_light_type = KCL::Light::SHADOW_DECAL;
		ll->m_radius = 1000.0f;

		GLB_Scene_ES3::RenderLight( ll);

		delete ll;
	}
}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	GLB::OpenGLStateManager::GlDisable( GL_BLEND);
	GLB::OpenGLStateManager::GlDisable( GL_CULL_FACE);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //              BLEND DECALS OVER
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
if(!(m_disabledRenderBits & RenderBits::ERB_Decals))
{
    GLB_Scene_ES2::Render(m_active_camera, m_visible_meshes[2], 0, 0, 0, 0);
}

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //              RENDER LIGHTNING EFFECT (GFXBENCH 3.1 ONLY)
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
if(!(m_disabledRenderBits & RenderBits::ERB_Compute_Lightning))
{
    RenderEffect(LightningEffect);
}
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //              BLEND PARTICLES OVER
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
if(!(m_disabledRenderBits & RenderBits::ERB_Particles))
{
#if 1
	RenderTFParticles();
#endif

#if 0
	{
		for( KCL::uint32 i=0; i<m_actors.size(); i++)
		{
			Actor *actor = m_actors[i];

			for( KCL::uint32 i=0; i<actor->m_emitters.size(); i++)
			{
				KCL::_emitter *emitter = (KCL::_emitter *)actor->m_emitters[i];
				if(emitter->m_visibleparticle_count)
				{
					//TODO !!!
					//if(material == 0) continue; // TODO THIS IS HACK
					KCL::uint32 texture_num = 0;
					GLB::Mesh3 *mesh;
					GLB::Material *material;

					if( emitter->m_emitter_type2 == KCL::ET_BILLBOARD)
					{
						mesh = dynamic_cast<GLB::Mesh3*>(m_billboard_geometry);
					}
					else if( emitter->m_emitter_type2 == KCL::ET_SPARK)
					{
						mesh = dynamic_cast<GLB::Mesh3*>(m_spark_geometry);
					}

					if( emitter->m_name.find( "smoke") != std::string::npos)
					{
						material = dynamic_cast<GLB::Material*>(m_instanced_smoke_material);
					}
					if( emitter->m_name.find( "fire") != std::string::npos)
					{
						material = dynamic_cast<GLB::Material*>(m_instanced_fire_material);
					}
					if( emitter->m_name.find( "spark") != std::string::npos)
					{
						material = dynamic_cast<GLB::Material*>(m_instanced_spark_material);
					}
					if( emitter->m_name.find( "soot") != std::string::npos)
					{
						material = dynamic_cast<GLB::Material*>(m_instanced_spark_material);
					}
					else
					{
						material = dynamic_cast<GLB::Material*>(m_instanced_fire_material);
					}


					Shader *shader = material->m_shaders[0][0];

					material->preInit( texture_num, 0, 0);

					if( shader->m_uniform_locations[GLB::uniforms::texture_3D_unit0] > -1)
					{
						//if( emitter->m_emitter_type == 1)
                        fire_texid->bind(texture_num);
#ifdef TEXTURE_COUNTING
							m_textureCounter.insert( fire_texid->getTextureId());
#endif

						//if( emitter->m_emitter_type == 2)
						//{
							//glBindTexture( GL_TEXTURE_3D, smoke_texid);
							//#ifdef TEXTURE_COUNTING
							//	m_textureCounter.insert( smoke_texid );
							//#endif
						//}
							glUniform1i( shader->m_uniform_locations[GLB::uniforms::texture_3D_unit0] , texture_num++);
					}

					if (shader->m_uniform_locations[GLB::uniforms::color] > -1)
					{
						glUniform3fv(shader->m_uniform_locations[GLB::uniforms::color], 1, emitter->GetColor().v);
					}


					glUniformMatrix4fv(shader->m_uniform_locations[GLB::uniforms::mvp], 1, GL_FALSE, m_active_camera->GetViewProjection().v);
					glUniformMatrix4fv(shader->m_uniform_locations[GLB::uniforms::mv], 1, GL_FALSE, m_active_camera->GetView().v);


					VboPool::Instance()->BindBuffer( mesh->m_vbo);

					for( KCL::uint32 l=0; l<8; l++)
					{
						if( shader->m_attrib_locations[l] > -1)
						{
							GLB::OpenGLStateManager::GlEnableVertexAttribArray( shader->m_attrib_locations[l]);

							glVertexAttribPointer(
								shader->m_attrib_locations[l],
								mesh->m_vertex_attribs[l].m_size,
								mesh->m_vertex_attribs[l].m_type,
								mesh->m_vertex_attribs[l].m_normalized,
								mesh->m_vertex_attribs[l].m_stride,
								mesh->m_vertex_attribs[l].m_data
								);
						}
					}

					//TODO!!! m_particles_vbo is not in VboPool
					VboPool::Instance()->BindBuffer(0);
					glBindBuffer(GL_ARRAY_BUFFER, m_particles_vbo);
					glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(KCL::ParticleRenderAttrib2) * emitter->m_visibleparticle_count , (const void*)emitter->attribs );


					if(shader->m_attrib_locations[attribs::in_instance_position] > -1)
					{
						GLB::OpenGLStateManager::GlEnableVertexAttribArray( shader->m_attrib_locations[attribs::in_instance_position]); //instance position
						glVertexAttribPointer( shader->m_attrib_locations[attribs::in_instance_position], 3, GL_FLOAT, 0, sizeof(KCL::ParticleRenderAttrib2), (const GLvoid*)offsetof(KCL::ParticleRenderAttrib2, m_pos)  );
						glVertexAttribDivisor( shader->m_attrib_locations[attribs::in_instance_position], 1 );
					}
					if(shader->m_attrib_locations[attribs::in_instance_life] > -1)
					{
						GLB::OpenGLStateManager::GlEnableVertexAttribArray( shader->m_attrib_locations[attribs::in_instance_life]); //instance life
						glVertexAttribPointer( shader->m_attrib_locations[attribs::in_instance_life], 1, GL_FLOAT, 0, sizeof(KCL::ParticleRenderAttrib2), (const GLvoid*)offsetof(KCL::ParticleRenderAttrib2, m_life_normalized) );
						glVertexAttribDivisor( shader->m_attrib_locations[attribs::in_instance_life], 1 );
					}
					if(shader->m_attrib_locations[attribs::in_instance_speed] > -1)
					{
						GLB::OpenGLStateManager::GlEnableVertexAttribArray( shader->m_attrib_locations[attribs::in_instance_speed]); //instance speed
						glVertexAttribPointer( shader->m_attrib_locations[attribs::in_instance_speed], 3, GL_FLOAT, 0, sizeof(KCL::ParticleRenderAttrib2), (const GLvoid*)offsetof(KCL::ParticleRenderAttrib2, m_velocity) );
						glVertexAttribDivisor( shader->m_attrib_locations[attribs::in_instance_speed], 1 );
					}
					if(shader->m_attrib_locations[attribs::in_instance_size] > -1)
					{
						GLB::OpenGLStateManager::GlEnableVertexAttribArray( shader->m_attrib_locations[attribs::in_instance_size]); //instance size
						glVertexAttribPointer( shader->m_attrib_locations[attribs::in_instance_size], 1, GL_FLOAT, 0, sizeof(KCL::ParticleRenderAttrib2), (const GLvoid*)offsetof(KCL::ParticleRenderAttrib2, m_size) );
						glVertexAttribDivisor( shader->m_attrib_locations[attribs::in_instance_size], 1 );
					}


					IndexBufferPool::Instance()->BindBuffer( mesh->m_ebo[0].m_buffer);

					GLB::OpenGLStateManager::Commit();

#ifdef OCCLUSION_QUERY_BASED_STAT
					m_glGLSamplesPassedQuery->Begin();
#endif
					glDrawElementsInstanced(GL_TRIANGLES, mesh->m_vertex_indices[0].size(), GL_UNSIGNED_SHORT, mesh->m_ebo[0].m_offset, emitter->m_visibleparticle_count );
					m_num_draw_calls++;
					m_num_triangles += (mesh->m_vertex_indices[0].size() / 3) * (emitter->m_visibleparticle_count);
					m_num_vertices += (mesh->m_vertex_indices[0].size()) * (emitter->m_visibleparticle_count);

#ifdef OCCLUSION_QUERY_BASED_STAT
					m_glGLSamplesPassedQuery->End();
					m_num_samples_passed += m_glGLSamplesPassedQuery->Result();

#ifdef CALCULATE_SHADER_INSTRUCTION_COUNT
					m_num_instruction += shader->m_instruction_count_v * (mesh->m_vertex_indices[0].size() * emitter->m_visibleparticle_count) + shader->m_instruction_count_f * m_glGLSamplesPassedQuery->Result();
#endif

#endif

					VboPool::Instance()->BindBuffer(0);
					IndexBufferPool::Instance()->BindBuffer(0);

					for( KCL::uint32 l=0; l<8; l++)
					{
						if( shader->m_attrib_locations[l] > -1)
						{
							GLB::OpenGLStateManager::GlDisableVertexAttribArray( shader->m_attrib_locations[l]);
						}
					}

					if(shader->m_attrib_locations[attribs::in_instance_position] > -1)
					{
						glVertexAttribDivisor( shader->m_attrib_locations[attribs::in_instance_position], 0 );
						GLB::OpenGLStateManager::GlDisableVertexAttribArray( shader->m_attrib_locations[attribs::in_instance_position]); //instance position
					}
					if(shader->m_attrib_locations[attribs::in_instance_life] > -1)
					{
						glVertexAttribDivisor( shader->m_attrib_locations[attribs::in_instance_life], 0 );
						GLB::OpenGLStateManager::GlDisableVertexAttribArray( shader->m_attrib_locations[attribs::in_instance_life]); //instance life
					}
					if(shader->m_attrib_locations[attribs::in_instance_speed] > -1)
					{
						glVertexAttribDivisor( shader->m_attrib_locations[attribs::in_instance_speed], 0 );
						GLB::OpenGLStateManager::GlDisableVertexAttribArray( shader->m_attrib_locations[attribs::in_instance_speed]); //instance speed
					}
					if(shader->m_attrib_locations[attribs::in_instance_size] > -1)
					{
						glVertexAttribDivisor( shader->m_attrib_locations[attribs::in_instance_size], 0 );
						GLB::OpenGLStateManager::GlDisableVertexAttribArray( shader->m_attrib_locations[attribs::in_instance_size]); //instance size
					}


					material->postInit();

				}//if(emitter->VisibleParticleCount()) END
			}
		}
	}
#endif
}

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //              BLEND LIGHTSHAFTS OVER
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
if(!(m_disabledRenderBits & RenderBits::ERB_LightShafts))
{
	RenderLightshafts();
}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	GLB::OpenGLStateManager::GlDisable( GL_CULL_FACE);
	GLB::OpenGLStateManager::GlDisable( GL_BLEND);
	GLB::OpenGLStateManager::GlDisable( GL_DEPTH_TEST);
	GLB::OpenGLStateManager::GlDepthMask( 1);
	if( !VAO_enabled)
	{
		GLB::OpenGLStateManager::GlDisableVertexAttribArray( 0);
	}

	//Remove Camfogshaft
	//delete l;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //              BLEND TRANSPARENTS OVER
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
if(!(m_disabledRenderBits & RenderBits::ERB_Transparents))
{
    Shader::m_last_shader = 0;
	//transparent meshes
	GLB_Scene_ES2::Render(m_active_camera, m_visible_meshes[1], 0, 0, 0, 0);
}

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //              RENDER LENSFLARES
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
if(!(m_disabledRenderBits & RenderBits::ERB_LensFlareQuery) && !(m_disabledRenderBits & RenderBits::ERB_LensFlares))
{
	if( occlusion_query_enable)
	{
		for( KCL::uint32 i=0; i<m_visible_lights.size(); i++)
		{
			GLB::Light *l = static_cast<GLB::Light*>(m_visible_lights[i]);

			if (l->m_has_lensflare)
			{
				KCL::uint32 num_samples = 0;

				if( l->IsPreviousQueryObjectInitialized())
				{
					glGetQueryObjectuiv( l->GetPreviousQueryObject(), GL_QUERY_RESULT, &num_samples);
				}

				if( num_samples)
				{
					l->visible_meshes[0].push_back( m_lens_flare_mesh);

					GLB_Scene_ES2::Render(m_active_camera, l->visible_meshes[0], 0, 0, 0, l);

					l->visible_meshes[0].clear();

                #ifdef OCCLUSION_QUERY_BASED_STAT
					m_num_samples_passed += num_samples;

                #ifdef CALCULATE_SHADER_INSTRUCTION_COUNT
					m_num_instruction += m_occlusion_query_shader->m_instruction_count_f * num_samples;
                #endif
                #endif
				}
			}
		}
	}
}

#if 0
    Debug_RenderStatics();
#endif

	RenderEffect( PostEffect);

	if( VAO_enabled)
	{
	glBindVertexArray( 0);
	}
	else
	{
		glBindBuffer( GL_ARRAY_BUFFER, 0);
	}

	GLB::OpenGLStateManager::GlDepthMask( 1);

    for(int i=0; i<16; ++i)
    {
        glBindSampler(i, 0);
    }







#ifdef TEXTURE_COUNTING
	Material::NullTextureCounter();
	m_num_used_texture = m_textureCounter.size();
#endif

	//printf(" %d, %d\n", m_num_draw_calls, m_num_triangles);
	//log_file = fopen( "log2.txt", "at");
	//fprintf( log_file, "%d, %d\n", m_animation_time, num_triangles);
	//fclose( log_file);
}

void GLB_Scene_ES3::DoLightingPass()
{
    //needs clearing regardless of usage, because shading uses it
	pp.BindFinalBuffer();
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f);
	glClear( GL_COLOR_BUFFER_BIT);

if(!(m_disabledRenderBits & RenderBits::ERB_Lighting))
{

#define DEPTH_TEST_LIGHTS

#ifdef DEPTH_TEST_LIGHTS
    {
	    GLB::OpenGLStateManager::GlEnable( GL_CULL_FACE);
	    GLB::OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
	    GLB::OpenGLStateManager::GlDepthFunc( GL_GEQUAL);
	    GLB::OpenGLStateManager::GlDepthMask(0);
	    GLB::OpenGLStateManager::GlCullFace( GL_FRONT);
    }
#else
    {
        GLB::OpenGLStateManager::GlEnable( GL_CULL_FACE);
	    GLB::OpenGLStateManager::GlDisable( GL_DEPTH_TEST);
	    GLB::OpenGLStateManager::GlCullFace( GL_FRONT);
    }
#endif

	for( KCL::uint32 i=0; i<m_visible_lights.size(); i++)
	{
		KCL::Light* l = m_visible_lights[i];

		if( i == 1)
		{
			GLB::OpenGLStateManager::GlEnable( GL_BLEND);
		}

		if( l->m_light_type == KCL::Light::SSAO)
		{
			GLB::OpenGLStateManager::GlBlendFunc( GL_DST_COLOR, 0);
		}
		else
		{
			GLB::OpenGLStateManager::GlBlendFunc( 1, 1);
		}

		RenderLight( l);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	GLB::OpenGLStateManager::GlDisable( GL_BLEND);

#ifdef DEPTH_TEST_LIGHTS
    {
	    GLB::OpenGLStateManager::GlDisable( GL_CULL_FACE);
        GLB::OpenGLStateManager::GlCullFace( GL_BACK);
	    GLB::OpenGLStateManager::GlDisable( GL_DEPTH_TEST);
	    GLB::OpenGLStateManager::GlDepthFunc( GL_LESS);
	    GLB::OpenGLStateManager::GlDepthMask(1);
    }
#else
    {
        GLB::OpenGLStateManager::GlDisable( GL_CULL_FACE);
    }
#endif
}
}

void GLB_Scene_ES3::RunEffect(Effect effect)
{
	switch(effect)
	{
	case BlurEffect:
	{
		glBindTexture( GL_TEXTURE_2D, filters[10]->m_color_texture);
		glGenerateMipmap( GL_TEXTURE_2D);
		glBindTexture( GL_TEXTURE_2D, 0);
		break;
	}

	case LightningEffect:
		break;

	default:
		break;
	}

}

void GLB_Scene_ES3::RenderEffect(Effect effect)
{
	switch (effect)
	{
	case GLB_Scene_ES3::BlurEffect:
{
		filters[0]->m_active_camera = m_active_camera;
		filters[0]->m_focus_distance = m_camera_focus_distance;
		filters[0]->m_shader = m_pp_shaders[0];
		filters[0]->m_input_textures[0] = filters[10]->m_color_texture;
		filters[0]->m_input_textures[1] = filters[8]->m_color_texture;
		filters[0]->m_input_textures[2] = pp.m_depth_texture;
#ifdef OCCLUSION_QUERY_BASED_STAT
		filters[0]->Render(m_num_draw_calls, m_num_triangles , m_num_vertices, m_textureCounter, m_num_samples_passed , m_num_instruction, m_glGLSamplesPassedQuery);
#else
		filters[0]->Render(m_num_draw_calls, m_num_triangles , m_num_vertices, m_textureCounter, m_num_samples_passed , m_num_instruction);
#endif
}
		break;
	case GLB_Scene_ES3::LightningEffect:
		break;
	case PostEffect:
		{
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //              RENDER + COMBINE ALL POSTEFFECTS: BLOOM, DoF
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
if(!(m_disabledRenderBits & RenderBits::ERB_Post))
{
	RunEffect(BlurEffect);

	if( VAO_enabled)
	{
		glBindVertexArray( m_fullscreen_quad_vao);
	}
	else
	{
		glBindBuffer( GL_ARRAY_BUFFER, m_fullscreen_quad_vbo);
	}

	filters[1]->m_shader = m_pp_shaders[2];
	filters[1]->m_input_textures[0] = filters[10]->m_color_texture;
	//filters[1].m_input_textures[1] = filters[9].m_color_texture;

#ifdef OCCLUSION_QUERY_BASED_STAT
	filters[1]->Render(m_num_draw_calls, m_num_triangles , m_num_vertices, m_textureCounter, m_num_samples_passed , m_num_instruction, m_glGLSamplesPassedQuery);
#else
	filters[1]->Render(m_num_draw_calls, m_num_triangles , m_num_vertices, m_textureCounter, m_num_samples_passed , m_num_instruction);
#endif

	for( KCL::uint32 i=2; i<=8; i++)
	{
		filters[i]->m_shader = m_pp_shaders[1];
		filters[i]->m_input_textures[0] = filters[i-1]->m_color_texture;

#ifdef OCCLUSION_QUERY_BASED_STAT
		filters[i]->Render(m_num_draw_calls, m_num_triangles , m_num_vertices, m_textureCounter, m_num_samples_passed , m_num_instruction, m_glGLSamplesPassedQuery);
#else
		filters[i]->Render(m_num_draw_calls, m_num_triangles , m_num_vertices, m_textureCounter, m_num_samples_passed , m_num_instruction);
#endif
	}

#if 0
	filters[11]->m_shader = m_pp_shaders[1];
	filters[11]->m_input_textures[0] = filters[10]->m_color_texture;

#ifdef OCCLUSION_QUERY_BASED_STAT
	filters[i]->Render(m_num_draw_calls, m_num_triangles , m_num_vertices, m_textureCounter, m_num_samples_passed , m_num_instruction, m_glGLSamplesPassedQuery);
#else
	filters[11]->Render(m_num_draw_calls, m_num_triangles , m_num_vertices, m_textureCounter, m_num_samples_passed , m_num_instruction);
#endif

	for( KCL::uint32 i=12; i<=16; i++)
	{
		filters[i]->m_shader = m_pp_shaders[1];
		filters[i]->m_input_textures[0] = filters[i - 1]->m_color_texture;

#ifdef OCCLUSION_QUERY_BASED_STAT
		filters[i]->Render(m_num_draw_calls, m_num_triangles , m_num_vertices, m_textureCounter, m_num_samples_passed , m_num_instruction, m_glGLSamplesPassedQuery);
#else
		filters[i]->Render(m_num_draw_calls, m_num_triangles , m_num_vertices, m_textureCounter, m_num_samples_passed , m_num_instruction);
#endif
	}
#endif

	//pp
	RenderEffect(BlurEffect);
}
			break;
		}
	default:
		break;
	}
}

#if defined _DEBUG && defined WIN32
static GLubyte line_indices[24] =
{
	0,1,
	1,2,
	2,3,
	3,0,
	4,5,
	5,6,
	6,7,
	7,4,
	3,6,
	2,7,
	0,5,
	1,4
};

void GLB_Scene_ES3::Debug_RenderStatics()
{
    OpenGLStateManager::Reset();

    OpenGLStateManager::DisableAllVertexAttribsInstantCommit();
	OpenGLStateManager::DisableAllCapabilites();
	OpenGLStateManager::Commit();

	//OpenGLStateManager::GlEnable( GL_BLEND);
	//OpenGLStateManager::GlEnable( GL_CULL_FACE);
	//OpenGLStateManager::GlBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	OpenGLStateManager::GlEnable(GL_DEPTH_TEST); //TODO:get access to g-buffers depth data
#ifdef HAVE_GLEW
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
#endif
	glLineWidth( 2);

    GLB::OpenGLStateManager::GlUseProgram(m_shader->m_p);
    Shader::m_last_shader = m_shader;

	if (m_shader->m_uniform_locations[GLB::uniforms::background_color] > -1)
	{
		glUniform3fv(m_shader->m_uniform_locations[GLB::uniforms::background_color], 1, KCL::Vector3D(1, 1, 1));
    }

	OpenGLStateManager::GlEnableVertexAttribArray(m_shader->m_attrib_locations[attribs::in_position]);

    //for( KCL::uint32 i=0; i<m_visible_meshes[0].size(); i++)
    for( KCL::uint32 i=0; i<m_rooms.size(); i++)
    {
        //KCL::AABB aabb = (m_visible_meshes[0])[i]->m_aabb;
        KCL::AABB aabb = m_rooms[i]->m_aabb;
        GLfloat vertices[24];
	    vertices[0]  = aabb.GetMinVertex().x;   vertices[1]  = aabb.GetMinVertex().y;   vertices[2]  = aabb.GetMinVertex().z; //vertex_0
	    vertices[3]  = aabb.GetMaxVertex().x;   vertices[4]  = aabb.GetMinVertex().y;   vertices[5]  = aabb.GetMinVertex().z; //vertex_1
	    vertices[6]  = aabb.GetMaxVertex().x;   vertices[7]  = aabb.GetMaxVertex().y;   vertices[8]  = aabb.GetMinVertex().z; //vertex_2
	    vertices[9]  = aabb.GetMinVertex().x;   vertices[10] = aabb.GetMaxVertex().y;   vertices[11] = aabb.GetMinVertex().z; //vertex_3
	    vertices[12] = aabb.GetMaxVertex().x;   vertices[13] = aabb.GetMinVertex().y;   vertices[14] = aabb.GetMaxVertex().z; //vertex_4
	    vertices[15] = aabb.GetMinVertex().x;   vertices[16] = aabb.GetMinVertex().y;   vertices[17] = aabb.GetMaxVertex().z; //vertex_5
	    vertices[18] = aabb.GetMinVertex().x;   vertices[19] = aabb.GetMaxVertex().y;   vertices[20] = aabb.GetMaxVertex().z; //vertex_6
	    vertices[21] = aabb.GetMaxVertex().x;   vertices[22] = aabb.GetMaxVertex().y;   vertices[23] = aabb.GetMaxVertex().z; //vertex_7

		if (m_shader->m_uniform_locations[GLB::uniforms::mvp] > -1)
	    {
			glUniformMatrix4fv(m_shader->m_uniform_locations[GLB::uniforms::mvp], 1, GL_FALSE, m_active_camera->GetViewProjection());
	    }

	    OpenGLStateManager::Commit();

		glVertexAttribPointer(m_shader->m_attrib_locations[attribs::in_position], 3, GL_FLOAT, 0, 0, vertices);

	    glFrontFace( GL_CW);
	    //glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (const void*)(triangle_indices));
	    glFrontFace( GL_CCW);
	    glDrawElements(GL_LINES, 24, GL_UNSIGNED_BYTE, (const void*)line_indices);
    }

    OpenGLStateManager::GlDepthMask( 1);
	OpenGLStateManager::GlDisableVertexAttribArray(m_shader->m_attrib_locations[attribs::in_position]);

	glLineWidth( 1);
	OpenGLStateManager::GlDisable( GL_BLEND);
	OpenGLStateManager::GlDisable(GL_DEPTH_TEST);

	OpenGLStateManager::DisableAllVertexAttribsInstantCommit();
	OpenGLStateManager::DisableAllCapabilites();
	OpenGLStateManager::Commit();
#ifdef HAVE_GLEW
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
#endif
}

void GLB_Scene_ES3::Debug_RenderLights()
{
    OpenGLStateManager::Reset();

    OpenGLStateManager::DisableAllVertexAttribsInstantCommit();
	OpenGLStateManager::DisableAllCapabilites();
	OpenGLStateManager::Commit();

	//OpenGLStateManager::GlEnable( GL_BLEND);
	//OpenGLStateManager::GlEnable( GL_CULL_FACE);
	//OpenGLStateManager::GlBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	OpenGLStateManager::GlEnable(GL_DEPTH_TEST); //TODO:get access to g-buffers depth data
#ifdef HAVE_GLEW
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
#endif
	glLineWidth( 2);

    GLB::OpenGLStateManager::GlUseProgram(m_shader->m_p);
    Shader::m_last_shader = m_shader;

	if (m_shader->m_uniform_locations[GLB::uniforms::background_color] > -1)
	{
		glUniform3fv(m_shader->m_uniform_locations[GLB::uniforms::background_color], 1, KCL::Vector3D(1, 1, 1));
    }

    for( KCL::uint32 i=0; i<m_visible_lights.size(); i++)
	{
		KCL::Light* l = m_visible_lights[i];

		glUniformMatrix4fv(m_shader->m_uniform_locations[GLB::uniforms::mvp], 1, GL_FALSE, m_active_camera->GetViewProjection().v);

	    //set to opaque
		glUniform1f(m_shader->m_uniform_locations[GLB::uniforms::transparency], 1.0f);

	    int random_seed = 0;
		OpenGLStateManager::GlEnableVertexAttribArray(m_shader->m_attrib_locations[attribs::in_position]);

	    Matrix4x4 model;
	    KCL::uint32 input_textures[] =
	    {
		    pp.m_color_map,
		    pp.m_normal_map,
		    pp.m_reflection_map,
		    pp.m_depth_texture,
		    pp.m_param_map
	    };
	    int light_shader_index = 3;
	    int light_buffer_index = -1;

	    if( l->m_light_type == KCL::Light::SSAO)
	    {
		    light_shader_index = 4;
	    }
	    else if( l->m_light_type == KCL::Light::DIRECTIONAL)
	    {
		    light_shader_index = 0;
	    }
	    else if( l->m_light_type == KCL::Light::OMNI)
	    {
		    light_shader_index = 1;
		    light_buffer_index = 0;

		    model.translate( Vector3D( l->m_world_pom.v[12], l->m_world_pom.v[13], l->m_world_pom.v[14]));
		    model.scale( Vector3D( l->m_radius * 1.25f, l->m_radius * 1.25f, l->m_radius * 1.25f) );
	    }
	    else if( l->m_light_type == KCL::Light::SPOT)
	    {
		    light_shader_index = 2;
		    light_buffer_index = 1;

		    float halfSpotAngle = Math::Rad(0.5f * l->m_spotAngle);

            float scalingFactorx = KCL::Vector3D(l->m_world_pom.v[0], l->m_world_pom.v[1], l->m_world_pom.v[2]).length();
            float scalingFactory = KCL::Vector3D(l->m_world_pom.v[4], l->m_world_pom.v[5], l->m_world_pom.v[6]).length();
            float scalingFactor = KCL::Vector3D(l->m_world_pom.v[8], l->m_world_pom.v[9], l->m_world_pom.v[10]).length();

		    model.zero();
            model.v33 = l->m_radius * (1.0 / scalingFactor);
		    model.v11 = model.v22 = model.v33 * tanf(halfSpotAngle) * 1.2f; //extra opening to counter low tess-factor of the cone
		    model.v43 = -model.v33;	// Translate so the top is at the origo
		    model.v44 = 1;
		    model *= l->m_world_pom;
	    }
	    else if( l->m_light_type == KCL::Light::SHADOW_DECAL)
	    {
		    light_shader_index = 15;
		    light_buffer_index = 0;

		    model.translate( Vector3D( l->m_world_pom.v[12], l->m_world_pom.v[13], l->m_world_pom.v[14]));
		    model.scale( Vector3D( l->m_radius * 1.25f, l->m_radius * 1.25f, l->m_radius * 1.25f) );
	    }
	    else
	    {
		    model.translate( Vector3D( l->m_world_pom.v[12], l->m_world_pom.v[13], l->m_world_pom.v[14]));
		    model.scale( Vector3D( l->m_radius * 1.25f, l->m_radius * 1.25f, l->m_radius * 1.25f) );
	    }

	    if( light_buffer_index == -1)
	    {
		    return;
	    }

	    if( l->m_is_shadow_caster)
	    {
		    //light_shader_index += 10;
	    }

	    Shader *s = m_shader;

	    glUseProgram( s->m_p);

		GLB::OpenGLStateManager::GlEnableVertexAttribArray(s->m_attrib_locations[attribs::in_position]);

	    int j = 5;
	    while( j--)
	    {
			if (s->m_uniform_locations[GLB::uniforms::texture_unit0 + j] > -1)
		    {
			    GLB::OpenGLStateManager::GlActiveTexture( GL_TEXTURE0 + j);
				glUniform1i(s->m_uniform_locations[GLB::uniforms::texture_unit0 + j], j);
			    glBindTexture( GL_TEXTURE_2D, input_textures[j]);
                glBindSampler(j, 0);

    #ifdef TEXTURE_COUNTING
			    m_textureCounter.insert( input_textures[j] );
    #endif
		    }
	    }

    #ifdef USE_UBOs
	    uMeshConsts.mvp = model * m_active_camera->GetViewProjection();
	    uMeshConsts.mv.identity();
	    uMeshConsts.model = model;
	    uMeshConsts.inv_model.identity();
	    uMeshConsts.inv_modelview.identity();
	    glBindBuffer(GL_UNIFORM_BUFFER, m_UBO_ids[Shader::sUBOnames::Mesh]);
	    glBufferData(GL_UNIFORM_BUFFER, sizeof(uMeshConsts), (const void*)(&uMeshConsts), GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_UBO_ids[Shader::sUBOnames::Mesh]);
	    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    #else //meshconsts
		if (s->m_uniform_locations[GLB::uniforms::mvp] > -1)
	    {
			glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::mvp], 1, GL_FALSE, model * m_active_camera->GetViewProjection());
	    }

		if (s->m_uniform_locations[GLB::uniforms::mv] > -1)
	    {
			glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::mv], 1, GL_FALSE, model * m_active_camera->GetView());
	    }

		if (s->m_uniform_locations[GLB::uniforms::model] > -1)
	    {
			glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::model], 1, GL_FALSE, model.v);
	    }
    #endif

    #ifndef USE_UBOs //frameconsts
		if (s->m_uniform_locations[GLB::uniforms::view_dir] > -1)
	    {
		    Vector3D view_dir( -m_active_camera->GetView().v[2], -m_active_camera->GetView().v[6], -m_active_camera->GetView().v[10]);
			glUniform3fv(s->m_uniform_locations[GLB::uniforms::view_dir], 1, view_dir.v);
	    }

		if (s->m_uniform_locations[GLB::uniforms::view_pos] > -1)
	    {
			glUniform3fv(s->m_uniform_locations[GLB::uniforms::view_pos], 1, m_active_camera->GetEye().v);
	    }

		if (s->m_uniform_locations[GLB::uniforms::inv_resolution] > -1)
	    {
			glUniform2f(s->m_uniform_locations[GLB::uniforms::inv_resolution], 1.0f / m_viewport_width, 1.0f / m_viewport_height);
	    }

		if (s->m_uniform_locations[GLB::uniforms::depth_parameters] > -1)
	    {
			glUniform4fv(s->m_uniform_locations[GLB::uniforms::depth_parameters], 1, m_active_camera->m_depth_linearize_factors.v);
	    }
    #endif

    #ifdef USE_UBOs
	    float i = 1.0f;
	    if( l->m_intensity_track)
	    {
		    Vector4D v;

		    l->t = m_animation_time / 1000.0f;

		    _key_node::Get( v, l->m_intensity_track, l->t, l->tb, true);

		    //i = v.x / l->m_intensity;
		    i = 0.01 * v.x;// / l->m_intensity;
	    }
	    uLightConsts.light_colorXYZ_pad = KCL::Vector4D(l->m_diffuse_color.x * i, l->m_diffuse_color.y * i, l->m_diffuse_color.z * i, 0.0f);

	    uLightConsts.light_posXYZ_pad = KCL::Vector4D( l->m_world_pom.v[12], l->m_world_pom.v[13], l->m_world_pom.v[14], 0.0f);
	    uLightConsts.light_x = KCL::Vector4D( -l->m_world_pom.v[8], -l->m_world_pom.v[9], -l->m_world_pom.v[10], l->m_world_pom[14]);

	    Vector2D spot_sin;
	    float fov = l->m_spotAngle;
	    spot_sin.x = cosf( Math::Rad( fov / 2.0f));
	    spot_sin.y = 1.0f / (1.0f - spot_sin.x);
	    uLightConsts.spotcosXY_attenZ_pad = KCL::Vector4D(spot_sin.x, spot_sin.y, -1.0f / (l->m_radius * l->m_radius), 0.0f);

	    glBindBuffer(GL_UNIFORM_BUFFER, m_UBO_ids[Shader::sUBOnames::Light]);
	    glBufferData(GL_UNIFORM_BUFFER, sizeof(uLightConsts), (const void*)(&uLightConsts), GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 3, m_UBO_ids[Shader::sUBOnames::Light]);
	    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    #else
		if (s->m_uniform_locations[GLB::uniforms::light_x] > -1)
	    {
		    Vector3D dir( -l->m_world_pom.v[8], -l->m_world_pom.v[9], -l->m_world_pom.v[10]);
		    dir.normalize();
			glUniform4f(s->m_uniform_locations[GLB::uniforms::light_x],
			    dir.x,
			    dir.y,
			    dir.z,
			    l->m_world_pom[14]);
	    }

		if (s->m_uniform_locations[GLB::uniforms::light_pos] > -1)
	    {
		    Vector3D light_pos( l->m_world_pom.v[12], l->m_world_pom.v[13], l->m_world_pom.v[14]);

			glUniform3fv(s->m_uniform_locations[GLB::uniforms::light_pos], 1, light_pos.v);
	    }

		if (s->m_uniform_locations[GLB::uniforms::light_color] > -1)
	    {
		    float i = 1.0f;
		    if( l->m_intensity_track)
		    {
			    Vector4D v;

			    l->t = m_animation_time / 1000.0f;

			    _key_node::Get( v, l->m_intensity_track, l->t, l->tb, true);

			    //i = v.x / l->m_intensity;
			    i = 0.01 * v.x;// / l->m_intensity;
		    }

			glUniform3f(s->m_uniform_locations[GLB::uniforms::light_color],
			    l->m_diffuse_color.x * i,
			    l->m_diffuse_color.y * i,
			    l->m_diffuse_color.z * i
			    );
	    }

		if (s->m_uniform_locations[GLB::uniforms::attenuation_parameter] > -1)
	    {
		    float r = l->m_radius;

			glUniform1f(s->m_uniform_locations[GLB::uniforms::attenuation_parameter], -1.0f / (r * r));
	    }

		if (s->m_uniform_locations[GLB::uniforms::spot_cos] > -1)
	    {
		    Vector2D spot_sin;

		    float fov = l->m_spotAngle;

		    spot_sin.x = cosf( Math::Rad( fov / 2.0f));
		    spot_sin.y = 1.0f / (1.0f - spot_sin.x);

			glUniform2fv(s->m_uniform_locations[GLB::uniforms::spot_cos], 1, spot_sin.v);
	    }
    #endif

		if (s->m_uniform_locations[GLB::uniforms::shadow_matrix0] > -1)
	    {
			glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::shadow_matrix0], 1, GL_FALSE, m_global_shadowmaps[0]->m_matrix.v);

		    GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0 + 7);
		    glBindTexture( GL_TEXTURE_2D, m_global_shadowmaps[0]->GetTextureId() );
            glBindSampler(7, 0);
			glUniform1i(s->m_uniform_locations[GLB::uniforms::shadow_unit0], 7);
    #ifdef TEXTURE_COUNTING
		    m_textureCounter.insert( m_global_shadowmaps[0]->GetTextureId() );
    #endif

		    GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
	    }

	    glBindBuffer(GL_ARRAY_BUFFER, m_lbos[light_buffer_index].m_vbo);
	    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_lbos[light_buffer_index].m_ebo);

		glVertexAttribPointer(s->m_attrib_locations[attribs::in_position], 3, GL_FLOAT, GL_FALSE, 0, 0);

	    GLB::OpenGLStateManager::Commit();
    #ifdef OCCLUSION_QUERY_BASED_STAT
	    m_glGLSamplesPassedQuery->Begin();
    #endif
	    glDrawElements( GL_TRIANGLES, m_lbos[light_buffer_index].m_num_indices, GL_UNSIGNED_SHORT, 0);

	    m_num_draw_calls++;
	    m_num_triangles += m_lbos[light_buffer_index].m_num_indices / 3;
	    m_num_vertices += m_lbos[light_buffer_index].m_num_indices;


    #ifdef OCCLUSION_QUERY_BASED_STAT
	    m_glGLSamplesPassedQuery->End();
	    m_num_samples_passed += m_glGLSamplesPassedQuery->Result();

    #ifdef CALCULATE_SHADER_INSTRUCTION_COUNT
	    m_num_instruction += s->m_instruction_count_v * m_lbos[light_buffer_index].m_num_indices + s->m_instruction_count_f * m_glGLSamplesPassedQuery->Result();
    #endif

    #endif

    }

    OpenGLStateManager::GlDepthMask( 1);
	OpenGLStateManager::GlDisableVertexAttribArray(m_shader->m_attrib_locations[attribs::in_position]);

	glLineWidth( 1);
	OpenGLStateManager::GlDisable( GL_BLEND);
	OpenGLStateManager::GlDisable(GL_DEPTH_TEST);

	OpenGLStateManager::DisableAllVertexAttribsInstantCommit();
	OpenGLStateManager::DisableAllCapabilites();
	OpenGLStateManager::Commit();
#ifdef HAVE_GLEW
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
#endif
}
#endif

void LoadHudTextures( const std::string &dir)
{
	int i = 0;
	std::vector<GLB::Image2D*> images;
	const char *filenames[] =
	{
		"hud_side.png",
		"hud_bottom.png",
		"hud_top.png",
		"hud_target.png",
		".png"
	};

	while( 1)
	{
		char filename[512];

		sprintf( filename, "%s%s", dir.c_str(), filenames[i]);

		GLB::Image2D *img = new GLB::Image2D;
		images.push_back( img);

		bool b = img->load( filename);
		if( !b)
		{
			break;
		}
		i++;
	}

	for( KCL::uint32 i=0; i<images.size(); i++)
	{
		GLuint texid;
		glGenTextures(1, &texid);
		glBindTexture( GL_TEXTURE_2D, texid);

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		int t = GL_RGBA;
		switch( images[i]->getBpp())
		{
		case 8:
			{
				t = GL_LUMINANCE;
				break;
			}
		case 16:
			{
				t = GL_LUMINANCE_ALPHA;
				break;
			}
		case 24:
			{
				t = GL_RGB;
				break;
			}
		case 32:
			{
				t = GL_RGBA;
				break;
			}
		}
		glTexImage2D( GL_TEXTURE_2D, 0, t, images[i]->getWidth(), images[i]->getHeight(), 0, t, GL_UNSIGNED_BYTE, images[i]->data());

		glBindTexture( GL_TEXTURE_2D, 0);

		delete images[i];

		hud_texids[i] = texid;
	}
}

uint32 CreateRenderbuffer( int samples, uint32 w, uint32 h, GLint format)
{
	int32 num_configs = 0;
	std::vector<int32> configs;
	KCL::uint32 rbo;

	glGetInternalformativ( GL_RENDERBUFFER, format, GL_NUM_SAMPLE_COUNTS, 1, &num_configs);
	configs.resize( num_configs);
	glGetInternalformativ( GL_RENDERBUFFER, format, GL_SAMPLES, num_configs, &configs[0]);

	std::vector<int32>::iterator config_found = std::find( configs.begin(), configs.end(), samples);

	if( config_found == configs.end())
	{
		return 0;
	}

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer( GL_RENDERBUFFER, rbo);
	if( samples)
	{
		glRenderbufferStorageMultisample( GL_RENDERBUFFER, samples, format, w, h);
	}
	else
	{
		glRenderbufferStorage( GL_RENDERBUFFER, format, w, h);
	}
	glBindRenderbuffer( GL_RENDERBUFFER, 0);

	return rbo;
}

uint32 Create2DTexture( KCL::uint32 max_mipmaps, bool linear, uint32 w, uint32 h, GLint format)
{
	uint32 texture_object;

	KCL::uint32 m_uiMipMapCount = 1;

    if( max_mipmaps == 0) //0 means complete mipchain
	{
		KCL::uint32 kk = std::max( w, h);

		while( kk > 1)
		{
			m_uiMipMapCount++;
			kk /= 2;
		}

	}
    else
    {
        m_uiMipMapCount = max_mipmaps;
    }

    bool mipmapped = m_uiMipMapCount > 1;

	glGenTextures(1, &texture_object);

	glBindTexture( GL_TEXTURE_2D, texture_object);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if( linear)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmapped ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmapped ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
	}

	glTexStorage2D( GL_TEXTURE_2D, m_uiMipMapCount, format, w, h);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, m_uiMipMapCount);

	glBindTexture( GL_TEXTURE_2D, 0);

	int e = glGetError();
	if( e)
	{
		INFO( "GL error (%x) - Create2DTexture", e);
	}

	return texture_object;
}



#else

KCL::KCL_Status GLB_Scene_ES3::Process_GL( GLB::FBO_COLORMODE color_mode, GLB::FBO_DEPTHMODE depth_mode, int samples)
{
	return KCL::KCL_TESTERROR_BUILT_WITH_INCOMPATIBLE_ES_VERSION;
}

GLB_Scene_ES3::GLB_Scene_ES3()
{
}


GLB_Scene_ES3::~GLB_Scene_ES3()
{
}


void GLB_Scene_ES3::Render()
{
}


void GLB_Scene_ES3::CreateLBOs()
{
}


void GLB_Scene_ES3::RenderLight( KCL::Light *l)
{

}


void GLB_Scene_ES3::Render( KCL::Camera2* camera, std::vector<Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light, int pass_type)
{
}

void GLB_Scene_ES3::RunEffect(Effect effect)
{
}
void GLB_Scene_ES3::RenderEffect(Effect effect)
{
}

void GLB_Scene_ES3::DoLightingPass()
{
}

KCL::KCL_Status GLB_Scene_ES3::CreateBuffers()
{
}

void GLB_Scene_ES3::DeleteBuffers()
{
}

KCL::KCL_Status GLB_Scene_ES3::reloadShaders()
{
}

Filter::Filter()
{
}

void Filter::SetUniforms()
{
}

bool GLB_Scene_ES3::UseEnvmapMipmaps()
{
	return false;
}


void GLB_Scene_ES3::MoveParticles()
{
}
void GLB_Scene_ES3::RenderTFParticles()
{
}
void GLB_Scene_ES3::RenderLightshafts()
{

}
void GLB_Scene_ES3::QueryLensFlare()
{
}
Filter * GLB_Scene_ES3::CreateFilter()
{
	return 0;
}

#endif
