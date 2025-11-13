/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_frustum_cull.h"
#include "gfxb_mesh.h"
#include "gfxb_light.h"
#include "gfxb_lightshaft.h"
#include <kcl_scene_handler.h>
#include <kcl_envprobe.h>
#include <algorithm>

using namespace GFXB;

KCL::uint32 FrustumCull::s_instance_counter = 0;


FrustumCull::FrustumCull(KCL::SceneHandler *scene, MeshFilter *mesh_filter)
{
	m_instance_id = s_instance_counter;
	s_instance_counter++;

	if (m_instance_id > MAX_INSTANCES - 1)
	{
		INFO("FrustumCull: Too many instances!");
		assert(false);
	}

	m_scene = scene;
	m_mesh_filter = mesh_filter;
	m_camera_room = nullptr;
	m_default_near = 0.1f;
	m_default_far = 2000.0f;

	m_near = m_default_near;
	m_far = m_default_far;

	m_partition_enabled = true;
	m_pvs_enabled = true;
	m_cull_actors = true;
	m_cull_lights = false;
	m_cull_probes = false;
	m_cull_with_near_far = false;

	m_visible_meshes.resize(m_mesh_filter->GetMaxMeshTypes());

	m_visible_actors = 0;

	m_frame_counter = 0;
	m_actor_frame_counters.resize(scene->m_actors.size(), 0);
}


FrustumCull::~FrustumCull()
{
	s_instance_counter--;
}


void FrustumCull::Reset()
{
	for (size_t i = 0; i < m_visible_meshes.size(); i++)
	{
		m_visible_meshes[i].clear();
	}

	m_visible_probes.clear();
	m_visible_inside_probes.clear();
	m_visible_outside_probes.clear();

	m_cull_path.clear();

	m_visible_actors = 0;

	m_visible_rooms.clear();
	m_visible_lights.clear();
	m_visible_light_shafts.clear();

	m_near = FLT_MAX;
	m_far = -FLT_MAX;

	m_frame_counter++;
}


void FrustumCull::Cull(KCL::Camera2 *camera)
{
	// Reset the cull state
	Reset();

	m_camera = *camera;

	// Execute the frustum cull with GL camera, so the result will be the same everywhere
	{
		bool original_value = KCL::zRangeZeroToOneGlobalOpt;
		KCL::zRangeZeroToOneGlobalOpt = false;

		if (m_camera.GetFov() == 0.0f)
		{
			m_camera.Ortho(camera->GetLeft(), camera->GetRight(), camera->GetBottom(), camera->GetTop(), camera->GetNear(), camera->GetFar());
		}
		else
		{
			m_camera.Perspective(camera->GetFov(), (KCL::uint32)camera->GetWidth(), (KCL::uint32)camera->GetHeight(), camera->GetNear(), camera->GetFar());
		}
		m_camera.Update();

		KCL::zRangeZeroToOneGlobalOpt = original_value;
	}

	KCL::Vector4D planes[] =
	{
		-m_camera.GetCullPlane(KCL::CULLPLANE_LEFT),
		-m_camera.GetCullPlane(KCL::CULLPLANE_RIGHT),
		-m_camera.GetCullPlane(KCL::CULLPLANE_BOTTOM),
		-m_camera.GetCullPlane(KCL::CULLPLANE_TOP),
		-m_camera.GetCullPlane(KCL::CULLPLANE_NEAR),
		-m_camera.GetCullPlane(KCL::CULLPLANE_FAR)
	};

	const KCL::uint32 plane_count = m_cull_with_near_far ? 6 : 5;

	bool partition_enabled = m_partition_enabled && m_scene->m_rooms.size() > 1;

	if (partition_enabled)
	{
		m_camera_room = m_scene->SearchMyRoomWithPlanes(m_camera.GetEye());
		if (m_camera_room)
		{
			CullRoom(m_camera_room, planes, plane_count, true);

			/*
			if (m_cull_actors == true)
			{
			CullActors();
			}
			*/
		}
		else
		{
			partition_enabled = false;
		}
	}

	if (partition_enabled == false)
	{
		if (m_scene->m_rooms.empty() == false)
		{
			CullRoom(m_scene->m_rooms[0], planes, plane_count, false);
		}

		if (m_cull_actors == true)
		{
			CullActors();
		}
	}
	m_near = m_near < 0.0f || m_near == FLT_MAX ? m_default_near : m_near;

	m_far = m_far < 0.0f || m_far == -FLT_MAX ? m_default_far : m_far + 5.0f;

	// Sort
	for (KCL::uint32 i = 0; i < m_mesh_filter->GetMaxMeshTypes(); i++)
	{
		PFN_MeshCompare compare_function = m_mesh_filter->GetMeshSortFunction(i);
		if (compare_function != nullptr && m_visible_meshes[i].size())
		{
			SortMeshes(m_visible_meshes[i], compare_function);
		}
	}

	if (m_visible_light_shafts.size())
	{
		SortLights(m_visible_light_shafts);
	}
}


void FrustumCull::CullRoom(KCL::XRoom *r, const KCL::Vector4D *planes, KCL::uint32 plane_count, bool partitions_enabled)
{
	m_visible_rooms.insert(r);

	for (size_t i = 0; i< r->m_meshes.size(); i++)
	{
		Mesh *m = (Mesh*)r->m_meshes[i];

		if (m->m_cull_frames[m_instance_id] == m_frame_counter)
		{
			continue;
		}

		KCL::OverlapResult result = KCL::XRoom::OVERLAP(planes, plane_count, &m->m_aabb);

		if (result != KCL::OVERLAP_OUTSIDE)
		{
			m->m_cull_frames[m_instance_id] = m_frame_counter;

			KCL::int32 mesh_type_index = m_mesh_filter->FilterMesh(&m_camera, m, result);

			if (mesh_type_index == -1)
			{
				continue;
			}

			m_visible_meshes[mesh_type_index].push_back(m);
			if (m_instance_id > 1)
			{
				m->m_debug_frustum_ids[m_instance_id - 1].x = 1.0f;
			}
		}
	}

	if (m_cull_actors)
	{
		for (size_t i = 0; i < r->m_actors.size(); i++)
		{
			KCL::Actor *a = r->m_actors[i];

			if (m_actor_frame_counters[a->m_userId] == m_frame_counter)
			{
				continue;
			}

			KCL::OverlapResult result = KCL::XRoom::OVERLAP(planes, plane_count, &a->m_aabb);
			if (result != KCL::OVERLAP_OUTSIDE)
			{
				m_actor_frame_counters[a->m_userId] = m_frame_counter;

				bool has_visible_mesh = false;

				for (size_t j = 0; j < a->m_meshes.size(); j++)
				{
					KCL::Mesh *mesh = a->m_meshes[j];

					KCL::int32 mesh_type_index = m_mesh_filter->FilterMesh(&m_camera, mesh, KCL::OVERLAP_INTERSECT);
					if (mesh_type_index == -1)
					{
						continue;
					}

					if (!mesh->m_visible)
					{
						continue;
					}

					has_visible_mesh = true;

					m_visible_meshes[mesh_type_index].push_back(mesh);
					if (m_instance_id > 1)
					{
						((Mesh*)mesh)->m_debug_frustum_ids[m_instance_id - 1].x = 1.0f;
					}
				}

				if (has_visible_mesh)
				{
					m_visible_actors++;
				}

				// Adjust camera near-far
				AdjustNearFar(a->m_aabb);
			}

			if (m_cull_lights)
			{
				for (size_t j = 0; j < a->m_lights.size(); j++)
				{
					Light *light = (Light*)a->m_lights[j];
					if (light->m_visible)
					{
						CullLight(light);
					}
				}
			}
		}
	}

	if (m_cull_probes)
	{
		KCL::AABB aabb;

		for (size_t i = 0; i < r->m_probes.size(); i++)
		{
			KCL::EnvProbe *probe = r->m_probes[i];

			if (probe->m_frame_counter == m_frame_counter)
			{
				continue;
			}

			aabb = probe->m_aabb;

			if (KCL::XRoom::OVERLAP(planes, plane_count, &aabb) != KCL::OVERLAP_OUTSIDE)
			{
				probe->m_frame_counter = m_frame_counter;

				KCL::uint32 num_intersections;
				KCL::Vector3D vertices[8];

				aabb.ConvertToBox(vertices);

				const KCL::Vector4D &near_plane = -m_camera.GetCullPlane(KCL::CULLPLANE_NEAR);

				for (num_intersections = 0; num_intersections < 8; num_intersections++)
				{
					float d =
						near_plane.x * vertices[num_intersections].x +
						near_plane.y * vertices[num_intersections].y +
						near_plane.z * vertices[num_intersections].z +
						near_plane.w;

					if (d >= 0.0f)
					{
						break;
					}
				}

				if (num_intersections < 8)
				{
					m_visible_inside_probes.push_back(probe);
				}
				else
				{
					m_visible_outside_probes.push_back(probe);
				}

				m_visible_probes.push_back(probe);
			}
		}
	}

	if (partitions_enabled)
	{
		m_cull_path.push_back(r);

		for (size_t i = 0; i < r->m_connections.size(); i++)
		{
			KCL::XRoomConnection *rc = r->m_connections[i];

			if (rc->m_portal)
			{
				if (rc->m_enabled == true)
				{

					std::vector<KCL::Vector3D> new_points1;
					std::vector<KCL::Vector3D> new_points2;
					std::vector<KCL::Vector4D> new_planes;
					KCL::XPortal *p = rc->m_portal;

					KCL::XRoom *other = rc->m_portal->Other(r);

					if (IsRoomInPath(other))
					{
						continue;
					}

					if (m_scene->m_pvs && !m_scene->m_pvs[m_camera_room->m_pvs_index][other->m_pvs_index])
					{
						continue;
					}

					new_points1 = p->m_points;

					for (KCL::uint32 k = 0; k < plane_count; k++)
					{
						const KCL::Vector4D &plane = planes[k];

						for (KCL::uint32 j = 0; j < new_points1.size(); j++)
						{
							KCL::Vector3D &P0 = new_points1[j];
							KCL::Vector3D &P1 = new_points1[j == new_points1.size() - 1 ? 0 : j + 1];

							if (Inside(P0, plane))
							{
								new_points2.push_back(P0);
								if (!Inside(P1, plane))
								{
									KCL::Vector3D rr;
									Intersect(P0, P1, plane, rr);
									new_points2.push_back(rr);
								}
							}
							else
							{
								if (Inside(P1, plane))
								{
									KCL::Vector3D rr;
									Intersect(P0, P1, plane, rr);
									new_points2.push_back(rr);
								}
							}
						}

						new_points1 = new_points2;
						new_points2.clear();
					}

					float eye_side = p->m_plane.x * m_camera.GetEye().x + p->m_plane.y * m_camera.GetEye().y + p->m_plane.z * m_camera.GetEye().z + p->m_plane.w;

					for (KCL::uint32 j = 0; j < new_points1.size(); j++)
					{
						KCL::Vector4D new_plane;
						KCL::XPortal::Edge e;
						KCL::Vector3D &P0 = new_points1[j];
						KCL::Vector3D &P1 = new_points1[j == new_points1.size() - 1 ? 0 : j + 1];

						e.CreateEdgeVector(P1, P0);
						e.CreatePlane(eye_side > 0.0f, m_camera.GetEye(), P0, new_plane);

						new_planes.push_back(new_plane);
					}

					if (new_planes.size())
					{
						CullRoom(other, new_planes.data(), (KCL::uint32)new_planes.size(), partitions_enabled);
					}
				}
			}
			else
			{
				KCL::XRoom *other = rc->Other(r);

				if (IsRoomInPath(other))
				{
					continue;
				}

				if (m_scene->m_pvs && !m_scene->m_pvs[m_camera_room->m_pvs_index][other->m_pvs_index])
				{
					continue;
				}

				KCL::OverlapResult result = KCL::XRoom::OVERLAP(planes, plane_count, &other->m_aabb);
				if (result != KCL::OVERLAP_OUTSIDE)
				{
					CullRoom(other, planes, plane_count, partitions_enabled);
				}
			}
		}

		m_cull_path.pop_back();
	}

	// Adjust camera near-far
	AdjustNearFar(r->m_aabb);
}


bool FrustumCull::IsRoomInPath(KCL::XRoom *r)
{
	for (KCL::uint32 i = 0; i < m_cull_path.size(); i++)
	{
		if (m_cull_path[i] == r)
		{
			return true;
		}
	}
	return false;
}


void FrustumCull::CullActors()
{
	for (size_t i = 0; i < m_scene->m_actors.size(); i++)
	{
		KCL::Actor *actor = m_scene->m_actors[i];

		if (m_excluded_actors.find(actor) != m_excluded_actors.end())
		{
			continue;
		}

		bool has_visible_mesh = false;

		if (m_camera.IsVisible(&actor->m_aabb))
		{
			for (size_t j = 0; j < actor->m_meshes.size(); j++)
			{
				KCL::Mesh *mesh = actor->m_meshes[j];

				KCL::int32 mesh_type_index = m_mesh_filter->FilterMesh(&m_camera, mesh, KCL::OVERLAP_INTERSECT);
				if (mesh_type_index == -1)
				{
					continue;
				}

				if (!mesh->m_visible)
				{
					continue;
				}

				has_visible_mesh = true;
				m_visible_meshes[mesh_type_index].push_back(mesh);
				if (m_instance_id > 1)
				{
					((Mesh*)mesh)->m_debug_frustum_ids[m_instance_id - 1].x = 1.0f;
				}
			}
			
			if (has_visible_mesh)
			{
				m_visible_actors++;
			}

			// Adjust camera near-far
			AdjustNearFar(actor->m_aabb);
		}

		if (m_cull_lights)
		{
			for (size_t j = 0; j < actor->m_lights.size(); j++)
			{
				Light *light = (Light*)actor->m_lights[j];
				if (light->m_visible)
				{
					CullLight(light);
				}
			}
		}
	}
}


void FrustumCull::CullLight(Light *l)
{
	if (l->m_frame_counter == m_frame_counter)
	{
		return;
	}
	l->m_frame_counter = m_frame_counter;

	bool has_aabb = true;
	if (l->m_light_shape->m_light_type == KCL::LightShape::DIRECTIONAL && l->m_light_shape->m_box_light == false)
	{
		has_aabb = false;
	}
	else if(m_camera.IsVisible(&l->m_aabb) == false)
	{
		return;
	}

	m_visible_lights.push_back(l);

	if (has_aabb)
	{
		AdjustNearFar(l->m_aabb);
	}

	if (l->m_has_lightshaft)
	{
		m_visible_light_shafts.push_back(l);
	}
}


bool FrustumCull::Inside(const KCL::Vector3D &p, const KCL::Vector4D &plane)
{
	return (plane.x * p.x + plane.y * p.y + plane.z * p.z + plane.w) <= 0.0f;
}


void FrustumCull::Intersect(const KCL::Vector3D &p0, const KCL::Vector3D &p1, const KCL::Vector4D &plane, KCL::Vector3D &result)
{
	float d0 = plane.x * p0.x + plane.y * p0.y + plane.z * p0.z + plane.w;
	float d1 = plane.x * p1.x + plane.y * p1.y + plane.z * p1.z + plane.w;

	float f = d0 / (d0 - d1);

	result = KCL::Vector3D::interpolate(p0, p1, f);
}


float FrustumCull::GetNear() const
{
	return m_near;
}


float FrustumCull::GetFar() const
{
	return m_far;
}


void FrustumCull::SetDefaultNearFar(float default_near, float default_far)
{
	m_default_near = default_near;
	m_default_far = default_far;
}


void FrustumCull::SetPartitionsEnabled(bool value)
{
	m_partition_enabled = value;
}


bool FrustumCull::IsPartitionsEnabled() const
{
	return m_partition_enabled;
}


void FrustumCull::SetPVSEnabled(bool value)
{
	m_pvs_enabled = value;
}


bool FrustumCull::IsPVSEnabled() const
{
	return m_pvs_enabled;
}


void FrustumCull::SetCullLights(bool value)
{
	m_cull_lights = value;
}


void FrustumCull::SetCullActors(bool value)
{
	m_cull_actors = value;
}


void FrustumCull::SetCullProbes(bool value)
{
	m_cull_probes = value;
}


void FrustumCull::SetCullWithNearFar(bool value)
{
	m_cull_with_near_far = value;
}


void FrustumCull::ExcludeActor(KCL::Actor *actor)
{
	m_excluded_actors.insert(actor);
}


bool FrustumCull::DepthCompare(const MeshSortInfo *A, const MeshSortInfo *B)
{
	// Front to back
	return A->m_vertex_center_dist < B->m_vertex_center_dist;
}


bool FrustumCull::DepthCompareAlphaTest(const MeshSortInfo *A, const MeshSortInfo *B)
{
	const bool a_alpha_tested = A->m_mesh->m_material->m_opacity_mode == KCL::Material::ALPHA_TEST;
	const bool b_alpha_tested = B->m_mesh->m_material->m_opacity_mode == KCL::Material::ALPHA_TEST;

	if (a_alpha_tested == b_alpha_tested)
	{
		// Front to back
		return A->m_vertex_center_dist < B->m_vertex_center_dist;
	}
	else
	{
		// Opaque meshes first
		return b_alpha_tested;
	}
}


bool FrustumCull::ReverseDepthCompare(const MeshSortInfo *A, const MeshSortInfo *B)
{
	// Back to front
	return A->m_vertex_center_dist > B->m_vertex_center_dist;
}


void FrustumCull::SortMeshes(std::vector<KCL::Mesh*> &meshes, PFN_MeshCompare sort_function)
{
	const size_t mesh_count = meshes.size();

	if (mesh_count > m_sort_meshes.size())
	{
		m_sort_meshes.resize(mesh_count);
		m_sort_mesh_pointers.resize(mesh_count);
	}

	Mesh *mesh;
	for (size_t i = 0; i < mesh_count; i++)
	{
		mesh = (Mesh*)meshes[i];

		MeshSortInfo &mesh_info = m_sort_meshes[i];
		mesh_info.m_mesh = mesh;

		// TODO: actors should be inserted to the result array directly
		if (mesh->m_owner && mesh->m_owner->m_type == KCL::ACTOR)
		{
			//KCL::Vector4D mesh_center(mesh->m_world_pom.v[12], mesh->m_world_pom.v[13], mesh->m_world_pom.v[14], 1.0);
			mesh_info.m_vertex_center_dist = KCL::Vector4D::dot(KCL::Vector4D(mesh->m_center, 1.0f), m_camera.GetCullPlane(KCL::CULLPLANE_NEAR));
		}
		else
		{
			mesh_info.m_vertex_center_dist = KCL::Vector4D::dot(KCL::Vector4D(mesh->m_vertexCenter), m_camera.GetCullPlane(KCL::CULLPLANE_NEAR));
		}
		m_sort_mesh_pointers[i] = &mesh_info;
	}

	// Sort the meshes
	std::vector<MeshSortInfo*>::iterator begin_it = m_sort_mesh_pointers.begin();

	std::stable_sort(begin_it, begin_it + mesh_count, sort_function);

	// Remap original visible meshes
	for (size_t i = 0; i < mesh_count; i++)
	{
		meshes[i] = m_sort_mesh_pointers[i]->m_mesh;
	}
}

bool FrustumCull::LightReverseDepthCompare(const FrustumCull::LightSortInfo *A, const FrustumCull::LightSortInfo *B)
{
	// Back to front
	return A->m_distance > B->m_distance;
}


void FrustumCull::SortLights(std::vector<KCL::Light*> &lights)
{
	const size_t light_count = lights.size();

	if (light_count > m_sort_lights.size())
	{
		m_sort_lights.resize(light_count);
		m_sort_light_pointers.resize(light_count);
	}

	KCL::Light *light;
	for (size_t i = 0; i < light_count; i++)
	{
		light = lights[i];

		LightSortInfo &light_info = m_sort_lights[i];
		light_info.m_light = light;
		light_info.m_distance = KCL::Vector4D::dot(KCL::Vector4D(((GFXB::Light*) light)->m_pos), m_camera.GetCullPlane(KCL::CULLPLANE_NEAR));

		m_sort_light_pointers[i] = &light_info;
	}

	// Sort the lights
	std::vector<LightSortInfo*>::iterator begin_it = m_sort_light_pointers.begin();

	std::stable_sort(begin_it, begin_it + light_count, LightReverseDepthCompare);

	// Remap lights
	for (size_t i = 0; i < light_count; i++)
	{
		lights[i] = m_sort_light_pointers[i]->m_light;
	}
}


void FrustumCull::ResetInstanceCounter()
{
	/*
	if (s_instance_counter != 0)
	{
		INFO("Error!!! FrustumCull: instance counter is not zero (%d)!", s_instance_counter);
	}
	*/
	s_instance_counter = 0;
}


KCL::int32 PassthroughMeshFilter::FilterMesh(KCL::Camera2 *camera, KCL::Mesh *mesh, KCL::OverlapResult overlap_result)
{
	return 0;
}


KCL::uint32 PassthroughMeshFilter::GetMaxMeshTypes()
{
	return 1;
}

FrustumCull::PFN_MeshCompare PassthroughMeshFilter::GetMeshSortFunction(KCL::uint32 mesh_type)
{
	return nullptr;
}
