/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "alu2_base.h"

#include "kcl_io.h"
#include "kcl_image.h"
#include "kcl_math3d.h"

#include <vector>
#include <sstream>
#include <limits.h> // UINT_MAX

#include "kcl_base.h"
#include "mtl_pipeline.h"
#include "graphics/metalgraphicscontext.h"
#include "metal/mtl_pipeline_builder.h"
#include "metal/fbo.h"
#include "metal/fbo_metal.h"



class ALUTest2_Metal : public ALUTest2_Base
{
public:
    ALUTest2_Metal(const GlobalTestEnvironment* const gte);
    virtual ~ALUTest2_Metal();
    
protected:
    virtual KCL::KCL_Status init ();
    virtual bool render ();
    
    virtual void FreeResources();
    
private:
    MTLVertexDescriptor* m_LightsVertexDescriptor;
    MTLVertexDescriptor* m_EmissiveVertexDescriptor;
    id <MTLBuffer> m_vertex_buffer;
    id <MTLBuffer> m_index_buffer;
    
    id <MTLTexture> m_color_texture;
    id <MTLTexture> m_normal_texture;
    id <MTLTexture> m_depth_texture;
    id <MTLSamplerState> m_sampler;
    
    MetalRender::Pipeline *m_shaderLights;
    MetalRender::Pipeline *m_shaderAmbEmit;
    KCL::uint32 m_uniform_light_pos_array;
    KCL::uint32 m_uniform_light_color_array;
    
    id <MTLTexture> LoadTexture(const char * filename);
    void CreateLights(const float * depth_data);
    
    void SaveScene();
    
    id <MTLDevice> m_device;
    id <MTLCommandQueue> m_command_queue;
};



#define NUM_LIGHTS_PER_LOOP 16


struct Alu2Uniforms
{
    KCL::Vector4D view_pos;
    KCL::Vector4D depth_parameters;
    KCL::Vector4D light_pos_atten_array[NUM_LIGHTS_PER_LOOP];
    KCL::Vector4D light_color_array[NUM_LIGHTS_PER_LOOP];
};


//
// Created from Manhattan frame: 59500ms
//
ALUTest2_Metal::ALUTest2_Metal(const GlobalTestEnvironment* const gte) : ALUTest2_Base(gte)
{
	m_LightsVertexDescriptor = nil;
    m_EmissiveVertexDescriptor = nil;
	m_index_buffer = 0;
	m_vertex_buffer = 0;

	m_color_texture = 0;
	m_normal_texture = 0;
	m_depth_texture = 0;
	m_sampler = 0;

	m_shaderLights = 0;
    m_shaderAmbEmit = 0;

    m_uniform_light_pos_array = 0;
    m_uniform_light_color_array = 0;
    
    MetalGraphicsContext* context = (MetalGraphicsContext*)gte->GetGraphicsContext() ;
    m_device = context->getDevice();
    m_command_queue = context->getMainCommandQueue();
}

ALUTest2_Metal::~ALUTest2_Metal()
{
	FreeResources();
}

KCL::KCL_Status ALUTest2_Metal::init()
{			
	KCL::KCL_Status init_status = KCL::KCL_TESTERROR_NOERROR;

	m_score = 0;
	
	// Create near sampler for G buffer sampling
    MTLSamplerDescriptor *samplerDesc = [[MTLSamplerDescriptor alloc] init];
    samplerDesc.minFilter = MTLSamplerMinMagFilterNearest;
    samplerDesc.magFilter = MTLSamplerMinMagFilterNearest;
    samplerDesc.mipFilter = MTLSamplerMipFilterNotMipmapped;
    
    samplerDesc.sAddressMode = MTLSamplerAddressModeRepeat;
    samplerDesc.tAddressMode = MTLSamplerAddressModeRepeat;
    samplerDesc.rAddressMode = MTLSamplerAddressModeClampToEdge;
    
    m_sampler = [m_device newSamplerStateWithDescriptor:samplerDesc];
	
	// Load the textures
	m_color_texture = LoadTexture("alu2/alu2_albedo_emission.png");	
	if (!m_color_texture)
	{
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}	
	m_normal_texture = LoadTexture("alu2/alu2_normal_normalized.png");
	if (!m_normal_texture)
	{
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}		

	// Load the depth texture
	KCL::AssetFile depth_file("alu2/alu2_depth.raw");
	if(depth_file.GetLastError())
	{
		INFO("ALUTest2 - ERROR: alu2/alu2_depth.raw not found!\n");
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}

	const float * depth_data = (float*) depth_file.GetBuffer(); 

    // Using 32 bit float depth instead of 24bit (24bit depth not supported on all Metal platform)
    MTLTextureDescriptor *depth_tex_desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                                                              width:G_BUFFER_WIDTH
                                                                                             height:G_BUFFER_HEIGHT
                                                                                          mipmapped:false];
#if !TARGET_OS_IPHONE
    depth_tex_desc.storageMode = MTLStorageModePrivate;
#endif
    
    m_depth_texture = [m_device newTextureWithDescriptor:depth_tex_desc];
    
#if TARGET_OS_IPHONE
    [m_depth_texture replaceRegion:MTLRegionMake3D(0, 0, 0, G_BUFFER_WIDTH, G_BUFFER_HEIGHT, 1)
               mipmapLevel:0
                 withBytes:depth_data
               bytesPerRow:4*G_BUFFER_WIDTH];
#else
    // On OSX the depth texture must be private, thus we unable to upload the data directly
    @autoreleasepool {
        id <MTLBuffer> temp_buffer = [m_device newBufferWithBytes:depth_data length:sizeof(float) * G_BUFFER_WIDTH*G_BUFFER_HEIGHT options:MTLStorageModeManaged];
        
        id <MTLCommandBuffer> command_buffer = [m_command_queue commandBuffer];
        id <MTLBlitCommandEncoder> blit_encodeer = [command_buffer blitCommandEncoder];
        
        [blit_encodeer copyFromBuffer:temp_buffer
                         sourceOffset:0
                    sourceBytesPerRow:4*G_BUFFER_WIDTH
                  sourceBytesPerImage:4*G_BUFFER_WIDTH*G_BUFFER_HEIGHT
                           sourceSize:MTLSizeMake(G_BUFFER_WIDTH, G_BUFFER_HEIGHT, 1)
                            toTexture:m_depth_texture
                     destinationSlice:0
                     destinationLevel:0
                    destinationOrigin:MTLOriginMake(0, 0, 0)];
        
        [blit_encodeer endEncoding];
        [command_buffer commit];
        [command_buffer waitUntilCompleted];
    }
#endif

	// Load the scene
	init_status = LoadScene();
	if (init_status != KCL::KCL_TESTERROR_NOERROR)
	{
		return init_status;
	}
    
    struct Vertex
    {
        KCL::Vector4D pos_uv;
        KCL::Vector4D corners; // the corners of the view frustum
    };
    
    // Lights vao
    {
        NSUInteger stride = sizeof(Vertex);
        m_LightsVertexDescriptor = [[MTLVertexDescriptor alloc] init];
        
        m_LightsVertexDescriptor.attributes[0].format = MTLVertexFormatFloat4;
        m_LightsVertexDescriptor.attributes[0].bufferIndex = 0;
        m_LightsVertexDescriptor.attributes[0].offset = 0;
        
        m_LightsVertexDescriptor.attributes[1].format = MTLVertexFormatFloat4;
        m_LightsVertexDescriptor.attributes[1].bufferIndex = 0;
        m_LightsVertexDescriptor.attributes[1].offset = sizeof(KCL::Vector4D);
        
        m_LightsVertexDescriptor.layouts[0].stride = stride ;
        m_LightsVertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
    }
    
    // Emissive vao
    {
        NSUInteger stride = sizeof(Vertex);
        m_EmissiveVertexDescriptor = [[MTLVertexDescriptor alloc] init];
        
        m_EmissiveVertexDescriptor.attributes[0].format = MTLVertexFormatFloat4;
        m_EmissiveVertexDescriptor.attributes[0].bufferIndex = 0;
        m_EmissiveVertexDescriptor.attributes[0].offset = 0;
        
        m_EmissiveVertexDescriptor.layouts[0].stride = stride ;
        m_EmissiveVertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
    }


	// Load the shaders
    bool force_highp = GetSetting().m_force_highp;
#if !TARGET_OS_IPHONE
    force_highp = true;
#endif
    MTLPipeLineBuilder::SetScene(KCL::SV_INVALID, nullptr);
    MTLPipeLineBuilder sb;
	m_shaderLights = sb
		.AddShaderDir("shaders_mtl/lowlevel2/")
        .AddDefineInt("DO_EMISSIVE_AND_AMBIENT", 0)
        .AddDefineInt("NUM_LIGHTS", NUM_LIGHTS_PER_LOOP)
		.AddDefineInt("NUM_COLORS", m_light_color_array.size())
		.ShaderFile("alu2.shader")
        .SetVertexLayout(m_LightsVertexDescriptor)
        .ForceHighp(force_highp)
        .SetTypeByPixelFormat(MTLPixelFormatBGRA8Unorm)
        .SetBlendType(MetalRender::Pipeline::BlendType::ONE_X_ONE)
		.Build(init_status);
	if (init_status != KCL::KCL_TESTERROR_NOERROR)
	{
		INFO("ALUTest2 - ERROR: Can not load ALU Lights shader!\n");
		return init_status;
	}
    
    m_shaderAmbEmit = sb
        .AddShaderDir("shaders_mtl/lowlevel2/")
        .AddDefineInt("DO_EMISSIVE_AND_AMBIENT", 1)
        .ShaderFile("alu2.shader")
        .SetVertexLayout(m_EmissiveVertexDescriptor)
        .ForceHighp(force_highp)
        .SetTypeByPixelFormat(MTLPixelFormatBGRA8Unorm)
        .SetBlendType(MetalRender::Pipeline::BlendType::ONE_X_ZERO)
        .Build(init_status);
    if (init_status != KCL::KCL_TESTERROR_NOERROR)
    {
        INFO("ALUTest2 - ERROR: Can not load Emissive shader!\n");
		return init_status;
	}

	// Calculate the aspect ratio and the uv scaling
	float viewport_width = getViewportWidth();
	float viewport_height = getViewportHeight();				
	bool landscape;
	KCL::Vector4D scaled_uv = FitViewportToGBuffer(viewport_width, viewport_height, landscape);

	// Upload the vertex data
	Vertex vertices[4];
	
    vertices[0].pos_uv.z = scaled_uv.x; vertices[0].pos_uv.w = scaled_uv.y;
	vertices[1].pos_uv.z = scaled_uv.x; vertices[1].pos_uv.w = scaled_uv.w;
	vertices[2].pos_uv.z = scaled_uv.z; vertices[2].pos_uv.w = scaled_uv.y;
	vertices[3].pos_uv.z = scaled_uv.z; vertices[3].pos_uv.w = scaled_uv.w;		
				
	vertices[0].corners = KCL::Vector4D(m_camera.corners[0], 0.0f);
	vertices[1].corners = KCL::Vector4D(m_camera.corners[2], 0.0f);
	vertices[2].corners = KCL::Vector4D(m_camera.corners[1], 0.0f);
	vertices[3].corners = KCL::Vector4D(m_camera.corners[3], 0.0f);

	if (landscape)
	{		
	    vertices[0].pos_uv.x = -1; vertices[0].pos_uv.y = -1;
	    vertices[1].pos_uv.x = -1; vertices[1].pos_uv.y = 1;
	    vertices[2].pos_uv.x = 1; vertices[2].pos_uv.y = -1;
	    vertices[3].pos_uv.x = 1; vertices[3].pos_uv.y = 1;	
	}
	else
	{    
        vertices[0].pos_uv.x = 1; vertices[0].pos_uv.y = -1;
	    vertices[1].pos_uv.x = -1; vertices[1].pos_uv.y = -1;
	    vertices[2].pos_uv.x = 1; vertices[2].pos_uv.y = 1;
	    vertices[3].pos_uv.x = -1; vertices[3].pos_uv.y = 1;
	}

#if TARGET_OS_IPHONE
    m_vertex_buffer = [m_device newBufferWithBytes:vertices length:sizeof(vertices) options:0];
#else
    m_vertex_buffer = [m_device newBufferWithBytes:vertices length:sizeof(vertices) options:MTLStorageModeManaged];
#endif
	
	static const KCL::uint16 billboardIndices[] = { 0, 1, 2, 1, 2, 3 };
#if TARGET_OS_IPHONE
    m_index_buffer = [m_device newBufferWithBytes:billboardIndices length:sizeof(KCL::uint16) * 6 options:0];
#else
    m_index_buffer = [m_device newBufferWithBytes:billboardIndices length:sizeof(KCL::uint16) * 6 options:MTLStorageModeManaged];
#endif
	
	return init_status;
}


id <MTLTexture> ALUTest2_Metal::LoadTexture(const char * filename)
{
	KCL::Image image;
	if (!image.load(filename))
	{
		INFO("Error in ALUTest2::LoadTexture: Can not load image: %s", filename);
		return 0;
	}
	image.flipY();

    id <MTLTexture> texture = nil;
    
    MTLTextureDescriptor *tex_desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                        width:G_BUFFER_WIDTH
                                                                                       height:G_BUFFER_HEIGHT
                                                                                    mipmapped:false];
    
    texture = [m_device newTextureWithDescriptor:tex_desc];
    
    [texture replaceRegion:MTLRegionMake3D(0, 0, 0, G_BUFFER_WIDTH, G_BUFFER_HEIGHT, 1)
               mipmapLevel:0
                 withBytes:image.getData()
               bytesPerRow:4*G_BUFFER_WIDTH];
    
	return texture;
}

bool ALUTest2_Metal::render()
{
@autoreleasepool
{
    id <MTLCommandBuffer> command_buffer = [m_command_queue commandBuffer];
    
    id <MTLTexture> frameBufferTexture = dynamic_cast<GLB::FBOMetalBase*>(GLB::FBO::GetLastBind())->GetTexture() ;
    
    MTLRenderPassDescriptor * render_pass_desc = [MTLRenderPassDescriptor renderPassDescriptor];
    render_pass_desc.colorAttachments[0].texture = frameBufferTexture ;
    render_pass_desc.colorAttachments[0].loadAction = MTLLoadActionClear ;
    render_pass_desc.colorAttachments[0].clearColor = MTLClearColorMake(0.0f,0.0f,0.0f,0.0f);
    render_pass_desc.colorAttachments[0].storeAction = MTLStoreActionStore ;
    render_pass_desc.depthAttachment.texture = nil ;
    render_pass_desc.depthAttachment.loadAction = MTLLoadActionClear ;
    render_pass_desc.depthAttachment.storeAction = MTLStoreActionDontCare;
    
    // Get a render encoder
    id <MTLRenderCommandEncoder> encoder = [command_buffer renderCommandEncoderWithDescriptor:render_pass_desc];
    
    // First pass: Render the ambient/emissive pixels
    m_shaderAmbEmit->Set(encoder);
    [encoder setFragmentSamplerState:m_sampler atIndex:0];
    [encoder setFragmentTexture:m_color_texture atIndex:0];
    [encoder setFragmentTexture:m_normal_texture atIndex:1];
    [encoder setFragmentTexture:m_depth_texture atIndex:2];

    [encoder setVertexBuffer:m_vertex_buffer offset:0 atIndex:0];

    [encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                        indexCount:6
                         indexType:MTLIndexTypeUInt16
                       indexBuffer:m_index_buffer
                 indexBufferOffset:0];
    
    // Second pass: Render and blend the lights
    m_shaderLights->Set(encoder);
    
    Alu2Uniforms fu;
    
    // Setup the camera uniforms
    fu.view_pos = m_camera.view_pos;
    fu.depth_parameters = m_camera.depth_parameters;
    
    for (KCL::uint32 i = 0; i < m_animated_pos_array.size(); i += NUM_LIGHTS_PER_LOOP)
    {
        KCL::uint32 num_lights = m_animated_pos_array.size()-i;
        if( num_lights > NUM_LIGHTS_PER_LOOP )
            num_lights = NUM_LIGHTS_PER_LOOP;
        
        // Set the lights uniforms
        memcpy(fu.light_pos_atten_array[0].v, m_animated_pos_array[i].v, num_lights*sizeof(KCL::Vector4D));
        for (int j = 0; j < num_lights; j++)
        {
            fu.light_color_array[j] = KCL::Vector4D(m_light_color_array[i+j],1.0);
        }
    
        [encoder setFragmentBytes:&fu length:sizeof(Alu2Uniforms) atIndex:1];

        [encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                            indexCount:6
                             indexType:MTLIndexTypeUInt16
                           indexBuffer:m_index_buffer
                     indexBufferOffset:0];
    }

    [encoder endEncoding];
    [command_buffer commit];
    
	return true;
}
}

void ALUTest2_Metal::FreeResources()
{
    // finish all GPU work before dealloc resources
    MetalRender::Finish();
    
    MetalRender::Pipeline::ClearCashes();
	m_shaderLights = NULL;
	m_shaderAmbEmit = NULL;
	
	releaseObj(m_color_texture);
	releaseObj(m_normal_texture);
	releaseObj(m_depth_texture);

    releaseObj(m_sampler);
	
	releaseObj(m_LightsVertexDescriptor);
    releaseObj(m_EmissiveVertexDescriptor);
	releaseObj(m_vertex_buffer);
    releaseObj(m_index_buffer);
	
	ALUTest2_Base::FreeResources();
}


GLB::TestBase *CreateALU2TestMetal(const GlobalTestEnvironment* const gte)
{
    return new ALUTest2_Metal(gte);
}

