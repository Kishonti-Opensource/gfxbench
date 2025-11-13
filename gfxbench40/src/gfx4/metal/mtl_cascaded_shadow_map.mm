/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_cascaded_shadow_map.h"
#include "mtl_scene_40.h"
#include "platform.h"


#include <kcl_room.h> // Frustum culling
#include <algorithm> // sort
#include <cfloat> // FLT_MAX, FLT_MIN

using namespace MetalRender;

#define SHADOW_NEAR		0.1f
#if (CASCADE_COUNT == 4)
    #define SHADOW_FAR		220.0f
#else
    #define SHADOW_FAR      50.0f
#endif

CascadedShadowMap::CascadedShadowMap(id<MTLDevice> device, KCL::uint32 map_size) :
	m_device(device),
    m_map_size(map_size),
    m_frustum_cull_counter(1),
    m_frame_counter(1)
{
    // The initial range of the shadow from the light point of view
    // TODO:Ajdust this
    m_default_shadow_range = 150.0f;

    // Cull small actors. Size in texels. Zero means no culling
    m_mesh_cull_size[0] = 0.0f;
    m_mesh_cull_size[1] = 0.0f;
#if (CASCADE_COUNT == 4)
    m_mesh_cull_size[2] = 7.0f;
    m_mesh_cull_size[3] = 11.0f;
#else
    m_mesh_cull_size[2] = 10.0f;
#endif

    SetCascade(0, 0.0f, KCL::Vector3D(1, 0, 0));
    SetCascade(1, 3.8f, KCL::Vector3D(0, 1, 0));
    SetCascade(2, 15.2f, KCL::Vector3D(0, 0, 1));
#if (CASCADE_COUNT == 4)
    SetCascade(3, 45.3f, KCL::Vector3D(1, 0, 1));
#endif

    m_fit_mode = FIT_OBB;

    m_camera = NULL;

    m_depth_component_format = MTLPixelFormatInvalid;
    m_texture_array = nil;

    memset(m_fbos, 0, sizeof(m_fbos));

    CreatePoissonDisk();
}

CascadedShadowMap::~CascadedShadowMap()
{
	releaseObj(m_texture_array);
	releaseObj(m_depth_sampler);
	releaseObj(m_shadow_sampler);
}

void CascadedShadowMap::SetCascade(KCL::uint32 index, float near_distance, const KCL::Vector3D &color)
{
    m_frustums[index].near_distance = near_distance;
    m_frustums[index].color = KCL::Vector4D(color);

    if (index == CASCADE_COUNT - 1)
    {
        // Finalize values
        float overlap_factor = 1.0005f;
        overlap_factor = 1.0f;
        for (KCL::uint32 i = 0; i < CASCADE_COUNT - 1; i++)
        {
            // Set the far distance
            m_frustums[i].far_distance = m_frustums[i + 1].near_distance * overlap_factor;
        }

        // Ensure constraints
        m_frustums[0].near_distance = 0.0f;
        m_frustums[CASCADE_COUNT - 1].far_distance = SHADOW_FAR;
    }
}

void CascadedShadowMap::Init(KCL::uint32 width, KCL::uint32 height, MTLPixelFormat depth_component_format)
{
    m_viewport_width = width;
    m_viewport_height = height;
    m_depth_component_format = depth_component_format;

    // PCF shadow sampler
	MTLSamplerDescriptor * shadow_sampler_desc = [[MTLSamplerDescriptor alloc] init];

	shadow_sampler_desc.sAddressMode = MTLSamplerAddressModeClampToEdge;
	shadow_sampler_desc.tAddressMode = MTLSamplerAddressModeClampToEdge;
	shadow_sampler_desc.minFilter = MTLSamplerMinMagFilterLinear;
	shadow_sampler_desc.magFilter = MTLSamplerMinMagFilterLinear;
	shadow_sampler_desc.compareFunction = MTLCompareFunctionLessEqual;

	m_shadow_sampler = [m_device newSamplerStateWithDescriptor:shadow_sampler_desc];
	releaseObj(shadow_sampler_desc);


	// Sampler for getting the depth values directly
	MTLSamplerDescriptor * depth_sampler_desc = [[MTLSamplerDescriptor alloc] init];

	depth_sampler_desc.sAddressMode = MTLSamplerAddressModeClampToEdge;
	depth_sampler_desc.tAddressMode = MTLSamplerAddressModeClampToEdge;
	depth_sampler_desc.minFilter = MTLSamplerMinMagFilterNearest;
	depth_sampler_desc.magFilter = MTLSamplerMinMagFilterNearest;

	m_depth_sampler = [m_device newSamplerStateWithDescriptor:depth_sampler_desc];
	releaseObj(depth_sampler_desc);

	// Create the shadow array texture
	MTLTextureDescriptor * tex_desc = [[MTLTextureDescriptor alloc] init];

	tex_desc.textureType = MTLTextureType2DArray;
	tex_desc.pixelFormat = depth_component_format;
	tex_desc.width = m_map_size;
	tex_desc.height = m_map_size;
	tex_desc.mipmapLevelCount = 1;
	tex_desc.arrayLength = CASCADE_COUNT;
	tex_desc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
#if !TARGET_OS_EMBEDDED
	tex_desc.storageMode = MTLStorageModePrivate;
#endif

	m_texture_array = [m_device newTextureWithDescriptor:tex_desc];
	releaseObj(tex_desc);

	m_bias_matrix.translate(KCL::Vector3D(0.5f, 0.5f, 0.0f));
	m_bias_matrix.scale(KCL::Vector3D(0.5f, 0.5f, 1.0f));
	
	m_viewport = { 0, 0, static_cast<float>(m_map_size), static_cast<float>(m_map_size), 0.0, 1.0 };
}

void CascadedShadowMap::BuildFrustums(const KCL::Vector3D &light_dir, const KCL::Camera2 *active_camera)
{
    m_light_dir = light_dir;
    m_light_dir.normalize();    // Be sure it is normalized

	m_camera = active_camera;

    // Update the shadow frustums
    Update();

    // Calculate the frustum distances in projected view space and the shadow matrices
    const KCL::Matrix4x4 & camera_projection = m_camera->GetProjection();
    for (int i = 0; i < CASCADE_COUNT; i++)
    {
        // Calculate the split depth values in NDC: projMatrix * vec4(0, 0, m_frustums[i].far_distance, 1) * 0.5 + 0.5
        m_frustum_distances.v[i] = (-m_frustums[i].far_distance * camera_projection.v33 + camera_projection.v43) / m_frustums[i].far_distance;
    }
}

id<MTLRenderCommandEncoder> CascadedShadowMap::GetRenderPassEncoder(id<MTLCommandBuffer> command_buffer, KCL::uint32 split)
{
    MTLRenderPassDescriptor * desc = [[MTLRenderPassDescriptor alloc] init];
    
    desc.depthAttachment.texture = m_texture_array;
    desc.depthAttachment.loadAction = MTLLoadActionClear;
    desc.depthAttachment.clearDepth = 1.0f;
    desc.depthAttachment.storeAction = MTLStoreActionStore;
    desc.depthAttachment.slice = split;

	id<MTLRenderCommandEncoder> encoder = [command_buffer renderCommandEncoderWithDescriptor:desc];
	encoder.label = [NSString stringWithFormat:@"Cascaded Shadow Map %d", split];
	releaseObj(desc);

	[encoder setViewport:m_viewport];
	[encoder setFrontFacingWinding:MTLWindingCounterClockwise];

	return encoder;
}

void CascadedShadowMap::SplitFrustumsLogaritmic()
{
    float lambda = 0.5f;
    float overlap_factor = 1.005f;

    float ratio = SHADOW_FAR / SHADOW_NEAR;
    float frustum_length = SHADOW_FAR - SHADOW_NEAR;

    m_frustums[0].near_distance = SHADOW_NEAR;
    m_frustums[CASCADE_COUNT - 1].far_distance = SHADOW_FAR;

    for (KCL::uint32 i = 1; i < CASCADE_COUNT; i++)
    {
        float si = i / (float)CASCADE_COUNT;

        m_frustums[i].near_distance = lambda*(SHADOW_NEAR * powf(ratio, si)) + (1.0f - lambda) * (SHADOW_NEAR + frustum_length * si);
        m_frustums[i-1].far_distance = m_frustums[i].near_distance * overlap_factor;
    }
}

void CascadedShadowMap::Update()
{
    // SplitFrustumsLogaritmic(frustum_near, frustum_far);

    // Calculate the world space view frustum
    KCL::Matrix4x4 inverse_projection;
    KCL::Matrix4x4::Invert4x4(m_camera->GetProjection(), inverse_projection);

    KCL::Matrix4x4 inverse_view;
    KCL::Matrix4x4::Invert4x4(m_camera->GetView(), inverse_view);

    // The light's coordinate system
    KCL::Vector3D right = KCL::Vector3D::cross(KCL::Vector3D(0.0f,1.0f,0.0f), m_light_dir);
    right.normalize();
    KCL::Vector3D up = KCL::Vector3D::cross(m_light_dir, right);
    up.normalize();

    // Frustum points in view projection space
    KCL::Vector3D projected_frustum_points[8] =
    {
        KCL::Vector3D(-1.0f,-1.0f,-1.0f), KCL::Vector3D(-1.0f,1.0f,-1.0f), KCL::Vector3D(1.0f,1.0f,-1.0f), KCL::Vector3D(1.0f,-1.0f,-1.0f),
        KCL::Vector3D(-1.0f,-1.0f, 1.0f), KCL::Vector3D(-1.0f,1.0f, 1.0f), KCL::Vector3D(1.0f,1.0f, 1.0f), KCL::Vector3D(1.0f,-1.0f, 1.0f),
    };

    // Frustum in view space
    for(KCL::uint32 i = 0; i < 8; i++)
    {
        m_view_frustum_points[i] = KCL::Vector3D(inverse_projection * KCL::Vector4D(projected_frustum_points[i]));
    }

    // Split directions in view space
    KCL::Vector3D directions[4];
    for(int i = 0; i < 4; i++)
    {
        directions[i] = m_view_frustum_points[i + 4] - m_view_frustum_points[i];
        directions[i].normalize();
    }

    // Split the view frustum
    float half_size = m_map_size / 2.0f;
    for(KCL::uint32 i = 0; i < CASCADE_COUNT; i++)
    {
        // Create the bounding box of the frustum split
        KCL::Vector3D bb_min(FLT_MAX, FLT_MAX, FLT_MAX);
        KCL::Vector3D bb_max = bb_min * -1.0f;

        // Calculate the view space bounding box
        // TODO: View space bounding box with view space slipping planes should be much strait forward
        KCL::Vector3D tmp[8];
        for(int j = 0; j < 4; j++)
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
            KCL::Vector3D normalized_texture_size = KCL::Vector3D(1.0f / m_map_size, 1.0f / m_map_size, 0.0f);

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
        m_frustums[i].light_space_aabb_max.z += m_default_shadow_range;

        if (m_use_map_based_selection)
        {
            // Move the far plane so we won't miss meshes during the frustum cull
            m_frustums[i].light_space_aabb_min.z -= 50.0f;
        }

        // Calculate the cull planes
        KCL::Matrix4x4 inverse_light;
        KCL::Matrix4x4::Invert4x4(m_frustums[i].camera.GetView(), inverse_light);
        CreateCullPlanes(m_frustums[i].light_space_aabb_min, m_frustums[i].light_space_aabb_max, inverse_light, m_frustums[i].cull_planes);
    }
}

// Shadow camera frustum cull. Note: Only one room is supported
void CascadedShadowMap::FrustumCull(KCL::uint32 split, KRL_Scene *scene, std::vector<KCL::Mesh*> visible_meshes[3], std::vector<KCL::MeshInstanceOwner2*> &visible_mios, bool force_cast_shadows)
{
    if (!split)
    {
        m_frame_counter++;
    }
    m_frustum_cull_counter++;

    KCL::Camera2 &camera = m_frustums[split].camera;

    KCL::Vector4D far_plane = m_frustums[split].cull_planes[KCL::CULLPLANE_FAR];
    KCL::Vector4D near_plane = m_frustums[split].cull_planes[KCL::CULLPLANE_NEAR];
    KCL::Vector4D p[] =
    {
        -m_frustums[split].cull_planes[0],
        -m_frustums[split].cull_planes[1],
        -m_frustums[split].cull_planes[2],
        -m_frustums[split].cull_planes[3],
        -far_plane,
        -near_plane,
    };

    // Light space XY dimensions of the light camera frustum
    float frustum_size_x = 0.0f;
    float frustum_size_y = 0.0f;
    if (m_fit_mode == FIT_SPHERE)
    {
        frustum_size_x = m_frustums[split].radius * 2.0f;
        frustum_size_y = m_frustums[split].radius * 2.0f;
    }
    else
    {
        frustum_size_x = fabsf(m_frustums[split].light_space_aabb_max.x - m_frustums[split].light_space_aabb_min.x);
        frustum_size_y = fabsf(m_frustums[split].light_space_aabb_max.y - m_frustums[split].light_space_aabb_min.y);
    }
    // Min size of one shadow texel in view(and world) space
    float texel_world_size_x = frustum_size_x / m_map_size * m_mesh_cull_size[split];
    float texel_world_size_y = frustum_size_y / m_map_size * m_mesh_cull_size[split];

    //float neasest_aabb = FLT_MAX;

    // NOTE: KCL::XRoom::FrustumCull can NOT be used for shadows. Only one room is supported
    // Cull the meshes
    if (scene->m_rooms.empty())
    {
        return;
    }

    std::vector<KCL::Mesh*> &meshes = scene->m_rooms[0]->m_meshes;
    KCL::uint32 mesh_count = meshes.size();
    KCL::Mesh *mesh;
    for (KCL::uint32 i = 0; i < mesh_count; i++)
    {
        mesh = meshes[i];

        // Check if the mesh was already rendered into another cascades
        if (mesh->m_frame_when_rendered == m_frame_counter)
        {
            continue;
        }

        bool shadow_casting_billboard = mesh->m_material->m_is_billboard && mesh->m_material->m_is_shadow_caster;
        if (!(force_cast_shadows || mesh->m_material->m_is_shadow_only || shadow_casting_billboard || mesh->m_flags & KCL::Mesh::OF_SHADOW_CASTER))
        {
            continue;
        }

        // Do not render small billboards to far cascades
        if (!force_cast_shadows && shadow_casting_billboard && split > 1)
        {
            continue;
        }

        // TODO: Precalculated for static meshes
        KCL::Vector3D center;
        KCL::Vector3D size;
        mesh->m_aabb.CalculateHalfExtentCenter(size, center);
        float radius = mesh->m_aabb.CalculateRadius();

        // Cull small object behind the camera in far cascades
        if (split > 0)
        {
            float nearPlaneDist = KCL::Vector4D::dot( KCL::Vector4D(center, 1.0f), m_camera->GetCullPlane(KCL::CULLPLANE_NEAR));
            if (nearPlaneDist < 0.0f)
            {
                if (radius < 0.8f && -nearPlaneDist > radius)
                {
                    continue;
                }
            }
        }

        // Cull small objects in far cascades
        if (m_mesh_cull_size[split] > 0.0f)
        {
            if (radius < texel_world_size_x || radius < texel_world_size_y)
            {
                continue;
            }
        }

        KCL::OverlapResult cull_result = KCL::XRoom::OVERLAP(p, 6, &mesh->m_aabb);
        if (cull_result == KCL::OVERLAP_OUTSIDE)
        {
            // No intersection
            continue;
        }
        if (cull_result == KCL::OVERLAP_INSIDE)
        {
            // Mark as fully rendered
            mesh->m_frame_when_rendered = m_frame_counter;
        }

        if( mesh->m_mio2)
        {
            if( mesh->m_mio2->m_frame_when_rendered != m_frustum_cull_counter)
            {
                mesh->m_mio2->m_frame_when_rendered = m_frustum_cull_counter;

                mesh->m_mio2->m_visible_instances.clear();

                visible_mios.push_back( mesh->m_mio2);
            }

            mesh->m_mio2->m_visible_instances.push_back( mesh);
        }
        else
        {
            visible_meshes[mesh->m_material->m_is_transparent].push_back(mesh);
        }

        // Find the closes AABB to the camera near plane
        /*
        KCL::Vector2D mm = mesh->m_aabb.DistanceFromPlane(near_plane);
        if (mm.x < neasest_aabb)
        {
            neasest_aabb = mm.x;
        }
        */
    }

    // Cull the actors
    KCL::uint32 actor_count = scene->m_actors.size();
    for(KCL::uint32 i = 0; i < actor_count; i++)
    {
        KCL::Actor *actor = scene->m_actors[i];
        mesh_count = actor->m_meshes.size();
        if (!mesh_count)
        {
            continue;
        }

        if (actor->m_meshes[0]->m_frame_when_rendered == m_frame_counter)
        {
            // Check if the actor was already rendered into another cascades
            continue;
        }

        //HACK: for now, set every mesh of each actor as shadow-caster
        //if(!(actor->m_flags & KCL::Mesh::ObjectFlags::OF_SHADOW_CASTER) && (!force_cast_shadows))
        //{
        //    continue;
        //}

        // Actor AABBs are upscaled by 1.2 for the main camera frustum cull but we don't want to be so conservative
        KCL::AABB aabb = actor->m_aabb;
        aabb.BiasAndScale(KCL::Vector3D(0, 0, 0), KCL::Vector3D(0.8f, 0.8f, 0.8f));
        KCL::OverlapResult overlap_result = overlap_result = KCL::XRoom::OVERLAP(p, 6, &aabb);

        if (overlap_result == KCL::OVERLAP_OUTSIDE)
        {
            // No intersection
            continue;
        }
        if (overlap_result == KCL::OVERLAP_INSIDE)
        {
            // Mark as fully rendered
            actor->m_meshes[0]->m_frame_when_rendered = m_frame_counter;
        }


        /*
        // Find the closes AABB to the camera near plane
        KCL::Vector2D mm = actor->m_aabb.DistanceFromPlane(near_plane);
        if (mm.x < neasest_aabb)
        {
            neasest_aabb = mm.x;
        }
        */

        // Collect the actor meshes
        for(KCL::uint32 i = 0; i < mesh_count; i++)
        {
            mesh = actor->m_meshes[i];
            //if (force_cast_shadows || mesh->m_material->m_is_shadow_caster || (mesh->m_flags & KCL::Mesh::ObjectFlags::OF_SHADOW_CASTER))
            //TODO: proper actor markup

            if (force_cast_shadows || mesh->m_material->m_is_shadow_only || mesh->m_flags & KCL::Mesh::OF_SHADOW_CASTER)
            {
                visible_meshes[mesh->m_material->m_is_transparent].push_back(mesh);
            }
        }
    }

    /*
    float camera_near = -m_frustums[split].light_space_aabb_max.z;
    float camera_far = -m_frustums[split].light_space_aabb_min.z;
    if (neasest_aabb != FLT_MAX && neasest_aabb > 0)
    {
        camera_near += neasest_aabb;
    }
    */

    // Move back the near plane to the bounding box
    float camera_near;
    if (m_use_map_based_selection)
    {
        camera_near = -m_frustums[split].light_space_aabb_max.z;
    }
    else
    {
        camera_near = -(m_frustums[split].light_space_aabb_max.z - m_default_shadow_range);
    }

    float camera_far = -m_frustums[split].light_space_aabb_min.z;
    if (m_fit_mode == FIT_SPHERE)
    {
        camera.Ortho(
            -m_frustums[split].radius,
            m_frustums[split].radius,
            -m_frustums[split].radius,
            m_frustums[split].radius,
            camera_near, camera_far);
    }
    else
    {
        camera.Ortho(
            m_frustums[split].light_space_aabb_min.x,
            m_frustums[split].light_space_aabb_max.x,
            m_frustums[split].light_space_aabb_min.y,
            m_frustums[split].light_space_aabb_max.y,
            camera_near, camera_far);
    }
    camera.Update();
    m_shadow_matrices[split] = camera.GetViewProjection() * m_bias_matrix;
}

void CascadedShadowMap::CreatePoissonDisk()
{
    float values[62] =
    {
        -0.1432226f, -0.6971881f,
        -0.4855936f, -0.4969095f,
        -0.1912299f, -0.3550386f,
        0.2838287f, -0.3891824f,
        0.1349263f, -0.7331034f,
        -0.04226192f, -0.9649448f,
        -0.1679147f, 0.01174963f,
        -0.4561833f, -0.1507082f,
        -0.4913784f, -0.7940582f,
        -0.8250874f, -0.4104999f,
        -0.735054f, -0.1364969f,
        0.02850272f, -0.1781504f,
        -0.6895428f, 0.216547f,
        0.374657f, -0.8589867f,
        0.4801042f, -0.1622594f,
        0.7959307f, -0.2359155f,
        0.2443489f, 0.09613793f,
        0.8459272f, 0.06077503f,
        0.6920388f, -0.6604857f,
        0.5186383f, 0.1555034f,
        -0.3205484f, 0.2873383f,
        -0.5701106f, 0.5812982f,
        -0.8429717f, 0.5278636f,
        0.05409959f, 0.4258883f,
        -0.1204641f, 0.788719f,
        -0.417988f, 0.8938821f,
        0.4473081f, 0.4350664f,
        0.7635057f, 0.3206361f,
        0.2135869f, 0.7461811f,
        0.5030554f, 0.7073274f,
        -0.9930909f, 0.1079298f
    };

    KCL::uint32 floats = sizeof(values) / sizeof(float);
    m_poisson_disk.reserve(floats / 2 + 1);
    m_poisson_disk.push_back(KCL::Vector2D(0.0f, 0.0f));
    for (KCL::uint32 i = 0; i < floats; i += 2)
    {
        m_poisson_disk.push_back(KCL::Vector2D(values[i], values[i + 1]));
    }
    std::sort(m_poisson_disk.begin(), m_poisson_disk.end(), &CascadedShadowMap::PoissonOffsetCompare);
}

bool CascadedShadowMap::PoissonOffsetCompare(const KCL::Vector2D &A, const KCL::Vector2D &B)
{
    return (A.x * A.x + A.y * A.y) < (B.x * B.x + B.y * B.y);
}

void CascadedShadowMap::ExpandBoundingBox(KCL::Vector3D & min, KCL::Vector3D & max, const KCL::Vector3D & p)
{
    if(min.x > p.x)
    {
        min.x = p.x;
    }
    if(max.x < p.x)
    {
        max.x = p.x;
    }

    if(min.y > p.y)
    {
        min.y = p.y;
    }
    if(max.y < p.y)
    {
        max.y = p.y;
    }

    if(min.z > p.z)
    {
        min.z = p.z;
    }
    if(max.z < p.z)
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

    v[0] = KCL::Vector3D( min.x, min.y, min.z);
    v[1] = KCL::Vector3D( max.x, min.y, min.z);
    v[2] = KCL::Vector3D( max.x, max.y, min.z);
    v[3] = KCL::Vector3D( min.x, max.y, min.z);

    v[4] = KCL::Vector3D( min.x, min.y, max.z);
    v[5] = KCL::Vector3D( max.x, min.y, max.z);
    v[6] = KCL::Vector3D( max.x, max.y, max.z);
    v[7] = KCL::Vector3D( min.x, max.y, max.z);

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
