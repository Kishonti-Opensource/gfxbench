/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_shadow_map.h"

#include "gfxb_barrier.h"
#include "gfxb_shader.h"
#include "gfxb_frustum_cull.h"
#include "gfxb_mesh.h"
#include "gfxb_mesh_shape.h"
#include "gfxb_material.h"
#include "gfxb_texture.h"
#include "gfxb_light.h"
#include <kcl_scene_handler.h>
#include <kcl_camera2.h>

using namespace GFXB;

ShadowMap::ShadowMap()
{
	m_name = "#shadow_map#";
	m_type = DIRECTIONAL;
	m_scene = nullptr;
	m_frustum_cull = nullptr;
	m_point_radius = 1.0f;
	m_point_near = 0.1f;
	m_shadow_texture = 0;
	m_light = nullptr;
	m_include_actors_on_update = false;
}


ShadowMap::~ShadowMap()
{
	delete m_frustum_cull;
}


GFXB::ShadowMap* ShadowMap::CreateShadowMap(KCL::SceneHandler *scene, GFXB::Light* owner_light, MeshFilter *shadow_filter, KCL::uint32 shadow_map_size, NGL_format format)
{
	ShadowMap::ShadowType shadow_type;
	switch (owner_light->m_light_shape->m_light_type)
	{
	case KCL::LightShape::DIRECTIONAL:
		shadow_type = ShadowMap::DIRECTIONAL;
		break;

	case KCL::LightShape::SPOT:
		shadow_type = ShadowMap::PERSPECTIVE;
		break;

	case KCL::LightShape::OMNI:
		shadow_type = ShadowMap::CUBE;
		break;

	default:
		INFO("Light::InitShadowMap - Unhandled light type: %d", owner_light->m_light_shape->m_light_type);
		return nullptr;
	}

	GFXB::ShadowMap* shadow_map = new ShadowMap();

	shadow_map->m_light = owner_light;
	shadow_map->Init(("shadow::" + owner_light->m_name).c_str(), scene, shadow_type, shadow_map_size, shadow_map_size, format, shadow_filter);

	return shadow_map;
}


void ShadowMap::Init(const char *name, KCL::SceneHandler *scene, ShadowType shadow_type, KCL::uint32 width, KCL::uint32 height, NGL_format depth_component_format, MeshFilter *mesh_filter)
{
	INFO("Init shadow map: %s %dx%d", name, width, height);

	m_name = name;

	m_scene = scene;
	m_type = shadow_type;
	m_width = width;
	m_height = height;

	// Setup the camera with some default values
	m_camera.Ortho(scene->m_aabb.GetMinVertex().x, m_scene->m_aabb.GetMaxVertex().x, m_scene->m_aabb.GetMinVertex().z, m_scene->m_aabb.GetMaxVertex().z, -50.0f, 50.0f);
	m_camera.LookAt(KCL::Vector3D(0.0f, 0.0f, 0.0f), -m_scene->m_light_dir, KCL::Vector3D(0.0f, 1.0f, 0.0f));
	m_camera.Update();

	int32_t m_shadow_texture_viewport[4];
	m_shadow_texture_viewport[0] = 0;
	m_shadow_texture_viewport[1] = 0;
	m_shadow_texture_viewport[2] = width;
	m_shadow_texture_viewport[3] = height;


	// Select the texture type accouring to the shadow type
	KCL::uint32 job_count = 0;
		NGL_texture_descriptor texture_layout;
	switch (m_type)
	{
	case CUBE:
		job_count = 6;
		texture_layout.m_type = NGL_TEXTURE_CUBE;
		break;
	case PARABOLOID:
		job_count = 2;
		texture_layout.m_type = NGL_TEXTURE_2D_ARRAY;
		texture_layout.m_num_array = 2;
		break;
	default:
		job_count = 1;
		texture_layout.m_type = NGL_TEXTURE_2D;
	}

	// Shadow texture
	{
		texture_layout.m_name = name;
		texture_layout.m_name += " texture";
		texture_layout.m_filter = NGL_NEAREST;
		if (texture_layout.m_type != NGL_TEXTURE_CUBE)
		{
			texture_layout.m_shadow_filter = NGL_LINEAR;
		}
		texture_layout.m_wrap_mode = NGL_CLAMP_ALL;

		texture_layout.m_size[0] = (uint32_t)m_shadow_texture_viewport[2];
		texture_layout.m_size[1] = (uint32_t)m_shadow_texture_viewport[3];
		texture_layout.m_format = NGL_D24_UNORM;
		texture_layout.m_is_renderable = true;
		texture_layout.SetAllClearValue(1.0f);

		nglGenTexture(m_shadow_texture, texture_layout, nullptr);

		Transitions::Get().Register(m_shadow_texture, texture_layout);
	}

	// Shadow jobs
	for (KCL::uint32 i = 0; i < job_count; i++)
	{
		NGL_job_descriptor rrd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_shadow_texture;
			ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			switch (m_type)
			{
			case PARABOLOID:
				ad.m_attachment.m_layer = i;
				break;

			case CUBE:
				ad.m_attachment.m_face = i;
				break;

			default:
				break;
			}
			rrd.m_attachments.push_back(ad);

			m_render_targets.push_back(ad.m_attachment);
		}
		{
			std::stringstream sstream;
			sstream << name << "::" << i;

			NGL_subpass sp;
			sp.m_name = sstream.str();
			sp.m_usages.push_back(NGL_DEPTH_ATTACHMENT);
			rrd.m_subpasses.push_back(sp);
		}

		rrd.m_load_shader_callback = LoadShader;

		KCL::uint32 job = nglGenJob(rrd);
		m_jobs.push_back(job);

		nglDepthState(job, NGL_DEPTH_LESS_WITH_OFFSET, true);
		nglViewportScissor(job, m_shadow_texture_viewport, m_shadow_texture_viewport);
	}

	// Shaders
	{
		// Normal mesh
		{
			ShaderDescriptor desc("shadow_caster.vert", "shadow_caster.frag");
			desc.AddDefineInt("PARABOLOID", m_type == PARABOLOID);
			m_shaders[0] = ShaderFactory::GetInstance()->AddDescriptor(desc);
		}

		// Alpha tested mesh
		{
			ShaderDescriptor desc("shadow_caster.vert", "shadow_caster.frag");
			desc.AddDefine("ALPHA_TEST");
			desc.AddDefineInt("PARABOLOID", m_type == PARABOLOID);
			m_shaders[1] = ShaderFactory::GetInstance()->AddDescriptor(desc);
		}

		// Skeletal mesh
		{
			ShaderDescriptor desc("shadow_caster.vert", "shadow_caster.frag");
			desc.AddDefineInt("SKELETAL", 1);
			desc.AddDefineInt("MAX_BONES", 3 * KCL::Mesh3::MAX_BONES); // One bone needs 3 vec4
			desc.AddDefineInt("PARABOLOID", m_type == PARABOLOID);
			m_shaders[2] = ShaderFactory::GetInstance()->AddDescriptor(desc);
		}
	}

	// Frustum cull
	if (mesh_filter)
	{
		m_frustum_cull = new FrustumCull(scene, mesh_filter);
		m_frustum_cull->SetCullWithNearFar(true);
	}

	m_state.resize(job_count);

	m_bias_matrix.identity();

	m_bias_matrix.translate(KCL::Vector3D(0.5f, 0.5f, (nglGetInteger(NGL_DEPTH_MODE) == NGL_NEGATIVE_ONE_TO_ONE) ? 0.5f : 0.0f));
	m_bias_matrix.scale(KCL::Vector3D(0.5f, (nglGetInteger(NGL_RASTERIZATION_CONTROL_MODE) == NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP) ? -0.5f : 0.5f, (nglGetInteger(NGL_DEPTH_MODE) == NGL_NEGATIVE_ONE_TO_ONE) ? 0.5f : 1.0f));
}


void ShadowMap::Clear(KCL::uint32 command_buffer)
{
	for (size_t i = 0; i < m_jobs.size(); i++)
	{
		nglBegin(m_jobs[i], command_buffer);
		nglEnd(m_jobs[i]);

		// Invalidate the state
		m_state[i].m_cull_state = INVALID;
	}
}


void ShadowMap::Animate(KCL::uint32 animation_time)
{
	KCL::Vector3D up(m_light->m_world_pom.v[4], m_light->m_world_pom.v[5], m_light->m_world_pom.v[6]);
	up.normalize();

	if (m_light->m_light_shape->m_light_type == KCL::LightShape::SPOT)
	{
		const float near_plane = 0.1f;
		const float far_plane = m_light->m_spot_focus_length;

		GetCamera().Perspective(m_light->m_light_shape->m_fov, 1, 1, near_plane, far_plane);
		GetCamera().LookAt(m_light->m_pos, m_light->m_pos - m_light->m_dir, up);
		GetCamera().Update();
	}
	else if (m_light->m_light_shape->m_light_type == KCL::LightShape::DIRECTIONAL && m_light->m_light_shape->m_box_light)
	{
		const float &hx = m_light->m_light_shape->m_width;
		const float &hy = m_light->m_light_shape->m_height;
		const float &hz = m_light->m_light_shape->m_depth;

		GetCamera().Ortho(-hx, hx, -hy, hy, -hz, hz);
		GetCamera().LookAt(m_light->m_pos, m_light->m_pos - m_light->m_dir, up);
		GetCamera().Update();
	}
	else if (m_light->m_light_shape->m_light_type == KCL::LightShape::OMNI)
	{
		SetPointLightCamera(m_light->m_pos, m_light->m_shadow_far, m_light->m_shadow_near);
	}
}


KCL::uint32 ShadowMap::Render(KCL::uint32 command_buffer)
{
	if (m_type == CUBE)
	{
		const bool left_handed = nglGetInteger(NGL_RASTERIZATION_CONTROL_MODE) == NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP;

		m_camera.Perspective(90.0f, 1, 1, m_point_near, m_point_radius);

		// Render all the 6 faces
		for (KCL::uint32 face = 0; face < 6; face++)
		{
			if (left_handed)
			{
				// Get the forward/up vectors for the cubamap face
				KCL::Vector3D forward;
				KCL::Vector3D up;
				KCL::Camera2::GetOmniVectors(face, forward, up);

				// Invert the up vector
				up = -up;

				// Create left handed view matrix
				KCL::Matrix4x4 view;
				LookAtLeftHeaded(view, m_point_pos, forward, up);

				m_camera.LookAt(view);
			}
			else
			{
				m_camera.LookAtOmni(m_point_pos, face);
			}

			m_camera.Update();

			m_frustum_cull->Cull(&m_camera);

			if (NeedToRedraw(m_state[face]))
			{
				Render(command_buffer, face, m_camera, m_frustum_cull->m_visible_meshes[0]);
			}
		}

		m_camera.LookAtOmni(KCL::Vector3D(0.0f,0.0f,0.0f), 4);
		m_camera.Update();
		m_shadow_matrix = m_camera.GetViewProjection() * m_bias_matrix;
	}
	else if (m_type == PARABOLOID)
	{
		static const KCL::Vector3D dir = KCL::Vector3D(0.0f, 1.0f, 0.0f);
		static const KCL::Vector3D up = KCL::Vector3D(0.0f, 0.0f, 1.0f);

		// Side 1
		{
			m_camera.Ortho(-m_point_radius, m_point_radius, -m_point_radius, m_point_radius, m_point_near, m_point_radius);
			m_camera.LookAt(m_point_pos, m_point_pos + dir, up);
			m_camera.Update();

			m_frustum_cull->Cull(&m_camera);

			Render(command_buffer, 0, m_camera, m_frustum_cull->m_visible_meshes[0]);
		}

		// Side 2
		{
			KCL::Camera2 cull_camera;
			cull_camera.Ortho(-m_point_radius, m_point_radius, -m_point_radius, m_point_radius, m_point_near, m_point_radius);
			cull_camera.LookAt(m_point_pos, m_point_pos - dir, up);
			cull_camera.Update();

			m_frustum_cull->Cull(&cull_camera);

			Render(command_buffer, 1, m_camera, m_frustum_cull->m_visible_meshes[0]);
		}

		m_shadow_matrix = m_camera.GetViewProjection();
	}
	else // Directional or spot shadow
	{
		m_frustum_cull->Cull(&m_camera);

		if (NeedToRedraw(m_state[0]))
		{
			Render(command_buffer, 0, m_camera, m_frustum_cull->m_visible_meshes[0]);

			m_shadow_matrix = m_camera.GetViewProjection() * m_bias_matrix;
		}
	}

	m_include_actors_on_update = false;

	return 0;
}


void ShadowMap::Render(KCL::uint32 command_buffer, KCL::uint32 job_index, const KCL::Camera2 &camera, const std::vector<KCL::Mesh*> &meshes)
{
	KCL::uint32 job = m_jobs[job_index];

	uint32_t shader_code;

	KCL::Matrix4x4 view = camera.GetView();
	KCL::Matrix4x4 vp = camera.GetViewProjection();
	KCL::Matrix4x4 mv;
	KCL::Matrix4x4 mvp;

	const void *p[UNIFORM_MAX];
	p[UNIFORM_VIEW] = view.v;
	p[UNIFORM_VP] = vp.v;
	p[UNIFORM_MV] = mv.v;
	p[UNIFORM_MVP] = mvp.v;

	KCL::Vector4D depth_params = camera.m_depth_linearize_factors;
	if (m_type == PARABOLOID)
	{
		depth_params.x = job_index ? -1.0f : 1.0f;
	}
	p[UNIFORM_DEPTH_PARAMETERS] = depth_params.v;

	Transitions::Get().TextureBarrier(m_render_targets[job_index], NGL_DEPTH_ATTACHMENT).Execute(command_buffer);

	nglBegin(job, command_buffer);

	for (uint32_t i = 0; i < meshes.size(); i++)
	{
		KCL::Mesh *mesh = meshes[i];

		// If we render without frustum culling (shadow for actors) this check comes handy
		if (mesh->m_visible == false)
		{
			continue;
		}

		Mesh3 *mesh3 = (Mesh3*)mesh->m_mesh;
		Material *material = (Material*)mesh->m_material;

		if (mesh->m_mesh->m_vertex_matrix_indices.size())
		{
			// Skeletal mesh
			p[UNIFORM_BONES] = &mesh3->m_node_matrices[0];
			shader_code = m_shaders[2];
		}
		else
		{
			mv = mesh->m_world_pom * camera.GetView();
			mvp = mesh->m_world_pom * camera.GetViewProjection();

			if (material->m_opacity_mode == KCL::Material::ALPHA_TEST)
			{
				// Alpha teseted
				shader_code = m_shaders[1];
			}
			else
			{
				// Normal mesh
				shader_code = m_shaders[0];
			}
		}

		// Bind material uniforms
		p[UNIFORM_COLOR_TEX] = &material->GetTexture(KCL::Material::COLOR)->m_id;
		p[UNIFORM_ALPHA_TEST_THRESHOLD] = &material->m_alpha_test_threshold;

		nglDraw(job, NGL_TRIANGLES, shader_code, 1, &mesh3->m_shadow_vbid, mesh3->m_ibid, material->m_is_two_sided ? NGL_TWO_SIDED : NGL_FRONT_SIDED, &p[0]);
	}

	nglEnd(job);
}


void ShadowMap::Warmup(KCL::uint32 command_buffer, const std::vector<KCL::Mesh*> &meshes)
{
	INFO("Warmup shadow map: %s", m_name.c_str());
	for (KCL::uint32 job = 0; job < m_jobs.size(); job++)
	{
		Render(command_buffer, job, m_camera, meshes);

		m_state[job].m_cull_state = INVALID;
	}
}


void ShadowMap::DeletePipelines()
{
	for (size_t i = 0; i < m_jobs.size(); i++)
	{
		nglDeletePipelines(m_jobs[i]);
	}
}


void ShadowMap::Resize(KCL::uint32 width, KCL::uint32 height)
{
	m_width = width;
	m_height = height;

	KCL::uint32 texture_size[3] = { width, height, 0 };
	nglResizeTextures(1, &m_shadow_texture, texture_size);

	KCL::int32 viewport[4] = { 0, 0, KCL::int32(width), KCL::int32(height) };
	for (size_t i = 0; i < m_jobs.size(); i++)
	{
		nglViewportScissor(m_jobs[i], viewport, viewport);

		// Invalidate the state
		m_state[i].m_cull_state = INVALID;
	}
}


const std::string &ShadowMap::GetName()
{
	return m_name;
}


ShadowMap::ShadowType ShadowMap::GetType() const
{
	return m_type;
}


void ShadowMap::SetPointLightCamera(const KCL::Vector3D pos, float radius, float near_plane)
{
	m_point_pos = pos;
	m_point_radius = radius;
	m_point_near = near_plane;
}


KCL::Camera2 &ShadowMap::GetCamera()
{
	return m_camera;
}


FrustumCull *ShadowMap::GetFrustumCull()
{
	return m_frustum_cull;
}


KCL::uint32 ShadowMap::GetWidth() const
{
	return m_width;
}


KCL::uint32 ShadowMap::GetHeight() const
{
	return m_height;
}


KCL::uint32 ShadowMap::GetShadowTexture() const
{
	return m_shadow_texture;
}


const KCL::Matrix4x4 &ShadowMap::GetShadowMatrix() const
{
	return m_shadow_matrix;
}

void *ShadowMap::GetUniformShadowTexture()
{
	return &m_shadow_texture;
}


void *ShadowMap::GetUniformShadowMatrix()
{
	return m_shadow_matrix.v;
}


const std::vector<NGL_texture_subresource> &ShadowMap::GetRenderTargets() const
{
	return m_render_targets;
}


void ShadowMap::LookAtLeftHeaded(KCL::Matrix4x4 &M, const KCL::Vector3D &eye, const KCL::Vector3D &forward, const KCL::Vector3D &up)
{
	KCL::Matrix4x4::Identity(M);

	KCL::Vector3D u(up);
	KCL::Vector3D side;

	u.normalize();

	side = KCL::Vector3D::cross(up, forward); // swap
	side.normalize();
	u = KCL::Vector3D::cross(forward, side); // swap
	u.normalize();

	M.v11 = side.x; M.v21 = side.y; M.v31 = side.z; M.v41 = 0.0f;
	M.v12 = u.x; M.v22 = u.y; M.v32 = u.z; M.v42 = 0.0f;
	M.v13 = -forward.x; M.v23 = -forward.y; M.v33 = -forward.z; M.v43 = 0.0f;

	M.translate(-eye);
}


bool ShadowMap::NeedToRedraw(UpdateState &state)
{
	const KCL::Matrix4x4 &current_vp = m_camera.GetViewProjection();

	// State is dynamic if frustum contains actors
	CullState current_cull_state = m_frustum_cull->m_visible_actors ? DYNAMIC : STATIC;

	// If both states are static (do not contain actors) and the vp matrices are the same, than the shadow map is up to date
	if (!m_include_actors_on_update || (current_cull_state == STATIC && state.m_cull_state == STATIC))
	{
		if (state.m_vp == current_vp)
		{
			return false;
		}
	}

	// Save the state
	state.m_cull_state = current_cull_state;
	state.m_vp = current_vp;

	return true;
}
