/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_scene_40_support.h"
#include "mtl_scene_40.h"
#include <kcl_mesh.h>
#include "mtl_material4.h"

#include <algorithm>

#include "ng/json.h"
#include "ng/log.h"


using namespace KCL;

namespace GFXB4
{
id<MTLTexture> CreateRenderTarget(id<MTLDevice> device, KCL::uint32 width, KCL::uint32 height, MTLPixelFormat format)
{
	MTLTextureDescriptor * tex_desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format
																						 width:width
																						height:height
																					 mipmapped:NO];

	tex_desc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
#if !TARGET_OS_EMBEDDED
	tex_desc.storageMode = MTLStorageModePrivate;
#endif

	id<MTLTexture> texture = [device newTextureWithDescriptor:tex_desc];
	releaseObj(tex_desc);

	return texture;
}
};

ParaboloidCulling::ParaboloidCulling(KCL::uint32 size_x, KCL::uint32 size_y, float limit) : CullingAlgorithm(CA_REFL)
{
    m_size_x = float(size_x);
    m_size_y = float(size_y);
    m_limit = limit;
}

void ParaboloidCulling::SetLimit(float limit)
{
   m_limit = limit;
}

bool ParaboloidCulling::CullMesh(KCL::Camera2 *camera, KCL::Mesh *mesh)
{
    if (!mesh->m_owner || mesh->m_owner->m_type != KCL::ROOM)
    {
        // Currently only implemented for static meshes
        return false;
    }

    // TODO: precalculate these for static meshes !!!
    KCL::Vector3D center;
    KCL::Vector3D size;
    mesh->m_aabb.CalculateHalfExtentCenter(size, center);
    float radius = mesh->m_aabb.CalculateRadius();
    
    // Transform the center to view space
    Vector4D vs_center = camera->GetView() * KCL::Vector4D(center, 1.0f);

    // Get 2 bounding view-space point
    Vector3D vs_a(vs_center.x + radius, vs_center.y + radius, vs_center.z + radius);
    Vector3D vs_b(vs_center.x - radius, vs_center.y - radius, vs_center.z - radius);

    // Paraboloid projection for A
    vs_a.normalize();
    vs_a.z = vs_a.z + 1.0f;
    vs_a.x = vs_a.x / vs_a.z;
    vs_a.y = vs_a.y / vs_a.z;

    // Paraboloid projection for B
    vs_b.normalize();
    vs_b.z = vs_b.z + 1.0f;
    vs_b.x = vs_b.x / vs_b.z;
    vs_b.y = vs_b.y / vs_b.z;

    // Project the distance vector to view port space
    float size_x = fabsf(vs_a.x - vs_b.x) * m_size_x;
    float size_y = fabsf(vs_a.y - vs_b.y) * m_size_y;
    
    return size_x < m_limit || size_y < m_limit;
}

PerspectiveCulling::PerspectiveCulling(float limit) : CullingAlgorithm()
{
    m_limit = limit;
}

bool PerspectiveCulling::CullMesh(KCL::Camera2 *camera, KCL::Mesh *mesh)
{
    if (!mesh->m_owner || mesh->m_owner->m_type != KCL::ROOM)
    {
        // Currently only implemented for static meshes
        return false;
    }

    // Now test for near-plane projected size
    float projSize = 10000000.0f;

    // TODO: precalculate these for static meshes !!!
    float rad = mesh->m_aabb.CalculateRadius();
    KCL::Vector3D center;
    KCL::Vector3D size;
    mesh->m_aabb.CalculateHalfExtentCenter(size, center);

    float nearPlaneDist = Vector4D::dot( KCL::Vector4D(center, 1.0f), camera->GetCullPlane(KCL::CULLPLANE_NEAR));
    if(nearPlaneDist > 0.0f)
    {
        projSize = rad * camera->GetNear() / nearPlaneDist;
        return projSize < m_limit;
    }
    return false;
}


bool MTL_Scene4Tools::displacement_depth_compare(const mesh_sort_info* A, const mesh_sort_info* B)
{
    bool a_tesselated = A->mesh->m_material->m_displacement_mode != KCL::Material::NO_DISPLACEMENT;
    if (a_tesselated == (B->mesh->m_material->m_displacement_mode != KCL::Material::NO_DISPLACEMENT))
    {
        // Both are tesselated or not
        return A->vertexCenterDist < B->vertexCenterDist;
    }
    // One of them is tesselated
    return a_tesselated; 

}

bool MTL_Scene4Tools::depth_compare(const mesh_sort_info* A, const mesh_sort_info* B)
{
    // Front to back
    return A->vertexCenterDist < B->vertexCenterDist;
}

bool MTL_Scene4Tools::reverse_depth_compare(const mesh_sort_info* A, const mesh_sort_info* B)
{
    // Back to front
    return A->vertexCenterDist > B->vertexCenterDist; 
}

bool MTL_Scene4Tools::material_compare(const mesh_sort_info* A, const mesh_sort_info* B)
{
    return A->sort_order < B->sort_order;
}

// Material sor
bool MTL_Scene4Tools::reverse_material_compare(const mesh_sort_info* A, const mesh_sort_info* B)
{
    if (A->sort_order != 0 && B->sort_order != 0)
    {
        return A->sort_order < B->sort_order;
    }
    return A->sort_order == 0;
}

bool MTL_Scene4Tools::material_depth_compare(const mesh_sort_info* A, const mesh_sort_info* B)
{
    if (A->sort_order == B->sort_order)
    {
        //Depth sort
        return A->vertexCenterDist < B->vertexCenterDist;
}
    return A->sort_order < B->sort_order;
}

std::vector<MTL_Scene4Tools::mesh_sort_info> MTL_Scene4Tools::sorted_meshes;
std::vector<MTL_Scene4Tools::mesh_sort_info*> MTL_Scene4Tools::sort_pointers;

void MTL_Scene4Tools::SortMeshes(std::vector<KCL::Mesh*> &visible_meshes, const KCL::Camera2 *camera, bool alpha_blend)
{
    //put sky to the end of the mesh list, ensuring it renders last
    /*
    KCL::Mesh* sky = NULL;
    if(visible_meshes.back()->m_material->m_material_type == Material::SKY)
    {
        sky = visible_meshes.back();
        visible_meshes.pop_back();
    }
    */

    // We use a common static helper vector to prevent memory allocations
    const KCL::uint32 mesh_count = visible_meshes.size();
    if (mesh_count < 2)
    {
        return;
    }

    sorted_meshes.resize(mesh_count);
    sort_pointers.resize(mesh_count);

    KCL::Mesh *mesh;
    for (uint32 i = 0; i < mesh_count; i++)
    {
        mesh = visible_meshes[i];

        mesh_sort_info &mesh_info = sorted_meshes[i];
        mesh_info.mesh = mesh;
        if (mesh->m_materials[RENDER_MATERIAL_ID])
        {
            mesh_info.sort_order = ((MetalRender::Material4*)mesh->m_materials[RENDER_MATERIAL_ID])->m_sort_order;
        }
        else
        {
			mesh_info.sort_order = ((MetalRender::Material4*)mesh->m_material)->m_sort_order;
        }

        if (mesh->m_owner && mesh->m_owner->m_type == KCL::ACTOR)
        {
            // Force actors to be drawn first
            mesh_info.vertexCenterDist = 0.0f;
        }
        else
        {
            mesh_info.vertexCenterDist = Vector4D::dot( KCL::Vector4D(mesh->m_vertexCenter), camera->GetCullPlane(KCL::CULLPLANE_NEAR));
        }
        sort_pointers[i] = &mesh_info;
    }

    // Sort the meshes
    std::vector<mesh_sort_info*>::iterator begin_it = sort_pointers.begin();
   
    if (alpha_blend)
    {
		std::stable_sort (begin_it, begin_it + mesh_count, &MTL_Scene4Tools::reverse_depth_compare);
    }
    else
    {
         //std::sort (begin_it, begin_it + mesh_count, &MTL_Scene4Tools::displacement_depth_compare);
		std::stable_sort (begin_it, begin_it + mesh_count, &MTL_Scene4Tools::material_depth_compare);
    }
    
    // Remap original visible meshes
    for (uint32 i = 0; i < mesh_count; i++)
    {
        visible_meshes[i] = sort_pointers[i]->mesh;
    }    
    /*
    if(sky)
    {
        visible_meshes.push_back(sky);
    }
    */
}


#define USE_TIME_QUERY 0


KCL::Vector3D MTL_Scene4Tools::CalculateRayDirection(KCL::Camera2 *camera,  float viewport_width, float viewport_height, float x, float y)
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

// Barycentric ray-triangle intersection
bool IntersectRayTriangle(const KCL::Vector3D &ray_org, const KCL::Vector3D &ray_dir, const KCL::Vector3D &a, const KCL::Vector3D &b, const KCL::Vector3D &c, KCL::Vector3D &interpolator)
{
    KCL::Vector3D edge1, edge2;
    KCL::Vector3D tvec, pvec, qvec;
    float det, inv_det;

    edge1 = b - a;
    edge2 = c - a;

    // Face cull
    KCL::Vector3D normal = KCL::Vector3D::cross(edge1, edge2);
    tvec = ray_org - a;
    if (KCL::Vector3D::dot(normal, tvec) < 0.0f)
    {
        return false;
    }

    pvec = KCL::Vector3D::cross(ray_dir, edge2);
    det = KCL::Vector3D::dot(edge1, pvec);

    if (det > -0.000001f && det < 0.000001f)
    {
        // The ray is paralell with the triangle
        return false;
    }

    inv_det = 1.0f / det;
    interpolator.y = KCL::Vector3D::dot(tvec, pvec);
    interpolator.y *= inv_det;
    if (interpolator.y < 0.0f || interpolator.y > 1.0f)
    {
        // No intersection
        return false;
    }

    qvec = KCL::Vector3D::cross(tvec, edge1);
    interpolator.z = KCL::Vector3D::dot(ray_dir, qvec);
    interpolator.z *= inv_det;    
    if( interpolator.z < 0.0f || interpolator.y + interpolator.z > 1.0f)
    {
        // No intersection
        return false;
    }

    // Calculate the interpolation factor from the ray pos to the intersection point
    // intersection = ray_org + ray_dir * interpolator.x
    interpolator.x = KCL::Vector3D::dot(edge2, qvec);
    interpolator.x *= inv_det;
    return true;
}

KCL::Mesh* MTL_Scene4Tools::PickMesh(MTL_Scene_Base *scene, KCL::Camera2 *camera, float x, float y)
{
    // Create the world space ray
    KCL::Vector3D ray_from = camera->GetEye();
    KCL::Vector3D ray_dir = CalculateRayDirection(camera, scene->m_viewport_width, scene->m_viewport_height, x, y);
    KCL::Vector3D ray_to = ray_from + ray_dir * 20000.0f;

    KCL::Mesh *result_mesh = NULL;
    float distance = 0.0f;
    float min_distance = FLT_MAX;
    for (KCL::uint32 mesh_type = 0; mesh_type < 3; mesh_type++)
    {
        for (KCL::uint32 i = 0; i < scene->m_visible_meshes[mesh_type].size(); i++)
        {
            KCL::Mesh* mesh = scene->m_visible_meshes[mesh_type][i];
            if (IntersectMesh(ray_from, ray_to, ray_dir, mesh, distance) && distance < min_distance)
            {
                min_distance = distance;
                result_mesh = mesh;
            }
        }   
    }

    for (KCL::uint32 i = 0; i < scene->m_visible_instances.size(); i++)
    {
        for (KCL::uint32 j = 0; j < scene->m_visible_instances[i].size(); j++)
        {
            KCL::Mesh* mesh = scene->m_visible_instances[i][j];
            if (IntersectMesh(ray_from, ray_to, ray_dir, mesh, distance) && distance < min_distance)
            {
                min_distance = distance;
                result_mesh = mesh;                
            }
        } 
    }

    return result_mesh;
}

bool MTL_Scene4Tools::IntersectMesh(const KCL::Vector3D &ray_from, const KCL::Vector3D &ray_to, const KCL::Vector3D &ray_dir, KCL::Mesh *mesh, float &distance)
{
    distance = FLT_MAX;

    // Transform the ray to model space         
    const KCL::Matrix4x4 &model = mesh->m_world_pom;
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
    KCL::Vector3D intersection;
    KCL::Vector3D intersection_model;
    const std::vector<KCL::Vector3D> &vertices = mesh->m_mesh->m_vertex_attribs3[0];
    const std::vector<KCL::uint16> &indices = mesh->m_mesh->m_vertex_indices[0];
    const int index_count = (mesh->m_mesh->getIndexCount(0) / 3) * 3; // Round the number of indices so it will work with patches too (somewhat)
    for (KCL::uint32 j = 0; j < index_count; j = j + 3)
    {
        if (IntersectRayTriangle(model_ray_from, model_ray_dir,
            vertices[indices[j + 0]],
            vertices[indices[j + 1]],
            vertices[indices[j + 2]],
            interpolator))
        {
            // Translate the model space intersection to world space
            intersection_model = model_ray_from + model_ray_dir * interpolator.x;
            intersection = KCL::Vector3D(model * KCL::Vector4D(intersection_model, 1.0f));
            // Test the intersection distance
            float dist = KCL::Vector3D::distance(ray_from, intersection);       
            if (dist < distance)
            {
                distance = dist;
                intersect = true;
            }           
        }
    }
    return intersect;
}


WorkGroupValidator::WGConfig::WGConfig(KCL::uint32 size_x, KCL::uint32 size_y, KCL::uint32 size_z)
{
	Set(size_x, size_y, size_z);
}

WorkGroupValidator::WGConfig::~WGConfig()
{
}

void WorkGroupValidator::WGConfig::Set(KCL::uint32 size_x, KCL::uint32 size_y, KCL::uint32 size_z)
{
	this->size_x = size_x;
	this->size_y = size_y;
	this->size_z = size_z;
}


WorkGroupValidator::WorkGroupValidator(id<MTLDevice> device)
{
	MTLSize maxThreadsPerThreadgroup = [device maxThreadsPerThreadgroup];

	m_max_ws_size_x = maxThreadsPerThreadgroup.width;
	m_max_ws_size_y = maxThreadsPerThreadgroup.height;
	m_max_ws_size_z = maxThreadsPerThreadgroup.depth;

#if TARGET_OS_EMBEDDED
	m_max_invocations = 512;
	m_max_shared_memory = 16384;
#else
	m_max_invocations = 1024;
	m_max_shared_memory = 32768;
#endif
}

WorkGroupValidator::~WorkGroupValidator()
{
}

bool WorkGroupValidator::Validate(const WGConfig &config) const
{
	return Validate(config.size_x, config.size_y, config.size_z);
}

bool WorkGroupValidator::Validate(const std::vector<WorkGroupValidator::WGConfig> &configs, std::vector<WorkGroupValidator::WGConfig> &result) const
{
	result.clear();
	for (KCL::uint32 i = 0; i < configs.size(); i++)
	{
		if (Validate(configs[i]))
		{
			result.push_back(configs[i]);
		}
	}
	return !result.empty();
}

bool WorkGroupValidator::Validate(KCL::uint32 size_x, KCL::uint32 size_y, KCL::uint32 size_z) const
{
	if (size_x > m_max_ws_size_x)
	{
		return false;
	}
	if (size_y > m_max_ws_size_y)
	{
		return false;
	}
	if (size_z > m_max_ws_size_z)
	{
		return false;
	}

	return size_x * size_y * size_z <= m_max_invocations;
}


const std::string WarmUpHelper::OCCLUSION_CULL = "occ";
const std::string WarmUpHelper::MOTION_BLUR_TILE_MAX = "mb_tile_max";
const std::string WarmUpHelper::MOTION_BLUR_NEIGHBOR_MAX = "mb_tile_neighbor";
const std::string WarmUpHelper::PARTICLE_SYSTEM_EMIT = "particle_system_emit";
const std::string WarmUpHelper::PARTICLE_SYSTEM_SIMULATE = "particle_system_simulate";
WarmUpHelper::WarmUpHelper(id<MTLDevice> device, MTL_Scene_40 *scene, const std::string &wg_sizes)
	: m_validator(device)
{
	m_scene = scene;
	m_start_time = 0;

	if (wg_sizes.empty())
	{
		return;
	}

	// Parse the workgroup size string
	std::stringstream ss(wg_sizes);
	std::string values;
	while (std::getline(ss, values, ','))
	{
		std::stringstream ss2(values);
		std::string name;
		KCL::uint32 wg_size_x;

		if (!(ss2 >> name))
		{
			break;
		}
		if (!(ss2 >> wg_size_x))
		{
			break;
		}

		WarmUpConfig *cfg = new WarmUpConfig();
		cfg->m_wg_config.size_x = wg_size_x;

		SetConfig(name, cfg);
	}
}

WarmUpHelper::~WarmUpHelper()
{
	std::map<std::string, WarmUpHelper::WarmUpConfig*>::iterator it;
	for (it = m_configs.begin(); it != m_configs.end(); it++)
	{
		delete it->second;
	}
	m_configs.clear();
}

WarmUpHelper::WarmUpConfig *WarmUpHelper::GetConfig(const std::string &name)
{
	std::map<std::string, WarmUpHelper::WarmUpConfig*>::iterator it = m_configs.find(name);
	if (it == m_configs.end())
	{
		return NULL;
	}
	return it->second;
}

void WarmUpHelper::SetConfig(const std::string &name, WarmUpHelper::WarmUpConfig *config)
{
	WarmUpConfig *_cfg = GetConfig(name);
	if (_cfg)
	{
		delete _cfg;
	}
	m_configs[name] = config;
}

void WarmUpHelper::BeginTimer()
{
	m_start_time = KCL::uint64(KCL::g_os->GetTimeMilliSec());
}

KCL::uint64 WarmUpHelper::EndTimer()
{
	KCL::uint64 now = KCL::uint64(KCL::g_os->GetTimeMilliSec());
	return now - m_start_time;
}
