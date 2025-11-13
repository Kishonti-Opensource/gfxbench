/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "compute_reduction31.h"
#include "platform.h"
#include "opengl/glb_discard_functions.h"
#include "opengl/glb_opengl_state_manager.h"
#include "glb_scene_.h"
#include "opengl/glb_shader2.h"

#include "opengl/ubo_cpp_defines.h"
#include "ubo_luminance.h"

#include <ng/log.h>
#include <kcl_os.h>
#include <fstream>
#include <sstream>

using namespace GLB;

//#define SAVE_LUMINANCE_VALUES

ComputeReduction::ComputeReduction()
{
    m_scene = NULL;

    m_initialized = false;
    
    m_pass1_shader = NULL;
    m_pass2_shader = NULL;

    m_pass1_output_buffer = 0;
    m_pass2_output_buffer = 0;

    m_input_texture = 0;
    m_input_sampler = 0;

    m_adaptation_mode = ADAPTATION_ENABLED;
    m_mode_location = -1;
    m_predefined_luminance_location = -1;
}

ComputeReduction::~ComputeReduction()
{
    glDeleteBuffers(1, &m_pass1_output_buffer);
    glDeleteBuffers(1, &m_pass2_output_buffer);

    glDeleteSamplers(1, &m_input_sampler);

#ifdef SAVE_LUMINANCE_VALUES
    SaveLuminanceValues();
#endif
}

KCL::KCL_Status ComputeReduction::Init(KCL::uint32 width, KCL::uint32 height, GLB_Scene_ES2_ *scene)
{
    KCL::KCL_Status error_status = KCL::KCL_TESTERROR_NOERROR;
    m_initialized = false;

    m_scene = scene;

    m_input_width = width;
    m_input_height = height;
    
    // The number of compute thread in each dimension
    const int threads_x = PASS1_DISPATCH_X * PASS1_WORKGROUP_SIZE_X;
    const int threads_y = PASS1_DISPATCH_Y * PASS1_WORKGROUP_SIZE_Y;

    // The number of samples in each dimension
    int sample_per_thread_x = width / threads_x / DOWNSAMPLE;
    int sample_per_thread_y = height / threads_y / DOWNSAMPLE;
    sample_per_thread_x = KCL::Max(sample_per_thread_x, 1);
    sample_per_thread_y = KCL::Max(sample_per_thread_y, 1);
    
    // Count of the all samples
    KCL::uint32 all_samples = sample_per_thread_x * sample_per_thread_y * threads_x * threads_y;

    // Sample taps UV steps
    float step_u = 1.0f / float(sample_per_thread_x * threads_x);
    float step_v = 1.0f / float(sample_per_thread_y * threads_y);
    
    // Load the compute shaders
    GLBShaderBuilder sb;
    sb.AddDefineInt("WORK_GROUP_SIZE_X", PASS1_WORKGROUP_SIZE_X);
    sb.AddDefineInt("WORK_GROUP_SIZE_Y", PASS1_WORKGROUP_SIZE_Y);
    m_pass1_shader = sb.FileCs("hdr_reduction_pass1.shader").Build(error_status);
    if (error_status != KCL::KCL_TESTERROR_NOERROR)
    {
        return error_status;
    }
        
    sb.AddDefineInt("WORK_GROUP_SIZE", PASS2_WORKGROUP_SIZE);  
    m_pass2_shader = sb.FileCs("hdr_reduction_pass2.shader").Build(error_status);
    if (error_status != KCL::KCL_TESTERROR_NOERROR)
    {
        return error_status;
    }    

    // Set the constant uniforms
    OpenGLStateManager::GlUseProgram(m_pass1_shader->m_p);
    glUniform2f(glGetUniformLocation(m_pass1_shader->m_p, "step_uv"), step_u, step_v);   
    glUniform2i(glGetUniformLocation(m_pass1_shader->m_p, "sample_count"), sample_per_thread_x, sample_per_thread_y);
    glUniform2f(glGetUniformLocation(m_pass1_shader->m_p, "texel_center"), 0.5f / float(width), 0.5f / float(height));
      
    m_mode_location = glGetUniformLocation(m_pass2_shader->m_p, "adaptation_mode");       
    m_predefined_luminance_location = glGetUniformLocation(m_pass2_shader->m_p, "predefined_luminance");
    OpenGLStateManager::GlUseProgram(m_pass2_shader->m_p);            
    glUniform1f(glGetUniformLocation(m_pass2_shader->m_p, "texture_samples_inv"), 1.0f / all_samples); 
    OpenGLStateManager::GlUseProgram(0);
    
    glGenBuffers(1, &m_pass1_output_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_pass1_output_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, PASS2_WORKGROUP_SIZE * sizeof(float), NULL, GL_DYNAMIC_DRAW);
        
    // Create the buffers
    LuminanceBufferLayout luminance_buffer;   
    glGenBuffers(1, &m_pass2_output_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_pass2_output_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(LuminanceBufferLayout), &luminance_buffer, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glGenSamplers(1, &m_input_sampler);
    glSamplerParameteri(m_input_sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(m_input_sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glSamplerParameteri(m_input_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(m_input_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    SetAdaptationMode(m_scene->m_adaptation_mode);

    m_initialized = true;
    return KCL::KCL_TESTERROR_NOERROR;
}

void ComputeReduction::SetAdaptationMode(KCL::uint32 mode)
{
    m_adaptation_mode = mode;
    if (m_adaptation_mode == ADAPTATION_PREDEFINED)
    {
        if (m_luminance_map.empty())
        {
            LoadLuminanceValues();
        }
    }
}

void ComputeReduction::SetInputTexture(KCL::uint32 texture)
{
    m_input_texture = texture;
}

KCL::uint32 ComputeReduction::GetLuminanceBuffer()
{
    return m_pass2_output_buffer;
}

void ComputeReduction::Execute()
{
    if (!m_initialized || !m_input_texture)
    {
        return;
    }

 //   DebugBeginTimer();
   
    // First reduction pass
    OpenGLStateManager::GlUseProgram(m_pass1_shader->m_p);

    OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_input_texture);
    glBindSampler(0, m_input_sampler);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_pass1_output_buffer);

    glDispatchComputeProc(PASS1_DISPATCH_X , PASS1_DISPATCH_Y, 1);
       
    glMemoryBarrierProc(GL_SHADER_STORAGE_BARRIER_BIT);

    // Second reduction pass + adaptation
    OpenGLStateManager::GlUseProgram(m_pass2_shader->m_p);
    
    if (m_adaptation_mode == ADAPTATION_ENABLED && (m_scene->m_animation_time == 0 || m_scene->m_is_warmup))
    {      
        // If this is the first frame or we are in warmup disable the adaptation
        glUniform1i(m_mode_location, ADAPTATION_DISABLED);
    }
    else
    {
        // Set the adaptation mode
        glUniform1i(m_mode_location, m_adaptation_mode);
        if (m_adaptation_mode == ADAPTATION_PREDEFINED)
        {
            // Set the predefined luminance for this frame
            glUniform1f(m_predefined_luminance_location, m_luminance_map[m_scene->m_animation_time]);
        } 
    }    

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_pass2_output_buffer);
        
    glDispatchComputeProc(1, 1, 1);
    
    // The luminance buffer should be used as uniform block
    glMemoryBarrierProc(GL_UNIFORM_BARRIER_BIT);

    // Clean up
    OpenGLStateManager::GlUseProgram(0);   
    glBindSampler(0, 0);
    glBindTexture(GL_TEXTURE_2D, 0);   
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);      
    
#ifdef SAVE_LUMINANCE_VALUES
    if (!m_scene->m_is_warmup)
    {
        MapLuminanceValue();
    }
#endif

 //   DebugFinishTimer();
}

void ComputeReduction::MapLuminanceValue()
{
    int test = sizeof(LuminanceBufferLayout);

    // Read back the luminance buffer    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_pass2_output_buffer);
    vec4 *map_ptr = (vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(LuminanceBufferLayout), GL_MAP_READ_BIT);    
    if (map_ptr)
    {
        // Save the adaptation value
        m_luminance_map[m_scene->m_animation_time] = map_ptr->x;
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        
        if ((m_scene->m_animation_time % 100) == 0)
        {
            INFO("%d %d %f\n", m_scene->m_animation_time, m_luminance_map.size(), map_ptr->x);
        }
    }
    else
    {
        INFO("ComputeReduction::MapLuminanceValue - map ptr is null!!!");
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ComputeReduction::SaveLuminanceValues()
{
    std::ofstream file(KCL::AssetFile::GetDataPath() + GetLuminanceFilename(m_scene));
    std::map<KCL::uint32, float>::iterator it;
    for (it = m_luminance_map.begin(); it != m_luminance_map.end(); it++)
    {
        file << it->first << " " << it->second << std::endl;
    }
    file.close();
}

 void ComputeReduction::LoadLuminanceValues()
 {
     std::string filename = GetLuminanceFilename(m_scene);
     KCL::AssetFile file(filename);
     if (file.GetLastError())
     {
         NGLOG_ERROR("ComputeReduction - Can not open: %s", filename);
         return;
     }

     std::stringstream in_stream(file.GetBuffer());
          
     m_luminance_map.clear();
     while (!in_stream.eof())
     {
         KCL::uint32 frame_time = 0;
         float lum_value = 0.0f;

         if (!(in_stream >> frame_time))
         {
             return;
         }
         if (!(in_stream >> lum_value))
         {
             return;
         }

         m_luminance_map[frame_time] = lum_value;
     }

     if (m_luminance_map.empty())
     {
         NGLOG_ERROR("ComputeReduction - Can not parse: %s", filename);
     }
 }

 std::string ComputeReduction::GetLuminanceFilename(GLB_Scene_ES2_ *scene)
 {
     switch (scene->m_scene_version)
     {
     case KCL::SV_31:
         return "manhattan31/luminance_values.txt";

     case KCL::SV_40:
         return "car_chase/luminance_values.txt";

     default:
         return "unknown scene version";
     }
 }

#if 0 //DEBUG

static GLuint _timer_query = 0;
static float* _pixels = 0;
void ComputeReduction::DebugBeginTimer()
{    
    if (!_timer_query)
    {
        glGenQueries(1, &_timer_query);
    }
    glBeginQuery(GL_TIME_ELAPSED, _timer_query);
}

void ComputeReduction::DebugFinishTimer()
{
    glEndQuery(GL_TIME_ELAPSED);
    while(true)
    {
        int avail = 0;
        glGetQueryObjectiv(_timer_query, GL_QUERY_RESULT_AVAILABLE, &avail);
        if(avail)
        {
            break;
        }
    }

    GLuint64 elapsed_time = 0;
    glGetQueryObjectui64v(_timer_query, GL_QUERY_RESULT, &elapsed_time);
    elapsed_time /= 1000; // to micro seconds
  
    // Read back the luminance buffer
    float adaptive_lum = 0;
    float avg_lum = 0;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_pass2_output_buffer);
    float * result_ptr = (float*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 2 * sizeof(float), GL_MAP_READ_BIT);    
    if (result_ptr)
    {
        adaptive_lum = result_ptr[0];
        avg_lum = result_ptr[1];
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    }
    else
    {
        INFO("map ptr is null!!!!");
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Read back the input texture and calc the avg luminance
    float cpp_lum = -1.0f;
    cpp_lum = DebugGetAvgLuminance(m_input_texture, m_input_width, m_input_height);
       
    float err = fabsf(avg_lum - cpp_lum);
    printf("CPU avg: %f GPU adap: %f GPU avg: %f error: %f dtime: %f ms anim_time: %d \n", cpp_lum, adaptive_lum, avg_lum, err, float(elapsed_time) / 1000.0f, m_scene->m_animation_time);
}

float ComputeReduction::DebugGetAvgLuminance(KCL::uint32 texture, KCL::uint32 width, KCL::uint32 height)
{
    if (!_pixels)
    {
        _pixels = new float[width * height * 4];
    }

    glBindTexture(GL_TEXTURE_2D, texture);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, _pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    float avg_lum = 0;
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            unsigned int addr = (y * width + x) * 4;
            float r = (float(_pixels[addr + 0]));
            float g = (float(_pixels[addr + 1]));
            float b = (float(_pixels[addr + 2]));
                        
            // Manhattan 3.1
            r *= 4.0;
            g *= 4.0;
            b *= 4.0;           
            
            float lum = KCL::Vector3D::dot(KCL::Vector3D(r, g, b), KCL::Vector3D(0.299f, 0.587f, 0.114f));
            lum += 0.00000001f;
            avg_lum += logf(lum);
        }
    }
    avg_lum /= float(width * height);
    avg_lum -= -0.00000001f;
    avg_lum = expf(avg_lum);
    
    return avg_lum;
}

#endif