/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glb_scene_opengl4_support.h"
#include "glb_scene_opengl4.h"
#include <kcl_mesh.h>
#include "opengl/gl_wrapper/gl_wrapper.h"
#include "opengl/ext.h"
#include "opengl/glb_material4.h"

#include <algorithm>

#include "ng/json.h"
#include "ng/log.h"


#ifndef GL_TIME_ELAPSED
#define  GL_TIME_ELAPSED 0x88BF
#endif

#if (defined __ANDROID__ || defined HAVE_GLES3)

typedef void(*PFNGLGETQUERYOBJECTIVPROCGFXB) (GLuint id, GLenum pname, GLint* params);
PFNGLGETQUERYOBJECTIVPROCGFXB glGetQueryObjectiv = 0;

typedef void(*PFNGLGETQUERYOBJECTUI64VPROCGFXB) (GLuint id, GLenum pname, GLuint64* params);
PFNGLGETQUERYOBJECTUI64VPROCGFXB glGetQueryObjectui64v = 0;

#endif

using namespace KCL;

Profiler* Profiler::s_instance = NULL;

namespace GFXB4
{

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
            INFO( "glb_scene_opengl4_support: GL error (%x) - Create2DTexture", e);
        }

        return texture_object;
    }

}

#if ENABLE_QUICK_STATS
void ProfilerBeginFrame(KCL::uint32 animation_time)
{
    Profiler::CreateInstance()->BeginFrame(animation_time);
}

void ProfilerEndFrame()
{
    Profiler::GetInstance()->EndFrame();
}

void ProfilerBeginPass(const char *name, KCL::int32 index)
{
    Profiler::GetInstance()->BeginPass(name, index);
}

void ProfilerEndPass()
{
    Profiler::GetInstance()->EndPass();
}

void ProfilerRelease()
{
    Profiler::ReleaseInstance();
}
#endif

Profiler* Profiler::CreateInstance()
{
    if (s_instance == NULL)
    {
        s_instance = new Profiler();
    }
    return s_instance;
}

Profiler* Profiler::GetInstance()
{
    return s_instance;
}

void Profiler::ReleaseInstance()
{
    delete s_instance;
    s_instance = NULL;
}

Profiler::Profiler()
{
    m_gl_timer = 0;

    m_last_print_time = 0;
    m_enabled = true;

    glGenQueries(1, &m_gl_timer);
    m_start_time = 0;

    m_frame_animation_time = 0;

#if GL_WRAPPER_ENABLED
    m_gl_wrapper = GetGLWrapper();
#else
    m_gl_wrapper = NULL;
#endif

}

Profiler::~Profiler()
{
    glDeleteQueries(1, &m_gl_timer);
}

void Profiler::SetFilter(const char *name)
{
    m_results.clear();
    m_filter = name;
}

bool Profiler::IsFiltered()
{
    if (m_filter.empty())
    {
        return false;
    }
    return m_current_pass.find(m_filter) == std::string::npos;
}

void Profiler::BeginFrame(KCL::uint32 animation_time)
{
    if (!m_enabled)
    {
        return;
    }
    m_frame_animation_time = animation_time;
}

void Profiler::EndFrame()
{
    if (!m_enabled)
    {
        return;
    }

    //PrintResults();

    
#ifdef HAVE_GUI_FOLDER
    for (KCL::uint32 i = 0; i < m_results.size(); i++)
    {
        m_results[i].Clear();
    }
#else
    PrintResults();
#endif
    
}

void Profiler::BeginPass(const char *name, KCL::int32 index)
{
    if (!m_enabled)
    {
        return;
    }    

    if (index < 0)
    {
        m_current_pass = name;
    }
    else
    {
        std::stringstream ss;
        ss << name << "#" << index;
        m_current_pass = ss.str();
    }

    if (IsFiltered())
    {
        return;
    }

    glPushDebugGroupProc(GL_DEBUG_SOURCE_APPLICATION, index, -1, m_current_pass.c_str());

    if (m_gl_wrapper)
    {
        m_gl_wrapper->BeginMeasureQuick();
    }

    if (USE_GL_TIMER)
    {
        glBeginQuery(GL_TIME_ELAPSED, m_gl_timer);
    }
    else
    {
        glFinish();
        m_start_time = KCL::g_os->GetTimeMilliSec();
    }
}

void Profiler::EndPass()
{
    if (!m_enabled)
    {
        return;
    }

    if (IsFiltered())
    {
        return;
    }

    double time_elapsed;

    if (USE_GL_TIMER)
    {
        glEndQuery(GL_TIME_ELAPSED);
        GLuint result_available = 0;
        while (!result_available)
        {
            glGetQueryObjectuiv(m_gl_timer, GL_QUERY_RESULT_AVAILABLE, &result_available);
        }

        GLuint64 time_elapsed_uint64 = 0;
        glGetQueryObjectui64vProc(m_gl_timer, GL_QUERY_RESULT, &time_elapsed_uint64);
        time_elapsed = double(time_elapsed_uint64);
    }
    else
    {
        glFinish();
        time_elapsed = (KCL::g_os->GetTimeMilliSec() - m_start_time) * 1000000.0;
    }

    PassResult &result = GetResult(m_current_pass.c_str());
    result.elapsed_time += time_elapsed;
    if (time_elapsed > result.max_elapsed_time && m_frame_animation_time > 10000)
    {
        result.max_elapsed_time = time_elapsed;
        result.max_elapsed_animation_time = m_frame_animation_time;
    }
    if (time_elapsed < result.min_elapsed_time)
    {
        result.min_elapsed_time = time_elapsed;
        result.min_elapsed_animation_time = m_frame_animation_time;
    }

    if (m_gl_wrapper)
    {
        KCL::uint32 draw_calls = 0;
        KCL::uint32 prim_count = 0;
        KCL::uint32 qprim_count = 0;
        m_gl_wrapper->EndMeasureQuick(draw_calls, prim_count, qprim_count);
        result.draw_calls_sum += draw_calls;
        result.prim_count_sum += prim_count;
        result.qprim_count_sum += qprim_count;
    }

    result.sample_count++;

    result.sum_elapsed_time += time_elapsed;
    result.sum_sample_count++;

    glPopDebugGroupProc();
}

Profiler::PassResult &Profiler::GetResult(const char *name)
{
    for (KCL::uint32 i = 0; i < m_results.size(); i++)
    {
        if (m_results[i].name == name)
        {
            return m_results[i];
        }
    }
    m_results.resize(m_results.size() + 1);
    PassResult &result = m_results.back();
    result.name = m_current_pass;
    return result;
}

void Profiler::PrintResults(bool forced)
{
    double now = KCL::g_os->GetTimeMilliSec();
    if (!forced && now - m_last_print_time < 5000)
    {
        return;
    }
    m_last_print_time = now;

    if (!forced)
    {
        // Report the average results
        double time_sum = 0.0;
        for (KCL::uint32 i = 0; i < m_results.size(); i++)
        {
            float samples = m_results[i].sample_count;
            if (samples == 0.0f)
            {
                continue;
            }

            NGLOG_INFO("%s: %sms",
                m_results[i].name,
                float(m_results[i].elapsed_time / 1000000.0) / samples);

            /*
            NGLOG_INFO("%s %sms max: %s %s min: %s %s",
                m_results[i].name,
                float(m_results[i].elapsed_time / 1000000.0) / samples,
                float(m_results[i].max_elapsed_time / 1000000.0), m_results[i].max_elapsed_animation_time,
                float(m_results[i].min_elapsed_time / 1000000.0), m_results[i].min_elapsed_animation_time);
            */


            /*
            NGLOG_INFO("%s time: %s max: %s min: %s geometry stats: %s %s %s",
            m_results[i].name,
            float(m_results[i].elapsed_time / 1000000.0) / samples,
            float(m_results[i].max_elapsed_time / 1000000.0),
            float(m_results[i].min_elapsed_time / 1000000.0),
            m_results[i].draw_calls_sum / samples,
            m_results[i].prim_count_sum / samples,
            m_results[i].qprim_count_sum / samples);
            */

            time_sum += m_results[i].elapsed_time / samples;
        }
        double time_msecs = time_sum / 1000000.0;
        double frame_rate = 0.0;
        if (time_msecs > 0.0)
        {
            frame_rate = 1000.0 / time_msecs;
        }
        NGLOG_INFO("GPU: %s ~FPS: %s", float(time_msecs), float(frame_rate));

    }
    else
    {  
        // Report the final results (separated by ; so we can export to CSV)
        std::stringstream sstream;
        sstream << "Overall average results:" << std::endl;

        double time_sum = 0.0;
        for (KCL::uint32 i = 0; i < m_results.size(); i++)
        {
            float samples = m_results[i].sum_sample_count;
            if (samples == 0.0f)
            {
                continue;
            }

            sstream << m_results[i].name;
            sstream << ';';
            sstream << (float(m_results[i].sum_elapsed_time / 1000000.0) / samples);
            sstream << std::endl;

            time_sum += m_results[i].sum_elapsed_time / samples;
        }

        double time_msecs = time_sum / 1000000.0;
        double frame_rate = 0.0;
        if (time_msecs > 0.0)
        {
            frame_rate = 1000.0 / time_msecs;
        }

        sstream << "GPU: " << float(time_msecs) << " ~FPS: " << float(frame_rate) << std::endl;
        NGLOG_INFO("%s", sstream.str());      
    }

    for (KCL::uint32 i = 0; i < m_results.size(); i++)
    {
        m_results[i].Clear();
    }
}


StatisticsLogger::StatisticsLogger()
{
}

StatisticsLogger::~StatisticsLogger()
{
}

void StatisticsLogger::ClearFrames()
{
    m_frames.clear();
    m_names.clear();
}

void StatisticsLogger::AddFrame(KCL::uint32 m_animation_time, std::vector<std::wstring> &qs_names, std::vector<KCL::uint32> &qs_draws, std::vector<KCL::uint32> &qs_tris)
{
    if (m_names.empty())
    {
        m_names = qs_names;
        m_names.push_back(L"TOTAL");
    }
    FrameStatistics frame;
    frame.m_animation_time = m_animation_time;
    frame.qs_draws = qs_draws;
    frame.qs_tris = qs_tris;

    KCL::uint32 max_draws = 0;
    KCL::uint32 max_tris = 0;
    for (KCL::uint32 i = 0; i < m_names.size() - 1; i++)
    {
        max_draws += frame.qs_draws[i];
        max_tris += frame.qs_tris[i];
    }
    frame.qs_draws.push_back(max_draws);
    frame.qs_tris.push_back(max_tris);

    m_frames.push_back(frame);
}

void StatisticsLogger::GetHeaviestFrames(const wchar_t *name, bool triangles, std::vector<FrameStatistics> &result)
{
    result.clear();
    if (m_frames.empty())
    {
        return;
    }
    KCL::int32 qs_index = GetItemIndex(name);
    if (qs_index < 0)
    {
        return;
    }

    KCL::uint32 max = 0;
    for (KCL::uint32 i = 0; i < m_frames.size(); i++)
    {
        KCL::uint32 value = triangles ? m_frames[i].qs_tris[qs_index]  : m_frames[i].qs_draws[qs_index];
        if (value > max)
        {
            max = value;
        }       
    }

    for (KCL::uint32 i = 0; i < m_frames.size(); i++)
    {
         KCL::uint32 value = triangles ? m_frames[i].qs_tris[qs_index]  : m_frames[i].qs_draws[qs_index];
         if (max == value)
         {
             result.push_back(m_frames[i]);
         }
    }
}

void StatisticsLogger::GetHeaviestFrames(const wchar_t *name, bool triangles, KCL::uint32 limit, std::vector<FrameStatistics> &result)
{
    result.clear();
    if (m_frames.empty())
    {
        return;
    }
    KCL::int32 qs_index = GetItemIndex(name);
    if (qs_index < 0)
    {
        return;
    }
    for (KCL::uint32 i = 0; i < m_frames.size(); i++)
    {
        KCL::uint32 value = triangles ? m_frames[i].qs_tris[qs_index]  : m_frames[i].qs_draws[qs_index];
         if (value >= limit)
         {
             result.push_back(m_frames[i]);
         }
    }
}

void StatisticsLogger::Save(const char *filename)
{
    ng::JsonValue json_result;

    ng::JsonValue json_names;
    char ascii_str[256];
    for (KCL::uint32 j = 0; j < m_names.size(); j++)
    {
        wcstombs(ascii_str, m_names[j].c_str(), 256);
        json_names.push_back(ascii_str);
    }
    json_result["names"] = json_names;

    ng::JsonValue json_frames;
    for (KCL::uint32 i = 0; i < m_frames.size(); i++)
    {
        ng::JsonValue json_triangles;
        ng::JsonValue json_draws;
        for (KCL::uint32 j = 0; j < m_names.size(); j++)
        {      
            json_draws.push_back(m_frames[i].qs_draws[j]);
            json_triangles.push_back(m_frames[i].qs_tris[j]);
        }
        ng::JsonValue json_frame;
        json_frame["draws"] = json_draws;
        json_frame["triangles"] = json_triangles;
        json_frame["frame_time"] = m_frames[i].m_animation_time;
        json_frames.push_back(json_frame);
    }
    json_result["frames"] = json_frames;

    ng::Result result;
    json_result.toFile(filename, false, result);
}

void StatisticsLogger::Load(const char *filename)
{
    m_names.clear();
    m_frames.clear();

    ng::JsonValue json_result;
    ng::Result result;
    json_result.fromFile(filename, result);
    if (!result.ok())
    {
        return;
    }

    ng::JsonValue json_name = json_result["names"];
    wchar_t wchar_str[256];
    for (int i = 0; i < json_name.size(); i++)
    {
        mbstowcs(wchar_str, json_name[i].string(), 256);
        m_names.push_back(wchar_str);
    }

     ng::JsonValue json_frames = json_result["frames"];
     for (int i = 0; i < json_frames.size(); i++)
     {
         FrameStatistics frame;
         const ng::JsonValue &json_draws = json_frames[i]["draws"];
         const ng::JsonValue &json_trinagles = json_frames[i]["triangles"];
         for (int j = 0; j < json_draws.size(); j++)
         {
             frame.qs_draws.push_back(KCL::uint32(json_draws[j].number()));
         }
         for (int j = 0; j < json_trinagles.size(); j++)
         {
             frame.qs_tris.push_back(KCL::uint32(json_trinagles[j].number()));
         }
         frame.m_animation_time = KCL::uint32(json_frames[i]["frame_time"].number());
         m_frames.push_back(frame);
     }
}

KCL::int32 StatisticsLogger::GetItemIndex(const wchar_t *name)
{
    for (KCL::uint32 i = 0; i < m_names.size(); i++)
    {
        std::wstring &wstr = m_names[i];
        if (wstr == name)
        {
            return i;
        }
    }
    return -1;
}

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


bool GLB_Scene4Tools::displacement_depth_compare(const mesh_sort_info* A, const mesh_sort_info* B)
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

bool GLB_Scene4Tools::depth_compare(const mesh_sort_info* A, const mesh_sort_info* B)
{
    // Front to back
    return A->vertexCenterDist < B->vertexCenterDist;
}

bool GLB_Scene4Tools::reverse_depth_compare(const mesh_sort_info* A, const mesh_sort_info* B)
{
    // Back to front
    return A->vertexCenterDist > B->vertexCenterDist; 
}

bool GLB_Scene4Tools::material_compare(const mesh_sort_info* A, const mesh_sort_info* B)
{
    return A->sort_order < B->sort_order;
}

// Material sor
bool GLB_Scene4Tools::reverse_material_compare(const mesh_sort_info* A, const mesh_sort_info* B)
{
    if (A->sort_order != 0 && B->sort_order != 0)
    {
        return A->sort_order < B->sort_order;
    }
    return A->sort_order == 0;
}

bool GLB_Scene4Tools::material_depth_compare(const mesh_sort_info* A, const mesh_sort_info* B)
{
    if (A->sort_order == B->sort_order)
    {
        //Depth sort
        return A->vertexCenterDist < B->vertexCenterDist;
}
    return A->sort_order < B->sort_order;
}

std::vector<GLB_Scene4Tools::mesh_sort_info> GLB_Scene4Tools::sorted_meshes;
std::vector<GLB_Scene4Tools::mesh_sort_info*> GLB_Scene4Tools::sort_pointers;

void GLB_Scene4Tools::SortMeshes(std::vector<KCL::Mesh*> &visible_meshes, const KCL::Camera2 *camera, bool alpha_blend)
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
            mesh_info.sort_order = ((GLB::Material4*)mesh->m_materials[RENDER_MATERIAL_ID])->m_sort_order;
        }
        else
        {
            mesh_info.sort_order = ((GLB::Material4*)mesh->m_material)->m_sort_order;
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
        std::sort (begin_it, begin_it + mesh_count, &GLB_Scene4Tools::reverse_depth_compare);
    }
    else
    {
         //std::sort (begin_it, begin_it + mesh_count, &GLB_Scene4Tools::displacement_depth_compare);
        std::sort (begin_it, begin_it + mesh_count, &GLB_Scene4Tools::material_depth_compare);
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


KCL::Vector3D GLB_Scene4Tools::CalculateRayDirection(KCL::Camera2 *camera,  float viewport_width, float viewport_height, float x, float y)
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

KCL::Mesh* GLB_Scene4Tools::PickMesh(GLB_Scene_ES2_ *scene, KCL::Camera2 *camera, float x, float y)
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

bool GLB_Scene4Tools::IntersectMesh(const KCL::Vector3D &ray_from, const KCL::Vector3D &ray_to, const KCL::Vector3D &ray_dir, KCL::Mesh *mesh, float &distance)
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

KCL::uint32 GLB_Scene4Tools::GetSupportedFPRT()
{
    if (GLB::g_extension->isES())
    {
        if (GLB::g_extension->hasExtension(GLB::GLBEXT_color_buffer_half_float))
        {
            return GL_RGBA16F;
        }
        if (GLB::g_extension->hasExtension(GLB::GLBEXT_color_buffer_float))
        {
            return GL_RGBA32F;
        }
        // On error
        NGLOG_ERROR("Floating point RT does not supported!");
        assert(0);
        return 0;
    }
    else
    {
        // Supported in core
        return GL_RGBA16F;
    }
}

bool GLTimeQuery::m_function_adresses_retrieved = false;
std::vector<GLTimeQuery*> GLTimeQuery::m_time_queries;
GLTimeQuery::GLTimeQuery(const char* name)
{
#if USE_TIME_QUERY

#if defined __ANDROID__ || defined HAVE_GLES3
    if (!m_function_adresses_retrieved) {
        glGetQueryObjectiv    = (PFNGLGETQUERYOBJECTIVPROC)    eglGetProcAddress("glGetQueryObjectiv");
        glGetQueryObjectui64v = (PFNGLGETQUERYOBJECTUI64VPROC) eglGetProcAddress("glGetQueryObjectui64v");
    }
#endif

    glGenQueries(1,&m_time_query);

#endif

    m_time_queries.push_back(this);
    m_name = name;
    m_sum = 0;
    tick = 0;
}

GLTimeQuery::~GLTimeQuery() {

#if USE_TIME_QUERY
    glDeleteQueries(1,&m_time_query);
#endif

    std::vector<GLTimeQuery*>::iterator p;

    p = std::find(m_time_queries.begin(), m_time_queries.end(), this);

    m_time_queries.erase(p);
}

void GLTimeQuery::Begin() {

    m_time_query_available = 0;
    m_time_elapsed = 0;
#if USE_TIME_QUERY
    glBeginQuery(GL_TIME_ELAPSED,m_time_query);
#endif

}

void GLTimeQuery::End()
{
#if USE_TIME_QUERY
    glEndQuery(GL_TIME_ELAPSED);
#endif
}

GLuint64 GLTimeQuery::GetResult() {
#if USE_TIME_QUERY
    while (!m_time_query_available) {
        glGetQueryObjectiv(m_time_query, GL_QUERY_RESULT_AVAILABLE, &m_time_query_available);
    }

    glGetQueryObjectui64v(m_time_query, GL_QUERY_RESULT, &m_time_elapsed);
#endif

    return m_time_elapsed;
}

GLuint64 GLTimeQuery::Print() {
    GetResult();
    INFO("%s time: %d",m_name.c_str(),m_time_elapsed/1000);
    return m_time_elapsed;
}

void GLTimeQuery::Commit() {
    End();
    GetResult();
    tick++;
    m_sum += m_time_elapsed;
}

GLuint64 GLTimeQuery::EndAndPrint() {
    End();
    return Print();
}

void GLTimeQuery::PrintAll() {
    for (unsigned int i = 0; i < m_time_queries.size(); i++) {
        m_time_queries[i]->Print();
    }
}

void GLTimeQuery::PrintSum() {
#if USE_TIME_QUERY
    double d_sum_time = m_sum/1000000.0;
    INFO("%s time sum: %f avg: %f",m_name.c_str(),d_sum_time,d_sum_time/tick);
#endif
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


WorkGroupValidator::WorkGroupValidator()
{
    GLint max_ws_size_x = 0;
    GLint max_ws_size_y = 0;
    GLint max_ws_size_z = 0;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &max_ws_size_x);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &max_ws_size_y);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &max_ws_size_z);
    m_max_ws_size_x = max_ws_size_x;
    m_max_ws_size_y = max_ws_size_y;
    m_max_ws_size_z = max_ws_size_z;

    GLint max_invocations = 0;
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max_invocations);
    m_max_invocations = max_invocations;

    GLint shared_memory_size = 0;
    glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &shared_memory_size);
    m_max_shared_memory = shared_memory_size;
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
WarmUpHelper::WarmUpHelper(GLB_Scene4 *scene, const std::string &wg_sizes)
{
    m_scene = scene;
    m_start_time = 0;

    m_timer_query = 0;
    if (s_use_timer_query)
    {
        glGenQueries(1, &m_timer_query);
    }

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

    if (m_timer_query)
    {
        glDeleteQueries(1, &m_timer_query);
    }
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
    if (s_use_timer_query)
    {
        glBeginQuery(GL_TIME_ELAPSED, m_timer_query);
    }
    else
    {
        glFinish();
        m_start_time = KCL::uint64(KCL::g_os->GetTimeMilliSec());	
    }	
}

KCL::uint64 WarmUpHelper::EndTimer()
{
    if (s_use_timer_query)
    {
        glEndQuery(GL_TIME_ELAPSED);
        GLuint result_available = 0;
        while (!result_available)
        {
            glGetQueryObjectuiv(m_timer_query, GL_QUERY_RESULT_AVAILABLE, &result_available);
        }

        GLuint64 time_elapsed_uint64 = 0;
        glGetQueryObjectui64vProc(m_timer_query, GL_QUERY_RESULT, &time_elapsed_uint64);
        return time_elapsed_uint64 / 1000000.0;
    }
    else
    {
        glFinish();
        KCL::uint64 now = KCL::uint64(KCL::g_os->GetTimeMilliSec());
        return now - m_start_time;
    }	
}
