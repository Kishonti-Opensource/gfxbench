/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_cascaded_shadow_map.h"
#include "gfxb_scene_base.h"
#include "gfxb_frustum_cull.h"
#include "gfxb_shader.h"
#include "gfxb_texture.h"
#include "gfxb_material.h"
#include "gfxb_mesh_shape.h"
#include "gfxb_barrier.h"

#include <sstream>

using namespace GFXB;


CascadedShadowMap::CascadedShadowMap(SceneBase *scene, KCL::uint32 shadow_map_size, NGL_format depth_component_format, MeshFilter *mesh_filter)
{
	m_scene = scene;
	m_shadow_map_size = shadow_map_size;
	m_depth_component_format = depth_component_format;

	// The initial range of the shadow from the light point of view
	m_shadow_positive_range = 50.0f;
	m_shadow_negative_range = 150.0f;
	m_shadow_near = 0.1f;
	m_shadow_far = 220.0f;

	m_fit_mode = FIT_OBB;
	m_selection_mode = SELECTION_MAP_BASED;

	m_cascade_count = 0;
	m_scene_camera = nullptr;
	m_shadow_caster_shader = 0;
	m_shadow_caster_alpha_test_shader = 0;
	m_shadow_caster_skeletal_shader = 0;
	m_shadow_caster_skeletal_alpha_test_shader = 0;
	m_shadow_map = 0;
	m_shadow_mesh_filter = mesh_filter;
}


CascadedShadowMap::~CascadedShadowMap()
{
	for (KCL::uint32 i = 0; i < m_frustum_culls.size(); i++)
	{
		delete m_frustum_culls[i];
	}
}


CascadedShadowMap &CascadedShadowMap::AddCascade(float near_distance)
{
	static KCL::Vector3D debug_colors[] =
	{
		KCL::Vector3D(1, 0, 0),
		KCL::Vector3D(0, 1, 0),
		KCL::Vector3D(0, 0, 1),
		KCL::Vector3D(1, 0, 1),
		KCL::Vector3D(1, 1, 0),
		KCL::Vector3D(0, 1, 1),
	};

	Frustum frustum;
	frustum.near_distance = near_distance;
	frustum.color = debug_colors[m_frustums.size() % COUNT_OF(debug_colors)];
	m_frustums.push_back(frustum);

	m_cascade_count = (KCL::uint32)m_frustums.size();

	return *this;
}


void CascadedShadowMap::FinalizeCascades()
{
	m_cascade_count = (KCL::uint32)m_frustums.size();
	if (m_cascade_count == 0)
	{
		INFO("CascadedShadowMap - Error: No cascades specified!");
	}

	if (m_cascade_count > 0)
	{
		// Finalize values
		float overlap_factor = 1.0005f; // TODO: Map based, distance based?
		overlap_factor = 1.0f;

		for (KCL::uint32 i = 0; i < m_cascade_count - 1; i++)
		{
			// Set the far distance
			m_frustums[i].far_distance = m_frustums[i + 1].near_distance * overlap_factor;
		}

		// TODO: Respect shadow near
		// Ensure constraints
		m_frustums[0].near_distance = 0.0f;
		m_frustums[m_cascade_count - 1].far_distance = m_shadow_far;
	}

	m_shadow_renderers.resize(m_cascade_count, 0);
	m_shadow_matrices.resize(m_cascade_count);

	for (KCL::uint32 i = 0; i < m_cascade_count; i++)
	{
		m_frustum_culls.push_back(new FrustumCull(m_scene, m_shadow_mesh_filter));
	}

	InitNGLResources();
}


void CascadedShadowMap::InitNGLResources()
{
	m_shadow_caster_shader = ShaderFactory::GetInstance()->AddDescriptor(ShaderDescriptor("shadow_caster.vert", "shadow_caster.frag"));
	m_shadow_caster_alpha_test_shader = ShaderFactory::GetInstance()->AddDescriptor(ShaderDescriptor("shadow_caster.vert", "shadow_caster.frag").AddDefine("ALPHA_TEST"));

	// Skeletal mesh
	ShaderDescriptor skeletal_desc("shadow_caster.vert", "shadow_caster.frag");
	skeletal_desc.AddDefineInt("SKELETAL", 1);
	skeletal_desc.AddDefineInt("MAX_BONES", 3 * KCL::Mesh3::MAX_BONES); // One bone needs 3 vec4
	m_shadow_caster_skeletal_shader = ShaderFactory::GetInstance()->AddDescriptor(skeletal_desc);

	skeletal_desc.AddDefine("ALPHA_TEST");
	m_shadow_caster_skeletal_alpha_test_shader = ShaderFactory::GetInstance()->AddDescriptor(skeletal_desc);

	NGL_texture_descriptor desc;
	desc.m_name = "cascaded shadow map";
	desc.m_type = NGL_TEXTURE_2D_ARRAY;
	desc.m_filter = NGL_NEAREST;
	desc.m_wrap_mode = NGL_CLAMP_ALL;
	desc.m_size[0] = m_shadow_map_size;
	desc.m_size[1] = m_shadow_map_size;
	desc.m_num_array = m_cascade_count;
	desc.m_format = m_depth_component_format;
	desc.m_is_renderable = true;
	desc.m_clear_value[0] = 1.0f;
	nglGenTexture(m_shadow_map, desc, nullptr);
	Transitions::Get().Register(m_shadow_map, desc);

	for (KCL::uint32 i = 0; i < m_cascade_count; i++)
	{
		NGL_job_descriptor rrd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_shadow_map;
			ad.m_attachment.m_layer = i;
			ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
		}
		{
			std::stringstream sstream;
			sstream << "csm#" << i;

			NGL_subpass sp;
			sp.m_name = sstream.str();
			sp.m_usages.push_back(NGL_DEPTH_ATTACHMENT);
			rrd.m_subpasses.push_back(sp);
		}

		rrd.m_load_shader_callback = LoadShader;
		m_shadow_renderers[i] = nglGenJob(rrd);

		int32_t viewport[4] =
		{
			0, 0,
			(int32_t)m_shadow_map_size,
			(int32_t)m_shadow_map_size
		};
		nglViewportScissor(m_shadow_renderers[i], viewport, viewport);
		nglDepthState(m_shadow_renderers[i], NGL_DEPTH_LESS, true);
	}

	m_bias_matrix.identity();

	m_bias_matrix.translate(KCL::Vector3D(0.5f, 0.5f, (nglGetInteger(NGL_DEPTH_MODE) == NGL_NEGATIVE_ONE_TO_ONE) ? 0.5f : 0.0f));
	m_bias_matrix.scale(KCL::Vector3D(0.5f, (nglGetInteger(NGL_RASTERIZATION_CONTROL_MODE) == NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP) ? -0.5f : 0.5f, (nglGetInteger(NGL_DEPTH_MODE) == NGL_NEGATIVE_ONE_TO_ONE) ? 0.5f : 1.0f));
}


void CascadedShadowMap::DeletePipelines()
{
	for (size_t i = 0; i < m_shadow_renderers.size(); i++)
	{
		nglDeletePipelines(m_shadow_renderers[i]);
	}
}


void CascadedShadowMap::BuildFrustums(const KCL::Camera2 *active_camera, const KCL::Vector3D &light_dir)
{
	m_light_dir = light_dir;
	m_light_dir.normalize();    // Be sure it is normalized
	m_scene_camera = active_camera;

	// Update the shadow frustums
	Update();

	// Calculate the frustum distances in projected view space and the shadow matrices
	const KCL::Matrix4x4 & camera_projection = m_scene_camera->GetProjection();
	for (KCL::uint32 i = 0; i < m_cascade_count; i++)
	{
		// Calculate the split depth values in NDC: projMatrix * vec4(0, 0, m_frustums[i].far_distance, 1) * 0.5 + 0.5
		m_frustum_distances.v[i] = 0.5f * (-m_frustums[i].far_distance * camera_projection.v33 + camera_projection.v43) / m_frustums[i].far_distance + 0.5f;
	}
}


void CascadedShadowMap::SplitFrustumsLogaritmic()
{
	float lambda = 0.5f;
	float overlap_factor = 1.005f;

	float ratio = m_shadow_far / m_shadow_near;
	float frustum_length = m_shadow_far - m_shadow_near;

	m_frustums[0].near_distance = m_shadow_near;
	m_frustums[m_cascade_count - 1].far_distance = m_shadow_far;

	for (KCL::uint32 i = 1; i < m_cascade_count; i++)
	{
		float si = i / (float)m_cascade_count;

		m_frustums[i].near_distance = lambda*(m_shadow_near * powf(ratio, si)) + (1.0f - lambda) * (m_shadow_near + frustum_length * si);
		m_frustums[i - 1].far_distance = m_frustums[i].near_distance * overlap_factor;
	}
}


void CascadedShadowMap::Update()
{
	// SplitFrustumsLogaritmic(frustum_near, frustum_far);

	// Calculate the world space view frustum
	KCL::Matrix4x4 inverse_projection;
	KCL::Matrix4x4::Invert4x4(m_scene_camera->GetProjection(), inverse_projection);

	KCL::Matrix4x4 inverse_view;
	KCL::Matrix4x4::Invert4x4(m_scene_camera->GetView(), inverse_view);

	// The light's coordinate system
	KCL::Vector3D right = KCL::Vector3D::cross(KCL::Vector3D(0.0f, 1.0f, 0.0f), m_light_dir);
	right.normalize();
	KCL::Vector3D up = KCL::Vector3D::cross(m_light_dir, right);
	up.normalize();

	// Frustum points in view projection space
	KCL::Vector3D projected_frustum_points[8] =
	{
		KCL::Vector3D(-1.0f, -1.0f, -1.0f), KCL::Vector3D(-1.0f, 1.0f, -1.0f), KCL::Vector3D(1.0f, 1.0f, -1.0f), KCL::Vector3D(1.0f, -1.0f, -1.0f),
		KCL::Vector3D(-1.0f, -1.0f, 1.0f), KCL::Vector3D(-1.0f, 1.0f, 1.0f), KCL::Vector3D(1.0f, 1.0f, 1.0f), KCL::Vector3D(1.0f, -1.0f, 1.0f),
	};

	// Frustum in view space
	for (KCL::uint32 i = 0; i < 8; i++)
	{
		m_view_frustum_points[i] = KCL::Vector3D(inverse_projection * KCL::Vector4D(projected_frustum_points[i]));
	}

	// Split directions in view space
	KCL::Vector3D directions[4];
	for (int i = 0; i < 4; i++)
	{
		directions[i] = m_view_frustum_points[i + 4] - m_view_frustum_points[i];
		directions[i].normalize();
	}

	// Split the view frustum
	float half_size = m_shadow_map_size / 2.0f;
	for (KCL::uint32 i = 0; i < m_cascade_count; i++)
	{
		// Create the bounding box of the frustum split
		KCL::Vector3D bb_min(FLT_MAX, FLT_MAX, FLT_MAX);
		KCL::Vector3D bb_max = bb_min * -1.0f;

		// Calculate the view space bounding box
		// TODO: View space bounding box with view space slipping planes should be much strait forward
		for (int j = 0; j < 4; j++)
		{
			float t = (-m_frustums[i].near_distance - m_view_frustum_points[j].z) / directions[j].z;
			m_frustums[i].points[j] = m_view_frustum_points[j] + directions[j] * t;

			t = (-m_frustums[i].far_distance - m_view_frustum_points[j].z) / directions[j].z;
			m_frustums[i].points[j + 4] = m_view_frustum_points[j] + directions[j] * t;

			ExpandBoundingBox(bb_min, bb_max, m_frustums[i].points[j]);
			ExpandBoundingBox(bb_min, bb_max, m_frustums[i].points[j + 4]);
		}
		m_frustums[i].view_space_aabb_min = bb_min;
		m_frustums[i].view_space_aabb_max = bb_max;

		// World space bounding sphere
		m_frustums[i].target = (bb_max + bb_min) * 0.5f;
		m_frustums[i].radius = KCL::Vector3D::length(bb_max - bb_min) * 0.5f;

		if (m_fit_mode == FIT_SPHERE)
		{
			// Modify the light direction to look to the center of the shadow map
			KCL::Vector3D target(inverse_view * KCL::Vector4D(m_frustums[i].target, 1.0));
			float x = ceilf(KCL::Vector3D::dot(target, up) * half_size / m_frustums[i].radius) * m_frustums[i].radius / half_size;
			float y = ceilf(KCL::Vector3D::dot(target, right) * half_size / m_frustums[i].radius) * m_frustums[i].radius / half_size;
			target = up * x + right * y + m_light_dir * KCL::Vector3D::dot(target, m_light_dir);

			KCL::Vector3D camera_pos = target + m_light_dir;// * (m_default_shadow_range + m_frustums[i].radius);
			KCL::Vector3D camera_dir = target - m_light_dir;
			m_frustums[i].camera.LookAt(camera_pos, camera_dir, up);
		}
		else
		{
			KCL::Vector3D camera_pos = KCL::Vector3D(0.0f, 0.0f, 0.0f);
			KCL::Vector3D camera_dir = -m_light_dir;
			m_frustums[i].camera.LookAt(camera_pos, camera_dir, up);
		}

		// Calculate the light space BB of the frustum
		KCL::Matrix4x4 viewspace_to_lightspace = inverse_view * m_frustums[i].camera.GetView();
		m_frustums[i].light_space_aabb_min = KCL::Vector3D(FLT_MAX, FLT_MAX, FLT_MAX);
		m_frustums[i].light_space_aabb_max = KCL::Vector3D(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		for (KCL::uint32 j = 0; j < 8; j++)
		{
			KCL::Vector4D light_space_frustum_point = viewspace_to_lightspace * KCL::Vector4D(m_frustums[i].points[j], 1.0f);
			ExpandBoundingBox(m_frustums[i].light_space_aabb_min, m_frustums[i].light_space_aabb_max, KCL::Vector3D(light_space_frustum_point));
		}

		// Fix up the edge shimmering
		// TODO: Refactor!
		if (m_fit_mode == FIT_OBB)
		{
			KCL::Vector3D normalized_texture_size = KCL::Vector3D(1.0f / m_shadow_map_size, 1.0f / m_shadow_map_size, 0.0f);

			KCL::Vector3D world_units_per_texel = m_frustums[i].light_space_aabb_max - m_frustums[i].light_space_aabb_min;

			world_units_per_texel.x = world_units_per_texel.x * normalized_texture_size.x;
			world_units_per_texel.y = world_units_per_texel.y * normalized_texture_size.y;
			world_units_per_texel.z = 0.0f;

			m_frustums[i].light_space_aabb_min.x = m_frustums[i].light_space_aabb_min.x / world_units_per_texel.x;
			m_frustums[i].light_space_aabb_min.y = m_frustums[i].light_space_aabb_min.y / world_units_per_texel.y;
			m_frustums[i].light_space_aabb_min.x = floorf(m_frustums[i].light_space_aabb_min.x);
			m_frustums[i].light_space_aabb_min.y = floorf(m_frustums[i].light_space_aabb_min.y);
			m_frustums[i].light_space_aabb_min.x = m_frustums[i].light_space_aabb_min.x * world_units_per_texel.x;
			m_frustums[i].light_space_aabb_min.y = m_frustums[i].light_space_aabb_min.y * world_units_per_texel.y;

			m_frustums[i].light_space_aabb_max.x = m_frustums[i].light_space_aabb_max.x / world_units_per_texel.x;
			m_frustums[i].light_space_aabb_max.y = m_frustums[i].light_space_aabb_max.y / world_units_per_texel.y;
			m_frustums[i].light_space_aabb_max.x = floorf(m_frustums[i].light_space_aabb_max.x);
			m_frustums[i].light_space_aabb_max.y = floorf(m_frustums[i].light_space_aabb_max.y);
			m_frustums[i].light_space_aabb_max.x = m_frustums[i].light_space_aabb_max.x * world_units_per_texel.x;
			m_frustums[i].light_space_aabb_max.y = m_frustums[i].light_space_aabb_max.y * world_units_per_texel.y;
		}

		// Move the near plane so we don't crop the shadow casters outside the view frustum
		m_frustums[i].light_space_aabb_max.z += m_shadow_negative_range;

		if (m_selection_mode == SELECTION_MAP_BASED)
		{
			// Move the far plane so we won't miss meshes during the frustum cull
			m_frustums[i].light_space_aabb_min.z -= m_shadow_positive_range;
		}

		// Calculate the cull planes
		KCL::Matrix4x4 inverse_light;
		KCL::Matrix4x4::Invert4x4(m_frustums[i].camera.GetView(), inverse_light);
		CreateCullPlanes(m_frustums[i].light_space_aabb_min, m_frustums[i].light_space_aabb_max, inverse_light, m_frustums[i].cull_planes);

		float camera_far2 = -m_frustums[i].light_space_aabb_min.z;
		float camera_near2 = -m_frustums[i].light_space_aabb_max.z;
		m_frustums[i].camera.Ortho(
			m_frustums[i].light_space_aabb_min.x,
			m_frustums[i].light_space_aabb_max.x,
			m_frustums[i].light_space_aabb_min.y,
			m_frustums[i].light_space_aabb_max.y,
			camera_near2, camera_far2);
		m_frustums[i].camera.Update();

		m_shadow_matrices[i] = m_frustums[i].camera.GetViewProjection() * m_bias_matrix;
	}
}


void CascadedShadowMap::ExecuteFrustumCull(KCL::uint32 index)
{
	m_frustum_culls[index]->Cull(&m_frustums[index].camera);
}


KCL::uint32 CascadedShadowMap::RenderShadow(KCL::uint32 command_buffer, KCL::uint32 cascade_index)
{
	ExecuteFrustumCull(cascade_index);
	std::vector<KCL::Mesh*> &visible_meshes = m_frustum_culls[cascade_index]->m_visible_meshes[0];

#if 0
	std::vector<KCL::Mesh*> all_meshes;
	for (size_t i = 0; i < m_scene->m_rooms.size(); i++)
	{
		for (size_t j = 0; j < m_scene->m_rooms[i]->m_meshes.size(); j++)
		{
			all_meshes.push_back(m_scene->m_rooms[i]->m_meshes[j]);
		}
	}
	for (size_t i = 0; i < m_scene->m_actors.size(); i++)
	{
		for (size_t j = 0; j < m_scene->m_actors[i]->m_meshes.size(); j++)
		{
			all_meshes.push_back(m_scene->m_actors[i]->m_meshes[j]);
		}
	}
	visible_meshes = all_meshes;
#endif

	NGL_texture_subresource subresource(m_shadow_map, 0, cascade_index);
	Transitions::Get().TextureBarrier(subresource, NGL_DEPTH_ATTACHMENT).Execute(command_buffer);

	const KCL::Camera2 *camera = GetCamera(cascade_index);
	KCL::uint32 ngl_job = m_shadow_renderers[cascade_index];

	nglBegin(ngl_job, command_buffer);

	KCL::Matrix4x4 vp = camera->GetViewProjection();
	KCL::Matrix4x4 mvp;

	const void* p[UNIFORM_MAX];
	p[UNIFORM_VP] = vp.v;
	p[UNIFORM_MVP] = mvp.v;

	NGL_cull_mode cull_mode = NGL_FRONT_SIDED;

	for (size_t i = 0; i < visible_meshes.size(); i++)
	{
		KCL::Mesh *mesh = visible_meshes[i];

		Mesh3 *mesh3 = (Mesh3*)mesh->m_mesh;
		Material *material = (Material*)mesh->m_material;
		cull_mode = material->m_is_two_sided ? NGL_TWO_SIDED : NGL_FRONT_SIDED;
		//cull_mode = NGL_TWO_SIDED;

		mvp = mesh->m_world_pom * camera->GetViewProjection();

		p[UNIFORM_COLOR_TEX] = &material->GetTexture(KCL::Material::COLOR)->m_id;
		p[UNIFORM_ALPHA_TEST_THRESHOLD] = &material->m_alpha_test_threshold;

		KCL::uint32 shader_code = m_shadow_caster_shader;

		bool use_alpha_test = mesh->m_material->m_opacity_mode == KCL::Material::ALPHA_TEST;

		if (mesh->m_mesh->m_vertex_matrix_indices.size())
		{
			p[UNIFORM_BONES] = &mesh3->m_node_matrices[0];
			shader_code = use_alpha_test ? m_shadow_caster_skeletal_alpha_test_shader : m_shadow_caster_skeletal_shader;
		}
		else if (use_alpha_test)
		{
			shader_code = m_shadow_caster_alpha_test_shader;
		}

		//nglDrawFrontSided(ngl_job, shader_code, mesh3->m_vbid, mesh3->m_ibid, p);
		nglDraw( ngl_job, NGL_TRIANGLES, shader_code, 1, &mesh3->m_shadow_vbid, mesh3->m_ibid, cull_mode, p );
	}

	nglEnd(ngl_job);

	return ngl_job;
}


void CascadedShadowMap::ExpandBoundingBox(KCL::Vector3D & min, KCL::Vector3D & max, const KCL::Vector3D & p)
{
	if (min.x > p.x)
	{
		min.x = p.x;
	}
	if (max.x < p.x)
	{
		max.x = p.x;
	}

	if (min.y > p.y)
	{
		min.y = p.y;
	}
	if (max.y < p.y)
	{
		max.y = p.y;
	}

	if (min.z > p.z)
	{
		min.z = p.z;
	}
	if (max.z < p.z)
	{
		max.z = p.z;
	}
}


KCL::Vector4D CascadedShadowMap::CreatePlane(const KCL::Vector3D &a, const KCL::Vector3D &b, const KCL::Vector3D &c)
{
	KCL::Vector3D n = KCL::Vector3D::cross(a - b, c - b);
	n.normalize();
	float d = -(n.x * a.x + n.y * a.y + n.z * a.z);
	return KCL::Vector4D(n.x, n.y, n.z, d);
}


KCL::Vector4D CascadedShadowMap::CreatePlane(const KCL::Vector3D &a, const KCL::Vector3D &b)
{
	KCL::Vector3D n = b - a;
	n.normalize();
	float d = -(n.x * a.x + n.y * a.y + n.z * a.z);
	return KCL::Vector4D(n.x, n.y, n.z, d);
}


void CascadedShadowMap::CreateCullPlanes(const KCL::Vector3D &min, const KCL::Vector3D &max, const KCL::Matrix4x4 &matrix, KCL::Vector4D *planes)
{
	static KCL::Vector3D v[8];

	v[0] = KCL::Vector3D(min.x, min.y, min.z);
	v[1] = KCL::Vector3D(max.x, min.y, min.z);
	v[2] = KCL::Vector3D(max.x, max.y, min.z);
	v[3] = KCL::Vector3D(min.x, max.y, min.z);

	v[4] = KCL::Vector3D(min.x, min.y, max.z);
	v[5] = KCL::Vector3D(max.x, min.y, max.z);
	v[6] = KCL::Vector3D(max.x, max.y, max.z);
	v[7] = KCL::Vector3D(min.x, max.y, max.z);

	for (KCL::uint32 i = 0; i < 8; i++)
	{
		v[i] = KCL::Vector3D(matrix * KCL::Vector4D(v[i], 1.0f));
	}

	// Set the cull planes
	planes[KCL::CULLPLANE_LEFT] = CreatePlane(v[0], v[1]);
	planes[KCL::CULLPLANE_RIGHT] = CreatePlane(v[1], v[0]);

	planes[KCL::CULLPLANE_BOTTOM] = CreatePlane(v[0], v[3]);
	planes[KCL::CULLPLANE_TOP] = CreatePlane(v[3], v[0]);

	planes[KCL::CULLPLANE_FAR] = CreatePlane(v[0], v[4]);
	planes[KCL::CULLPLANE_NEAR] = CreatePlane(v[4], v[0]);
}


void CascadedShadowMap::ExcludeActor(KCL::Actor *actor)
{
	for (size_t i = 0; i < m_frustum_culls.size(); i++)
	{
		m_frustum_culls[i]->ExcludeActor(actor);
	}
}


CascadedShadowMap &CascadedShadowMap::SetShadowNearFar(float near_distance, float far_distance)
{
	m_shadow_near = near_distance;
	m_shadow_far = far_distance;
	return *this;
}


CascadedShadowMap &CascadedShadowMap::SetShadowNegativeRange(float range)
{
	m_shadow_negative_range = range;
	return *this;
}


CascadedShadowMap &CascadedShadowMap::SetShadowPositiveRange(float range)
{
	m_shadow_positive_range = range;
	return *this;
}



CascadedShadowMap &CascadedShadowMap::SetFitMode(FitMode mode)
{
	m_fit_mode = mode;
	return *this;
}


CascadedShadowMap &CascadedShadowMap::SetSelectionMode(SelectionMode mode)
{
	m_selection_mode = mode;
	return *this;
}


void *CascadedShadowMap::GetUniformShadowMap()
{
	return &m_shadow_map;
}


void *CascadedShadowMap::GetUniformShadowMatrix(KCL::uint32 cascade_index)
{
	return m_shadow_matrices[cascade_index].v;
}


void *CascadedShadowMap::GetUniformShadowMatrixes()
{
	return m_shadow_matrices[0].v;
}


void *CascadedShadowMap::GetUniformFrustumDistances()
{
	return m_frustum_distances.v;
}


//
//	Mesh Filter
//
KCL::int32 ShadowMeshFilter::FilterMesh(KCL::Camera2 *camera, KCL::Mesh *mesh, KCL::OverlapResult overlap_result)
{
	const KCL::Material *material = mesh->m_material;

	// Billboards are not yet supported
	bool shadow_casting_billboard = material->m_is_billboard && material->m_is_shadow_caster;
	if (!material->m_is_shadow_only && !shadow_casting_billboard && !(mesh->m_flags & KCL::Mesh::OF_SHADOW_CASTER))
	{
		return -1;
	}


	// Beizer patches are not supported yet
	if (material->m_displacement_mode == KCL::Material::DISPLACEMENT_ABS)
	{
		return -1;
	}

	return 0;
}


KCL::uint32 ShadowMeshFilter::GetMaxMeshTypes()
{
	return 1;
}


FrustumCull::PFN_MeshCompare ShadowMeshFilter::GetMeshSortFunction(KCL::uint32 mesh_Type)
{
	return FrustumCull::DepthCompare;
}
