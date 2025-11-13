/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_SCENE_OPENGL4_SUPPORT_H
#define GLB_SCENE_OPENGL4_SUPPORT_H

#include <kcl_math3d.h>
#include <kcl_camera2.h>

#include "glb_mesh.h"
#include "opengl/shader.h"
#include "opengl/glb_shader2.h"
#include "opengl/glb_discard_functions.h"

#include <vector>
#include <map>

class GLB_Scene_ES2_;
class GLB_Scene4;
class GLWrapper;

namespace GLB
{
    class FragmentBlur;
    class Material4;
}

namespace GFXB4
{
    class ComputeReduction;
}

class GLB_Scene4Tools
{
public:	
    static void SortMeshes(std::vector<KCL::Mesh*> &visible_meshes, const KCL::Camera2 *camera, bool alpha_blend);
    static KCL::Mesh* PickMesh(GLB_Scene_ES2_ *scene, KCL::Camera2 *camera, float x, float y);
    static KCL::uint32 GetSupportedFPRT();
private:
    struct mesh_sort_info
    {
        KCL::Mesh *mesh;
      KCL::int32 sort_order;
        float vertexCenterDist;
    };

    // Front to back
    static bool depth_compare(const mesh_sort_info* A, const mesh_sort_info* B);

    // Back to front
    static bool reverse_depth_compare(const mesh_sort_info* A, const mesh_sort_info* B);

    // Material sort
    static bool material_compare(const mesh_sort_info* A, const mesh_sort_info* B);

    // Material sort
    static bool reverse_material_compare(const mesh_sort_info* A, const mesh_sort_info* B);

    // Material + depth sort (front to back)
    static bool material_depth_compare(const mesh_sort_info* A, const mesh_sort_info* B);    
   
    // RC2 sort function
    static bool displacement_depth_compare(const mesh_sort_info* A, const mesh_sort_info* B);

    static std::vector<mesh_sort_info*> sort_pointers;
    static std::vector<mesh_sort_info> sorted_meshes;

    static KCL::Vector3D CalculateRayDirection(KCL::Camera2 *camera, float viewport_width, float viewport_height, float x, float y);
    static bool IntersectMesh(const KCL::Vector3D &ray_from, const KCL::Vector3D &ray_to, const KCL::Vector3D &ray_dir, KCL::Mesh *mesh, float &distance);
};

class StatisticsLogger
{
public:    
    struct FrameStatistics
    {    
        std::vector<KCL::uint32> qs_draws;
        std::vector<KCL::uint32> qs_tris;
        KCL::uint32 m_animation_time;
    };

    StatisticsLogger();
    ~StatisticsLogger();

    void ClearFrames();
    void AddFrame(KCL::uint32 m_animation_time, std::vector<std::wstring> &qs_names, std::vector<KCL::uint32> &qs_draws, std::vector<KCL::uint32> &qs_tris);
    void Save(const char *filename);
    void Load(const char *filename);
  
    void GetHeaviestFrames(const wchar_t *name, bool triangles, std::vector<FrameStatistics> &result);
    void GetHeaviestFrames(const wchar_t *name, bool triangles, KCL::uint32 limit, std::vector<FrameStatistics> &result);
    
    std::vector<std::wstring> m_names;
    std::vector<FrameStatistics> m_frames;
    KCL::int32 GetItemIndex(const wchar_t *name);
};

class ParaboloidCulling : public KCL::CullingAlgorithm
{
public:
    ParaboloidCulling(KCL::uint32 size_x, KCL::uint32 size_y, float limit);
    virtual ~ParaboloidCulling() {}

    void SetLimit(float limit);
    virtual bool CullMesh(KCL::Camera2 *camera, KCL::Mesh *mesh);

private:
    float m_limit;
    float m_size_x;
    float m_size_y;
};

class PerspectiveCulling : public KCL::CullingAlgorithm
{
public:
    PerspectiveCulling(float limit);
    virtual ~PerspectiveCulling() {}
    virtual bool CullMesh(KCL::Camera2 *camera, KCL::Mesh *mesh);

private:
    float m_limit;
};


class WorkGroupValidator
{
public:
    struct WGConfig
    {
        WGConfig(KCL::uint32 size_x = 1, KCL::uint32 size_y = 1, KCL::uint32 size_z = 1);
        ~WGConfig();
        void Set(KCL::uint32 size_x, KCL::uint32 size_y = 1, KCL::uint32 size_z = 1);

        KCL::uint32 size_x;
        KCL::uint32 size_y;
        KCL::uint32 size_z;
    };

    WorkGroupValidator();
    ~WorkGroupValidator();

    bool Validate(const WGConfig &config) const;
    bool Validate(const std::vector<WGConfig> &configs, std::vector<WorkGroupValidator::WGConfig> &result) const;
    bool Validate(KCL::uint32 size_x, KCL::uint32 size_y = 1, KCL::uint32 size_z = 1) const;

    KCL::uint32 m_max_ws_size_x;
    KCL::uint32 m_max_ws_size_y;
    KCL::uint32 m_max_ws_size_z;

    KCL::uint32 m_max_invocations;

    KCL::uint32 m_max_shared_memory;
};

class Profiler
{
    static const bool USE_GL_TIMER = true;
public:
    struct PassResult
    {
        std::string name;
        KCL::uint32 sample_count;

        double elapsed_time;

        KCL::uint32 draw_calls_sum;
        KCL::uint32 prim_count_sum;
        KCL::uint32 qprim_count_sum;
        
        KCL::uint32 sum_sample_count;
        double sum_elapsed_time;
        double min_elapsed_time;
        double max_elapsed_time;

        KCL::uint32 min_elapsed_animation_time;
        KCL::uint32 max_elapsed_animation_time;

        PassResult()
        {
            Clear();

            sum_sample_count = 0;
            sum_elapsed_time = 0;
            min_elapsed_time = FLT_MAX;
            max_elapsed_time = -FLT_MAX;

            min_elapsed_animation_time = 0;
            max_elapsed_animation_time = 0;
        }

        void Clear()
        {
            sample_count = 0;
            elapsed_time = 0.0;
            draw_calls_sum = 0;
            prim_count_sum = 0;
            qprim_count_sum = 0;
        }
    };

    ~Profiler();

    static Profiler *CreateInstance();
    static Profiler *GetInstance();
    static void ReleaseInstance();

    void SetFilter(const char *name);

    void BeginFrame(KCL::uint32 animation_time);
    void EndFrame();

    void BeginPass(const char *name, KCL::int32 index = -1);
    void EndPass();

    void PrintResults(bool forced = false);

    std::vector<PassResult> &GetResults() { return m_results; }
    bool IsEnabled() { return m_enabled; }
    void SetEnabled(bool enabled) { m_enabled = enabled; }
    
    PassResult &GetResult(const char *name);

private:
    Profiler();
    static Profiler *s_instance;

    std::vector<PassResult> m_results;
    KCL::uint32 m_gl_timer;
    double m_start_time;
    GLWrapper *m_gl_wrapper;
    bool m_enabled;

    std::string m_current_pass;
    std::string m_filter;
    
    double m_last_print_time;

    KCL::uint32 m_frame_animation_time;

    bool IsFiltered();
};

#if ENABLE_QUICK_STATS
extern void ProfilerBeginFrame(KCL::uint32 animation_time);
extern void ProfilerEndFrame();
extern void ProfilerBeginPass(const char *name, KCL::int32 index = -1);
extern void ProfilerEndPass();
extern void ProfilerRelease();
#else
#define ProfilerBeginFrame(...)     ;
#define ProfilerEndFrame(...)       ;
#define ProfilerBeginPass(...)      ;
#define ProfilerEndPass(...)        ;
#define ProfilerRelease(...)        ;
#endif


class GLTimeQuery
{
public:
    GLTimeQuery(const char* name);
    virtual ~GLTimeQuery();

    void Begin();
    void End();
    GLuint64 GetResult();
    void Commit();
    GLuint64 EndAndPrint();
    GLuint64 Print();
    void PrintSum();

    static void PrintAll();

    GLuint64 m_sum;
    int tick;
private:
    std::string m_name;

    GLuint m_time_query;
    GLint m_time_query_available;
    GLuint64 m_time_elapsed;

    static bool m_function_adresses_retrieved;
    static std::vector<GLTimeQuery*> m_time_queries;
};

class WarmUpHelper
{
public:
    static const std::string OCCLUSION_CULL;
    static const std::string MOTION_BLUR_TILE_MAX;
    static const std::string MOTION_BLUR_NEIGHBOR_MAX;
    static const std::string PARTICLE_SYSTEM_EMIT;
    static const std::string PARTICLE_SYSTEM_SIMULATE;

    static const bool s_use_timer_query = false;

    class WarmUpConfig
    {
    public:
        WarmUpConfig() { }
        virtual ~WarmUpConfig() {}

        WorkGroupValidator::WGConfig m_wg_config;
    };

    WarmUpHelper(GLB_Scene4 *scene, const std::string &wg_sizes);
    ~WarmUpHelper();

    WarmUpConfig *GetConfig(const std::string &name);
    void SetConfig(const std::string &name, WarmUpConfig *config);

    void BeginTimer();
    KCL::uint64 EndTimer();

    GLB_Scene4 *GetScene() const { return m_scene; }
    const WorkGroupValidator *GetValidator() const { return &m_validator; }

private:
    std::map<std::string, WarmUpConfig*> m_configs;
    KCL::uint64 m_start_time;

    GLB_Scene4 *m_scene;

    WorkGroupValidator m_validator;

    KCL::uint32 m_timer_query;
};

namespace GFXB4
{
KCL::uint32 Create2DTexture( KCL::uint32 max_mipmaps, bool linear, KCL::uint32 w, KCL::uint32 h, GLint format);
}


template<typename T, typename W>
std::string PARTICLE_BUFFERS_FILENAME(T anim_time, W name)
{
    std::stringstream filename_stream;
    filename_stream << "particle_buffers_" << anim_time << "ms_"<<name;
    return filename_stream.str();
}


#endif
