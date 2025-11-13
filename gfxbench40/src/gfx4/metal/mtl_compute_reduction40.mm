/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_compute_reduction40.h"
#include "metal/mtl_pipeline_builder.h"


#include "kcl_os.h"
#include "kcl_math3d.h"
#include <fstream>
#include <sstream>

typedef KCL::Vector2D hfloat2;
typedef KCL::Vector4D hfloat4;
typedef float hfloat;

struct int2
{
    int x,y ;
};


#include "hdr_reduction40.h"

using namespace MetalRender;

//#define SAVE_LUMINANCE_VALUES

ComputeReduction40::ComputeReduction40(id <MTLDevice> device)
{
    m_device = device ;
    
    m_scene = NULL;

    m_sample_count = 0;

    m_initialized = false;
    
    m_pass1_shader = NULL;
    m_pass2_shader = NULL;

    m_pass1_output_buffer = 0;
    m_pass2_output_buffer = 0;

    m_input_texture = 0;
    m_input_sampler = 0;

    m_adaptation_mode = ADAPTATION_ENABLED;
    m_predefined_luminance_location = -1;	
}

ComputeReduction40::~ComputeReduction40()
{
    releaseObj(m_pass1_output_buffer) ;
    releaseObj(m_pass2_output_buffer) ;
    releaseObj(m_input_sampler) ;

#ifdef SAVE_LUMINANCE_VALUES
    SaveLuminanceValues();
#endif
}

KCL::KCL_Status ComputeReduction40::Init(KCL::uint32 width, KCL::uint32 height, KRL_Scene *scene)
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
    sample_per_thread_x = width / threads_x / DOWNSAMPLE;
    sample_per_thread_y = height / threads_y / DOWNSAMPLE;
    sample_per_thread_x = KCL::Max(sample_per_thread_x, 1);
    sample_per_thread_y = KCL::Max(sample_per_thread_y, 1);
    
    // Count of the all samples
    m_sample_count = sample_per_thread_x * sample_per_thread_y * threads_x * threads_y;

    // Sample taps UV steps
    m_step_u = 1.0f / float(sample_per_thread_x * threads_x);
    m_step_v = 1.0f / float(sample_per_thread_y * threads_y);
    
    // Load the compute shaders
    MTLPipeLineBuilder sb;
    sb.AddDefineInt("WORK_GROUP_SIZE_X", PASS1_WORKGROUP_SIZE_X);
    sb.AddDefineInt("WORK_GROUP_SIZE_Y", PASS1_WORKGROUP_SIZE_Y);
    sb.AddDefineIVec2("SAMPLE_COUNT", sample_per_thread_x, sample_per_thread_y);
    m_pass1_shader = sb.ShaderFile("hdr_reduction_pass1_40.shader").Build(error_status);
    if (error_status != KCL::KCL_TESTERROR_NOERROR)
    {
        return error_status;
    }
    m_pass1_shader->IsThreadCountOk("reduction pass1", PASS1_WORKGROUP_SIZE_X*PASS1_WORKGROUP_SIZE_Y);
        
    // Load and setup pass2 shaders
    sb.AddDefineInt("WORK_GROUP_SIZE", PASS2_WORKGROUP_SIZE);
    m_pass2_shader = sb.ShaderFile("hdr_reduction_pass2_40.shader").Build(error_status);
    if (error_status != KCL::KCL_TESTERROR_NOERROR)
    {
        return error_status;
    }
    m_pass2_shader->IsThreadCountOk("reduction pass2", PASS2_WORKGROUP_SIZE);
    
    // Create the buffers
#if TARGET_OS_EMBEDDED
    m_pass1_output_buffer = [m_device newBufferWithLength:PASS2_WORKGROUP_SIZE * sizeof(float)
                                              options:MTLResourceOptionCPUCacheModeDefault];
    
    m_pass2_output_buffer = [m_device newBufferWithLength:sizeof(KCL::Vector4D)
                                                  options:MTLResourceOptionCPUCacheModeDefault];
#else
    m_pass1_output_buffer = [m_device newBufferWithLength:PASS2_WORKGROUP_SIZE * sizeof(float) options:MTLResourceStorageModeManaged];
    m_pass2_output_buffer = [m_device newBufferWithLength:sizeof(KCL::Vector4D) options:MTLResourceStorageModeManaged];
#endif
    
    MTLSamplerDescriptor *inputSamplerDesc = [[MTLSamplerDescriptor alloc] init];
    inputSamplerDesc.minFilter = MTLSamplerMinMagFilterLinear;
    inputSamplerDesc.magFilter = MTLSamplerMinMagFilterLinear;
    inputSamplerDesc.mipFilter = MTLSamplerMipFilterNotMipmapped;
    
    inputSamplerDesc.sAddressMode = MTLSamplerAddressModeRepeat;
    inputSamplerDesc.tAddressMode = MTLSamplerAddressModeRepeat;
    inputSamplerDesc.rAddressMode = MTLSamplerAddressModeClampToEdge;
    
    m_input_sampler = [m_device newSamplerStateWithDescriptor:inputSamplerDesc];
    
    SetAdaptationMode(m_scene->m_adaptation_mode);

    m_initialized = true;
    return KCL::KCL_TESTERROR_NOERROR;
}

void ComputeReduction40::SetAdaptationMode(KCL::uint32 mode)
{
    m_adaptation_mode = mode;
    if (m_adaptation_mode == ADAPTATION_PREDEFINED)
    {
        LoadLuminanceValues();
    }
}

void ComputeReduction40::SetInputTexture(id <MTLTexture> texture)
{
    m_input_texture = texture;
}

id <MTLBuffer> ComputeReduction40::GetLuminanceBuffer()
{
    return m_pass2_output_buffer;
}

void ComputeReduction40::Execute(id <MTLCommandBuffer> command_buffer, const UBOFrame &ubo_frame)
{
    if (!m_initialized || !m_input_texture)
    {
        return;
    }
    
    id <MTLComputeCommandEncoder> compute_encoder = [command_buffer computeCommandEncoder] ;
	compute_encoder.label = @"Compute Reduction";
   
    // First reduction pass
    m_pass1_shader->SetAsCompute(compute_encoder) ;
    
    // Set pass1 the constant uniforms
    hdr_reduction_pass1_uniforms hrp1u ;
    hrp1u.step_uv = KCL::Vector2D(m_step_u, m_step_v) ;
    hrp1u.texel_center = KCL::Vector2D(0.5f / float(m_input_width), 0.5f / float(m_input_height)) ;
    hrp1u.sample_count.x = sample_per_thread_x ;
    hrp1u.sample_count.y = sample_per_thread_y ;
    
    [compute_encoder setBytes:&hrp1u length:sizeof(hdr_reduction_pass1_uniforms) atIndex:HDR_REDUCTION_PASS1_UNIFORMS_BFR_SLOT] ;

    [compute_encoder setTexture:m_input_texture atIndex:HDR_REDUCTION_PASS1_INPUT_TEX_SLOT] ;
    [compute_encoder setSamplerState:m_input_sampler atIndex:HDR_REDUCTION_PASS1_INPUT_SAMPLER_SLOT] ;
    [compute_encoder setBuffer:m_pass1_output_buffer offset:0 atIndex:HDR_REDUCTION_PASS1_OUTPUT_BFR_SLOT] ;

    MTLSize threadsPerGroup = { PASS1_WORKGROUP_SIZE_X, PASS1_WORKGROUP_SIZE_Y, 1 };
    MTLSize numThreadgroups = { PASS1_DISPATCH_X, PASS1_DISPATCH_Y, 1};
    
    [compute_encoder dispatchThreadgroups:numThreadgroups
                    threadsPerThreadgroup:threadsPerGroup] ;


    // Second reduction pass + adaptation
    KCL::uint32 mode = m_adaptation_mode;
    if (m_adaptation_mode == ADAPTATION_ENABLED && (m_scene->m_animation_time == 0 || m_scene->m_is_warmup))
    {
        // If this is the first frame or we are in warmup disable the adaptation
        mode = ADAPTATION_DISABLED;
    }

    m_pass2_shader->SetAsCompute(compute_encoder) ;
    
    hdr_reduction_pass2_uniforms hrp2u ;
    hrp2u.time_dt_pad2        = ubo_frame.time_dt_pad2;
    hrp2u.EFW_tau             = ubo_frame.EFW_tau;
    hrp2u.texture_samples_inv = 1.0f / m_sample_count ;
    hrp2u.adaptation_mode     = mode ;  // 0: adaptive, 1 - disabled, 2 - predefined
#ifdef OPT_TEST_GFX40
	hrp2u.exposure_bloomthreshold_tone_map_white_pad = ubo_frame.exposure_bloomthreshold_pad2;
#endif

	if (mode == ADAPTATION_PREDEFINED)
	{
		// Set the predefined luminance for this frame
        hrp2u.predefined_luminance = m_luminance_map[m_scene->m_animation_time];
	}
    
    [compute_encoder setBytes:&hrp2u length:sizeof(hdr_reduction_pass2_uniforms) atIndex:HDR_REDUCTION_PASS2_UNIFORMS_BFR_SLOT] ;
    [compute_encoder setBuffer:m_pass1_output_buffer offset:0 atIndex:HDR_REDUCTION_PASS2_INPUT_BFR_SLOT] ;
    [compute_encoder setBuffer:m_pass2_output_buffer offset:0 atIndex:HDR_REDUCTION_PASS2_OUTPUT_BFR_SLOT] ;
    
    threadsPerGroup = { PASS2_WORKGROUP_SIZE, 1, 1 };
    numThreadgroups = { 1, 1, 1};
    
    [compute_encoder dispatchThreadgroups:numThreadgroups
                     threadsPerThreadgroup:threadsPerGroup] ;
    
    [compute_encoder endEncoding] ;
    
#ifdef SAVE_LUMINANCE_VALUES
    if (!m_scene->m_is_warmup)
    {
        MapLuminanceValue(command_buffer);
    }
#endif

    //DebugFinishTimer(command_buffer);
}

void ComputeReduction40::MapLuminanceValue(id <MTLCommandBuffer> command_buffer)
{
#if !TARGET_OS_IPHONE
    id <MTLBlitCommandEncoder> blit_command_encoder = [command_buffer blitCommandEncoder];
    [blit_command_encoder synchronizeResource:m_pass2_output_buffer];
    [blit_command_encoder endEncoding];
#endif
    
    // wait to finish the commandbuffer
    [command_buffer commit];
    [command_buffer waitUntilCompleted];
    
    // Read back the luminance buffer
    KCL::Vector4D* map_ptr = (KCL::Vector4D*)[m_pass2_output_buffer contents];
    
    if (map_ptr)
    {
        // Save the adaptation value
        m_luminance_map[m_scene->m_animation_time] = map_ptr->x;
        
        if ((m_scene->m_animation_time % 100) == 0)
        {
            INFO("%d %d %f\n", m_scene->m_animation_time, m_luminance_map.size(), map_ptr->x);
        }
    }
    else
    {
        INFO("ComputeReduction::MapLuminanceValue - map ptr is null!!!");
    }
}

void ComputeReduction40::SaveLuminanceValues()
{
    std::ofstream file(KCL::AssetFile::GetDataPath() + GetLuminanceFilename(m_scene));
    std::map<KCL::uint32, float>::iterator it;
    for (it = m_luminance_map.begin(); it != m_luminance_map.end(); it++)
    {
        file << it->first << " " << it->second << std::endl;
    }
    file.close();
}

 void ComputeReduction40::LoadLuminanceValues()
 {
     KCL::AssetFile file(GetLuminanceFilename(m_scene));   
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
 }

 std::string ComputeReduction40::GetLuminanceFilename(KRL_Scene *scene)
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


static float* _float_pixels = 0;
static KCL::uint8* _byte_pixels = 0;
void ComputeReduction::DebugBeginTimer()
{    

}

void ComputeReduction::DebugFinishTimer(id <MTLCommandBuffer> command_buffer)
{
#if !TARGET_OS_IPHONE
    id <MTLBlitCommandEncoder> blit_command_encoder = [command_buffer blitCommandEncoder];
    [blit_command_encoder synchronizeResource:m_pass2_output_buffer];
    [blit_command_encoder synchronizeResource:m_input_texture];
    [blit_command_encoder endEncoding];
#endif
    
    [command_buffer commit];
    [command_buffer waitUntilCompleted];
                                                      
    
    // Read back the luminance buffer
    float adaptive_lum = 0;
    float avg_lum = 0;
    float * result_ptr = (float*)[m_pass2_output_buffer contents];
    if (result_ptr)
    {
        adaptive_lum = result_ptr[0];
        avg_lum = result_ptr[1];
    }
    else
    {
        INFO("map ptr is null!!!!");
    }

    // Read back the input texture and calc the avg luminance
    float cpp_lum = -1.0f;
    cpp_lum = DebugGetAvgLuminance(m_input_texture, m_input_width, m_input_height);
       
    float err = fabsf(avg_lum - cpp_lum);
    printf("CPU avg: %f GPU adap: %f GPU avg: %f error: %f ms anim_time: %d \n", cpp_lum, adaptive_lum, avg_lum, err, m_scene->m_animation_time);
}

float ComputeReduction::DebugGetAvgLuminance(id <MTLTexture> texture, KCL::uint32 width, KCL::uint32 height)
{
    if (!_float_pixels)
    {
        _float_pixels = new float[width * height * 4];
        _byte_pixels  = new KCL::uint8[width * height * 4];
    }
    
    if (texture.pixelFormat != MTLPixelFormatRGBA8Unorm)
    {
        // only works with MTLPixelFormatRGBA8Unorm
        // mtl_scene_30.mm line 246
        assert(0);
    }
    
    [texture getBytes:_byte_pixels
          bytesPerRow:width*4*sizeof(KCL::uint8)
           fromRegion:MTLRegionMake2D(0, 0, texture.width, texture.height)
          mipmapLevel:0];
    
    for (int i = 0; i < 4*width*height; i++)
    {
        _float_pixels[i] = static_cast<float>(_byte_pixels[i])/255.0f;
    }
    
    float avg_lum = 0;
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            unsigned int addr = (y * width + x) * 4;
            float r = (float(_float_pixels[addr + 0]));
            float g = (float(_float_pixels[addr + 1]));
            float b = (float(_float_pixels[addr + 2]));
                        
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
