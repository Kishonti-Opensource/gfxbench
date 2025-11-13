/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mesh_tool.h"
#include "kcl_mesh.h"
#include "kcl_light2.h"

#include <algorithm>

using namespace KCL;

KCL::Vector3D MeshTool::CalculateRayDirection(KCL::Camera2 *camera,  float viewport_width, float viewport_height, float x, float y)
{
	// Camera coordinate system base vectors
	KCL::Vector3D view = camera->GetCenter() - camera->GetEye();
	view.normalize();
	KCL::Vector3D h = KCL::Vector3D::cross(view, camera->GetUp()); // horizontal
	h.normalize();
	KCL::Vector3D v = KCL::Vector3D::cross(h, view); // vertical
	v.normalize();

	// Normalize with the view plane
	float vLength = tanf(KCL::Math::Rad(camera->GetFov()) / 2.0f) * camera->GetNear();
	float hLength = vLength * camera->GetAspectRatio();
	v = v * vLength;
	h = h * hLength;

	// NDC
	float mx = x - (viewport_width / 2.0f);
	float my = (viewport_height - y) - (viewport_height / 2.0f);
	mx = mx / (viewport_width / 2.0f);
	my = my / (viewport_height / 2.0f);

	// Calculate the ray direction
	KCL::Vector3D pos = camera->GetEye() + view * camera->GetNear() + h * mx + v * my;
	KCL::Vector3D dir = pos - camera->GetEye();
	dir.normalize();

	return dir;
}

inline KCL::Vector3D VectorMin(const KCL::Vector3D &a, const KCL::Vector3D &b)
{
	return KCL::Vector3D(
		a.x < b.x ? a.x : b.x,
		a.y < b.y ? a.y : b.y,
		a.z < b.z ? a.z : b.z);
}

inline KCL::Vector3D VectorMax(const KCL::Vector3D &a, const KCL::Vector3D &b)
{
	return KCL::Vector3D(
		a.x > b.x ? a.x : b.x,
		a.y > b.y ? a.y : b.y,
		a.z > b.z ? a.z : b.z);
}

bool IntersectRayAABB(const KCL::Vector3D &ray_org, const KCL::Vector3D &ray_dir, const KCL::Vector3D &aabb_min,const KCL::Vector3D &aabb_max, float &near_dist, float &far_dist)
{
	KCL::Vector3D dirInv(1.0f / ray_dir.x, 1.0f / ray_dir.y, 1.0f / ray_dir.z);

	KCL::Vector3D tnear4 = dirInv * (aabb_min - ray_org);
	KCL::Vector3D tfar4 = dirInv * (aabb_max - ray_org);

	KCL::Vector3D t0 = VectorMin(tnear4, tfar4);
	KCL::Vector3D t1 = VectorMax(tnear4, tfar4);

	near_dist = KCL::Max(KCL::Max(t0.x, t0.y), t0.z);
	far_dist = KCL::Min(KCL::Min(t1.x, t1.y), t1.z);

	return (far_dist >= near_dist) && (far_dist > 0.0f);
}

// Moller-Trumbore intersection
bool IntersectRayTriangle(const KCL::Vector3D &ray_org, const KCL::Vector3D &ray_dir, const KCL::Vector3D &a, const KCL::Vector3D &b, const KCL::Vector3D &c, KCL::Vector3D &interpolator, bool cull_face)
{
	typedef KCL::Vector3D Vec3;
	Vec3 e1, e2;
	Vec3 P;
	Vec3 Q;
	Vec3 T;
	float det, inv_det, u, v;
	float t;

	float EPSILON = 0.00001f;

	//Find vectors for two edges sharing V1
	e1 = b - a;
	e2 = c - a;

	KCL::Vector3D normal = KCL::Vector3D::cross(e1, e2);

	//calculate distance from V1 to ray origin
	T = ray_org - a;

	// Face cull
	if (cull_face)
	{
		if (KCL::Vector3D::dot(normal, T) < 0.0f)
		{
			return false;
		}
	}

	//Begin calculating determinant - also used to calculate u parameter
	P = KCL::Vector3D::cross(e2, ray_dir);
	//if determinant is near zero, ray lies in plane of triangle
	det = KCL::Vector3D::dot(e1, P);

	if (det > -EPSILON && det < EPSILON)
	{
		return false;
	}

	inv_det = 1.f / det;


	//Calculate u parameter and test bound
	u = KCL::Vector3D::dot(T, P) * inv_det;
	//The intersection lies outside of the triangle
	if (u < 0.f || u > 1.f)
	{
		return false;
	}

	//Prepare to test v parameter
	Q = KCL::Vector3D::cross(e1, T);

	//Calculate V parameter and test bound
	v = KCL::Vector3D::dot(ray_dir, Q)*inv_det;
	//The intersection lies outside of the triangle
	if (v < 0.f || u + v  > 1.f)
	{
		return false;
	}

	t = KCL::Vector3D::dot(e2,Q)*inv_det;
	if (t > EPSILON)
	{
		interpolator.x = t;
		return true;
	}

	return false;
}


bool MeshTool::calcQuadratic(const float &a, const float &b, const float &c, float &x0, float &x1)
{
	float discriminant = b * b - 4 * a * c;
	if (discriminant < 0)
	{
		return false;
	}
	else if (discriminant == 0)
	{
		x0 = x1 = -0.5f * b / a;
	}
	else
	{
		float q = (b > 0) ? -0.5f * (b + sqrt(discriminant)) : -0.5f * (b - sqrt(discriminant));
		x0 = q / a;
		x1 = c / q;
	}
	if (x0 > x1)
	{
		std::swap(x0, x1);
	}

	return true;
}


KCL::Node* MeshTool::PickLightOrEmitter(PickMeshArg &pick_arg)
{
	// Create the world space ray
	KCL::Vector3D ray_from = pick_arg.camera->GetEye();

	if (pick_arg.viewport.x < pick_arg.pick_pos.x ||
		pick_arg.viewport.y < pick_arg.pick_pos.y ||
		pick_arg.pick_pos.x < 0 ||
		pick_arg.pick_pos.y < 0)
	{
		return NULL;
	}

	KCL::Vector3D ray_dir = CalculateRayDirection(pick_arg.camera, pick_arg.viewport.x, pick_arg.viewport.y, pick_arg.pick_pos.x, pick_arg.pick_pos.y);
	//KCL::Vector3D ray_to = ray_from + ray_dir;
	for (size_t i = 0; i < pick_arg.lights.size(); ++i)
	{
		float radius2 = pick_arg.lights[i]->m_light_shape->m_radius * pick_arg.lights[i]->m_light_shape->m_radius;
		if (radius2 < 0)
		{
			continue;
		}
		radius2 = 1;//HACK
		KCL::Vector3D center(pick_arg.lights[i]->m_world_pom.v41, pick_arg.lights[i]->m_world_pom.v42, pick_arg.lights[i]->m_world_pom.v43);
		if (PickSphere(ray_from, ray_dir, center, radius2))
		{
			//printf("intersected light %s\n", pick_arg.lights[i]->m_name.c_str());//debug
			return pick_arg.lights[i];
		}
	}
	for (size_t i = 0; i < pick_arg.emitters.size(); ++i)
	{
		float radius2 = 9;
		KCL::Vector3D center(pick_arg.emitters[i]->m_world_pom.v41, pick_arg.emitters[i]->m_world_pom.v42, pick_arg.emitters[i]->m_world_pom.v43);
		if (PickSphere(ray_from, ray_dir, center, radius2))
		{
			//printf("intersected emitter %s\n", pick_arg.emitters[i]->m_name.c_str());//debug
			return pick_arg.emitters[i];
		}
	}
	return nullptr;
}


KCL::Mesh* MeshTool::PickMesh(PickMeshArg &pick_arg)
{
	// Create the world space ray
	KCL::Vector3D ray_from = pick_arg.camera->GetEye();

	if (pick_arg.viewport.x < pick_arg.pick_pos.x || pick_arg.viewport.y < pick_arg.pick_pos.y ||
		pick_arg.pick_pos.x < 0 || pick_arg.pick_pos.y < 0)
	{
		return NULL;
	}

	KCL::Vector3D ray_dir = CalculateRayDirection(pick_arg.camera, pick_arg.viewport.x, pick_arg.viewport.y, pick_arg.pick_pos.x, pick_arg.pick_pos.y);
	KCL::Vector3D ray_to = ray_from + ray_dir;

	KCL::Mesh *result_mesh = NULL;
	float distance = 0.0f;
	float min_distance = FLT_MAX;
	for (KCL::uint32 i = 0; i < pick_arg.meshes.size(); i++)
	{
		KCL::Mesh* mesh = pick_arg.meshes[i];
		if (IntersectMesh(ray_from, ray_to, ray_dir, mesh, distance) && distance < min_distance)
		{
			min_distance = distance;
			result_mesh = mesh;
		}
	}

	for (KCL::uint32 i = 0; i < pick_arg.mesh_instance.size(); i++)
	{
		for (KCL::uint32 j = 0; j < pick_arg.mesh_instance[i].size(); j++)
		{
			KCL::Mesh* mesh = pick_arg.mesh_instance[i][j];
			if (IntersectMesh(ray_from, ray_to, ray_dir, mesh, distance) && distance < min_distance)
			{
				min_distance = distance;
				result_mesh = mesh;
			}
		}
	}

	return result_mesh;
}


bool MeshTool::IntersectMesh(const KCL::Vector3D &ray_from, const KCL::Vector3D &ray_to, const KCL::Vector3D &ray_dir, KCL::Mesh *mesh, float &distance)
{
	distance = FLT_MAX;

	// Transform the ray to model space
	KCL::Matrix4x4 inv_model;
	KCL::Matrix4x4::Invert4x4( mesh->m_world_pom, inv_model);
	KCL::Vector3D model_ray_from = KCL::Vector3D(inv_model * KCL::Vector4D(ray_from, 1.0f));
	KCL::Vector3D model_ray_to = KCL::Vector3D(inv_model * KCL::Vector4D(ray_to, 1.0f));
	KCL::Vector3D model_ray_dir = model_ray_to - model_ray_from;
	model_ray_dir.normalize();

	// Intersect the model's AABB
	float near_dist, far_dist;
	if (!IntersectRayAABB(ray_from, ray_dir, mesh->m_aabb.GetMinVertex(), mesh->m_aabb.GetMaxVertex(), near_dist, far_dist))
	{
		return false;
	}

	bool intersect = false;
	KCL::Vector3D interpolator;

	const std::vector<KCL::Vector3D> &vertices = mesh->m_mesh->m_vertex_attribs3[0];
	const std::vector<KCL::uint16> &indices = mesh->m_mesh->m_vertex_indices[0];
	const size_t index_count = (mesh->m_mesh->getIndexCount(0) / 3) * 3; // Round the number of indices so it will work with patches too (somewhat)

	bool cull_face = true;
	if (mesh->m_material->m_material_type == KCL::Material::FOLIAGE)
	{
		cull_face = false;
	}

	for (size_t j = 0; j < index_count; j = j + 3)
	{
		if (IntersectRayTriangle(model_ray_from, model_ray_dir,
			vertices[indices[j + 0]],
			vertices[indices[j + 1]],
			vertices[indices[j + 2]],
			interpolator, cull_face))
		{
			// Move the intersection point to world space and calculate the distance
			KCL::Vector3D intersection_model = model_ray_from + model_ray_dir * interpolator.x;
			KCL::Vector3D intersection_world = KCL::Vector3D(mesh->m_world_pom * KCL::Vector4D(intersection_model, 1.0f));

			float triangle_distance = KCL::Vector3D::distance2(ray_from, intersection_world);

			if (triangle_distance < distance)
			{
				distance = triangle_distance;
				intersect = true;
			}
		}
	}
	return intersect;
}


bool MeshTool::PickSphere(KCL::Vector3D &ray_from, KCL::Vector3D &ray_dir, KCL::Vector3D &center, float radius2, float *distance)
{
	float t0, t1;
	KCL::Vector3D L = ray_from - center;
	float a = KCL::Vector3D::dot(ray_dir, ray_dir);
	float b = 2 * KCL::Vector3D::dot(ray_dir, L);
	float c = KCL::Vector3D::dot(L, L) - radius2;
	if (!calcQuadratic(a, b, c, t0, t1))
	{
		return false;
	}
	if (distance)
	{
		*distance = c;
	}
	return true;
}

/*!
	Return the position of the nearest selected grid point.
	Return FLT_MAX when there is no matching.
*/
int KCL::MeshTool::PickByPositions(KCL::PickMeshArg pick_arg)
{
	// Create the world space ray
	KCL::Vector3D ray_from = pick_arg.camera->GetEye();

	if (pick_arg.viewport.x < pick_arg.pick_pos.x ||
		pick_arg.viewport.y < pick_arg.pick_pos.y ||
		pick_arg.pick_pos.x < 0 ||
		pick_arg.pick_pos.y < 0)
	{
		return -1;
	}

	KCL::Vector3D ray_dir = CalculateRayDirection(pick_arg.camera, pick_arg.viewport.x, pick_arg.viewport.y, pick_arg.pick_pos.x, pick_arg.pick_pos.y);

	float nearest = FLT_MAX;
	KCL::Vector3D nearest_vector(FLT_MAX, FLT_MAX, FLT_MAX);
	int nearest_index = -1;

	for (int i = 0; i < pick_arg.grid_positions.size(); i++)
	{
		float dist = 0;
		float radius2 = 0.07f;

		KCL::Vector3D center(pick_arg.grid_positions[i]);
		if (PickSphere(ray_from, ray_dir, center, radius2, &dist))
		{
			if (dist < nearest)
			{
				nearest = dist;
				nearest_vector = center;
				nearest_index = i;
			}
		}
	}
	if (nearest < FLT_MAX)
	{
		//printf("nearest intersected position %0.2f %0.2f %0.2f dist=%0.2f\n", nearest_vector.x, nearest_vector.y, nearest_vector.z, nearest);//debug
	}
	return nearest_index;
}
