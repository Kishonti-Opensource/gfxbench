/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "fill2_base.h"

#include "metal/mtl_texture.h"
#include "metal/mtl_pipeline.h"
#include <limits.h> // UINT_MAX
#include "kcl_base.h"
#include <vector>
#include "metal/mtl_pipeline_builder.h"
#include "graphics/metalgraphicscontext.h"
#include "metal/fbo.h"
#include "metal/fbo_metal.h"

struct VertexUniforms
{
    KCL::Vector4D offsets; // pos_x-y offsets, u-v offsets
    float rotation;
    float aspectRatio;
    
    float _pad1;
    float _pad2;
};

class CompressedFillTest2_Metal : public CompressedFillTest2_Base
{
public:
	CompressedFillTest2_Metal(const GlobalTestEnvironment* const gte);
	virtual ~CompressedFillTest2_Metal();

protected:
	virtual KCL::KCL_Status init ();
	virtual bool render();
	virtual void renderApiFinish();

	virtual void FreeResources();

private:
	struct RenderPass
	{
		MetalRender::Texture * textures[4];
		KCL::Vector4D offsets;
		RenderPass()
		{
			Clear();
		}
		void Clear()
		{
			textures[0] = textures[1] = textures[2] = textures[3] = NULL;
		}
	};
	static const KCL::uint32 m_render_pass_count = 4;
	RenderPass m_render_passes[m_render_pass_count];
	
	void CreateDepthTexture();
	void CreateFullScreenTextures(MetalRender::Texture* src_texture1, MetalRender::Texture* src_texture2);
	MetalRender::Texture * CreateTexture(KCL::Image * image);
	
	std::vector<MetalRender::Texture *> m_textures;

	id <MTLBuffer> m_vertex_buffer_portrait;
    id <MTLBuffer> m_vertex_buffer_landscape;
	id <MTLBuffer> m_index_buffer;
    id <MTLBuffer> m_vertex_buffer;

	id <MTLTexture> m_fullscreen_background;
	id <MTLTexture> m_fullscreen_mask;
	MetalRender::Texture * m_cube_texture;
	id <MTLTexture> m_depth_texture;
	id <MTLSamplerState> m_near_sampler;

	MetalRender::Pipeline *m_shader;
	MetalRender::Pipeline *m_shader_cube;
	MetalRender::Pipeline *m_shader_resize;
    
    float m_aspectRatio;

	KCL::uint32 m_viewport_width;
	KCL::uint32 m_viewport_height;
    
    id <MTLDevice> m_device;
    id <MTLCommandQueue> m_command_queue;
};


CompressedFillTest2_Metal::CompressedFillTest2_Metal(const GlobalTestEnvironment* const gte) : CompressedFillTest2_Base(gte)
{
	m_vertex_buffer_portrait = nil;
    m_vertex_buffer_landscape = nil;
    m_vertex_buffer = nil;
    m_index_buffer = nil;

	m_fullscreen_background = 0;
	m_fullscreen_mask = 0;
	m_cube_texture = NULL;
	m_depth_texture = 0;
	m_near_sampler = 0;

	m_shader = NULL;
	m_shader_cube = NULL;
	m_shader_resize = NULL;

	m_viewport_width = 0;
	m_viewport_height = 0;
    
    m_aspectRatio = 1.0f;
    
    MetalGraphicsContext* context = (MetalGraphicsContext*)gte->GetGraphicsContext() ;
    m_device = context->getDevice();
    m_command_queue = context->getMainCommandQueue();
}

CompressedFillTest2_Metal::~CompressedFillTest2_Metal()
{
	FreeResources();
}


void CompressedFillTest2_Metal::renderApiFinish()
{
    MetalRender::Finish();
}


KCL::KCL_Status CompressedFillTest2_Metal::init()
{
	m_viewport_width = getViewportWidth();
	m_viewport_height = getViewportHeight();

    bool landscape = m_window_height <= m_window_width || (GetSetting().GetScreenMode() != 0) || GetSetting().m_virtual_resolution;

    // Create the vertex layout descriptor
    NSUInteger stride = 4 * sizeof(float); // distance between the beginning of vertices
    MTLVertexDescriptor* vertex_descriptor = [[MTLVertexDescriptor alloc] init];
    
    vertex_descriptor.attributes[0].format = MTLVertexFormatFloat4;
    vertex_descriptor.attributes[0].bufferIndex = 0;
    vertex_descriptor.attributes[0].offset = 0;
    
    vertex_descriptor.layouts[0].stride = stride ;
    vertex_descriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
		
	// Load the shaders
    MTLPipeLineBuilder::SetScene(KCL::SV_INVALID, nullptr);

	KCL::KCL_Status error = KCL::KCL_TESTERROR_NOERROR;
	MTLPipeLineBuilder sb;

    bool force_highp = GetSetting().m_force_highp;
#if !TARGET_OS_IPHONE
    force_highp = true;
#endif
	m_shader = sb
		.AddShaderDir("shaders_mtl/lowlevel2/")
		.ShaderFile("compressedfill2.shader")
        .ForceHighp(force_highp)
        .SetTypeByPixelFormat(MTLPixelFormatBGRA8Unorm)
        .SetBlendType(MetalRender::Pipeline::BlendType::CONSTANT_ALPHA_X_ONE_MINUS_CONSTANT_ALPHA)
        .SetVertexLayout(vertex_descriptor)
		.Build(error);
	if (error != KCL::KCL_TESTERROR_NOERROR)
	{
		return error;
	}

	m_shader_cube = sb
		.AddShaderDir("shaders_mtl/lowlevel2/")
		.AddDefine("CUBE_DEPTH_PASS")
		.ShaderFile("compressedfill2.shader")
        .ForceHighp(force_highp)
        .SetTypeByPixelFormat(MTLPixelFormatBGRA8Unorm)
        .SetBlendType(MetalRender::Pipeline::BlendType::SRC_ALPHA_X_ONE_MINUS_SRC_ALPHA___X___ONE_X_ZERO)
        .SetVertexLayout(vertex_descriptor)
		.Build(error);
	if (error != KCL::KCL_TESTERROR_NOERROR)
	{
		return error;
	}

	m_shader_resize = sb
		.AddShaderDir("shaders_mtl/lowlevel2/")
		.AddDefine("RESIZE_PASS")
		.ShaderFile("compressedfill2.shader")
        .ForceHighp(force_highp)
        .SetVertexLayout(vertex_descriptor)
		.Build(error);
	if (error != KCL::KCL_TESTERROR_NOERROR)
	{
		return error;
	}

	// Setup the shaders
    if (landscape)
    {
        m_aspectRatio = m_viewport_width / m_viewport_height;
    }
    else
    {
        m_aspectRatio = m_viewport_height / m_viewport_width;
    }

	// Load the images	
	std::string imageFolder;
    if ([m_device supportsFamily:MTLGPUFamilyApple2] )
    {
		imageFolder = "fill2/images_ETC2";	
	}
    else
	{
		imageFolder = "fill2/images_DXT5";	
	}
		
	KCL::Image image_2048;
	KCL::Image image_1024;
	KCL::Image image_128;
	KCL::Image image_uncompressed_1920_background;
	KCL::Image image_uncompressed_1920_mask;
	KCL::Image image_cube;
		
	if (!image_2048.load((imageFolder + std::string("/fill_tex_2048.png")).c_str()))
	{
		INFO("Error in CompressedFillTest31::init. Can not load image: %s", image_2048.getName().c_str());
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}
	if (!image_1024.load((imageFolder + std::string("/fill_tex_1024.png")).c_str()))
	{
		INFO("Error in CompressedFillTest31::init. Can not load image: %s", image_1024.getName().c_str());
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}
	if (!image_128.load((imageFolder + std::string("/fill_tex_128.png")).c_str()))
	{
		INFO("Error in CompressedFillTest31::init. Can not load image: %s", image_128.getName().c_str());
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}
	if (!image_uncompressed_1920_background.load("fill2/fill_texture_bg.png"))
	{
		INFO("Error in CompressedFillTest31::init. Can not load image: %s", image_uncompressed_1920_background.getName().c_str());
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}
    image_uncompressed_1920_background.flipY();
	if (!image_uncompressed_1920_mask.load("fill2/fill_texture_alpha.png"))
	{
		INFO("Error in CompressedFillTest31::init. Can not load image: %s", image_uncompressed_1920_mask.getName().c_str());
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}
    image_uncompressed_1920_mask.flipY();
	if (!image_cube.loadCube((imageFolder + std::string("/envmap000")).c_str()))
	{
		INFO("Error in CompressedFillTest31::init. Can not load cube images: %s", image_cube.getName().c_str());
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}
	
	m_render_passes[0].textures[0] = CreateTexture(&image_1024);
	m_render_passes[0].textures[1] = CreateTexture(&image_128);
	m_render_passes[0].textures[2] = CreateTexture(&image_1024);
	m_render_passes[0].textures[3] = CreateTexture(&image_2048);	

	m_render_passes[1].textures[0] = CreateTexture(&image_128);
	m_render_passes[1].textures[1] = CreateTexture(&image_128);
	m_render_passes[1].textures[2] = CreateTexture(&image_1024);
	m_render_passes[1].textures[3] = CreateTexture(&image_2048);	

	m_render_passes[2].textures[0] = CreateTexture(&image_1024);
	m_render_passes[2].textures[1] = CreateTexture(&image_128);
	m_render_passes[2].textures[2] = CreateTexture(&image_1024);
	m_render_passes[2].textures[3] = CreateTexture(&image_2048);	
	
	m_render_passes[3].textures[0] = CreateTexture(&image_2048);
	m_render_passes[3].textures[1] = CreateTexture(&image_128);
	m_render_passes[3].textures[2] = CreateTexture(&image_1024);
	m_render_passes[3].textures[3] = CreateTexture(&image_2048);

    if (landscape)
    {
        m_render_passes[0].offsets = KCL::Vector4D(-0.5f, -0.5f, 0.0f, 0.0f);
        m_render_passes[1].offsets = KCL::Vector4D(0.5f, -0.5f, 0.5f, 0.0f);
        m_render_passes[2].offsets = KCL::Vector4D(-0.5f, 0.5f, 0.0f, 0.5f);
        m_render_passes[3].offsets = KCL::Vector4D(0.5f, 0.5f, 0.5f, 0.5f);
    }
    else
    {
        m_render_passes[0].offsets = KCL::Vector4D(0.5f, -0.5f, 0.0f, 0.0f);
        m_render_passes[1].offsets = KCL::Vector4D(0.5f, 0.5f, 0.5f, 0.0f);
        m_render_passes[2].offsets = KCL::Vector4D(-0.5f, -0.5f, 0.0f, 0.5f);
        m_render_passes[3].offsets = KCL::Vector4D(-0.5f, 0.5f, 0.5f, 0.5f);
    }
		
	// Create near sampler for "depth" and "G buffer" sampling
    MTLSamplerDescriptor *nearSamplerDesc = [[MTLSamplerDescriptor alloc] init];
    nearSamplerDesc.minFilter = MTLSamplerMinMagFilterNearest;
    nearSamplerDesc.magFilter = MTLSamplerMinMagFilterNearest;
    nearSamplerDesc.mipFilter = MTLSamplerMipFilterNotMipmapped;
    
    nearSamplerDesc.sAddressMode = MTLSamplerAddressModeRepeat;
    nearSamplerDesc.tAddressMode = MTLSamplerAddressModeRepeat;
    nearSamplerDesc.rAddressMode = MTLSamplerAddressModeClampToEdge;
    
    m_near_sampler = [m_device newSamplerStateWithDescriptor:nearSamplerDesc];
    

	// Create the landscape vertex buffer
    KCL::Vector4D screenBillboard[4];
    screenBillboard[0] = KCL::Vector4D(-0.5f, -0.5f, 0, 0);
    screenBillboard[1] = KCL::Vector4D(-0.5f,  0.5f, 0, 1);
    screenBillboard[2] = KCL::Vector4D(0.5f, -0.5f, 1, 0);
    screenBillboard[3] = KCL::Vector4D(0.5f,  0.5f, 1, 1);    
    
#if TARGET_OS_IPHONE
    m_vertex_buffer_landscape = [m_device newBufferWithBytes:screenBillboard[0].v length:sizeof(float) * 16 options:0];
#else
    m_vertex_buffer_landscape = [m_device newBufferWithBytes:screenBillboard[0].v length:sizeof(float) * 16 options:MTLStorageModeManaged];
#endif

    static const KCL::uint16 billboardIndices[] = { 0, 1, 2, 1, 2, 3 };
#if TARGET_OS_IPHONE
    m_index_buffer = [m_device newBufferWithBytes:billboardIndices length:sizeof(KCL::uint16) * 6 options:0];
#else
    m_index_buffer = [m_device newBufferWithBytes:billboardIndices length:sizeof(KCL::uint16) * 6 options:MTLStorageModeManaged];
#endif
        
	// Create the portrait vertex buffer
    screenBillboard[0] = KCL::Vector4D(0.5f, -0.5f, 0, 0); 
    screenBillboard[1] = KCL::Vector4D(-0.5f, -0.5f, 0, 1);
    screenBillboard[2] = KCL::Vector4D(0.5f, 0.5f, 1, 0);
    screenBillboard[3] = KCL::Vector4D(-0.5f,  0.5f, 1, 1);     

#if TARGET_OS_IPHONE
    m_vertex_buffer_portrait = [m_device newBufferWithBytes:screenBillboard[0].v length:sizeof(float) * 16 options:0];
#else
    m_vertex_buffer_portrait = [m_device newBufferWithBytes:screenBillboard[0].v length:sizeof(float) * 16 options:MTLStorageModeManaged];
#endif

    if (landscape)
    {
        m_vertex_buffer = m_vertex_buffer_landscape;
    }
    else
    {
        m_vertex_buffer = m_vertex_buffer_portrait;
    }    

	// Create the textures	
	MetalRender::Texture * fullhd_background_texture = CreateTexture(&image_uncompressed_1920_background);
	MetalRender::Texture * fullhd_mask_texture = CreateTexture(&image_uncompressed_1920_mask);
	CreateFullScreenTextures(fullhd_background_texture, fullhd_mask_texture);
	CreateDepthTexture();
	m_cube_texture = CreateTexture(&image_cube);	
	
	return KCL::KCL_TESTERROR_NOERROR;
}

MetalRender::Texture* CompressedFillTest2_Metal::CreateTexture(KCL::Image * image)
{
	MetalRender::Texture * texture = new MetalRender::Texture(image, false);
	texture->setMinFilter(KCL::TextureFilter_Linear);
	texture->setMagFilter(KCL::TextureFilter_Linear);
	texture->setMipFilter(KCL::TextureFilter_Linear);
	texture->setWrapS(KCL::TextureWrap_Repeat);
	texture->setWrapT(KCL::TextureWrap_Repeat);
	texture->commit();
	m_textures.push_back(texture);
	return texture;
}

void CompressedFillTest2_Metal::CreateFullScreenTextures(MetalRender::Texture* src_texture1, MetalRender::Texture* src_texture2)
{
@autoreleasepool
{
    id <MTLCommandBuffer> command_buffer = [m_command_queue commandBuffer];

    KCL::uint32 width = m_viewport_width;
    KCL::uint32 height = m_viewport_height;
    if (m_viewport_width < m_viewport_height)
    {
        width = m_viewport_height;
        height = m_viewport_width;
    }

    MTLTextureDescriptor *tex_desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                        width:width
                                                                                       height:height
                                                                                    mipmapped:false];
    tex_desc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
    tex_desc.storageMode = MTLStorageModePrivate;
    m_fullscreen_background = [m_device newTextureWithDescriptor:tex_desc];
    m_fullscreen_mask = [m_device newTextureWithDescriptor:tex_desc];

    MTLRenderPassDescriptor * render_pass_desc = [MTLRenderPassDescriptor renderPassDescriptor];
    render_pass_desc.colorAttachments[0].texture = m_fullscreen_background ;
    render_pass_desc.colorAttachments[0].loadAction = MTLLoadActionClear ;
    render_pass_desc.colorAttachments[0].clearColor = MTLClearColorMake(0.0f,0.0f,0.0f,1.0f);
    render_pass_desc.colorAttachments[0].storeAction = MTLStoreActionStore ;
    render_pass_desc.depthAttachment.texture = nil ;
    render_pass_desc.depthAttachment.loadAction = MTLLoadActionDontCare ;
    render_pass_desc.depthAttachment.storeAction = MTLStoreActionDontCare;
    
    id <MTLRenderCommandEncoder> fullscreen_bg_encoder = [command_buffer renderCommandEncoderWithDescriptor:render_pass_desc];

    MTLViewport viewport = { 0, 0, (double)width, (double)height, 0.0, 1.0};
    [fullscreen_bg_encoder setViewport:viewport];

    m_shader_resize->Set(fullscreen_bg_encoder);
    
    [fullscreen_bg_encoder setVertexBuffer:m_vertex_buffer_landscape offset:0 atIndex:0];
    src_texture1->Set(fullscreen_bg_encoder, 0);
    
    [fullscreen_bg_encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                      indexCount:6
                                       indexType:MTLIndexTypeUInt16
                                     indexBuffer:m_index_buffer
                               indexBufferOffset:0];
    
    [fullscreen_bg_encoder endEncoding];
    
    render_pass_desc.colorAttachments[0].texture = m_fullscreen_mask ;
    
    id <MTLRenderCommandEncoder> fullscreen_mask_encoder = [command_buffer renderCommandEncoderWithDescriptor:render_pass_desc];
    [fullscreen_mask_encoder setViewport:viewport];
    
    m_shader_resize->Set(fullscreen_mask_encoder);
    [fullscreen_mask_encoder setVertexBuffer:m_vertex_buffer_landscape offset:0 atIndex:0];
    src_texture2->Set(fullscreen_mask_encoder, 0);
	
    [fullscreen_mask_encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                        indexCount:6
                                         indexType:MTLIndexTypeUInt16
                                       indexBuffer:m_index_buffer
                                 indexBufferOffset:0];
    
    [fullscreen_mask_encoder endEncoding];
    [command_buffer commit];
}
}

void CompressedFillTest2_Metal::CreateDepthTexture()
{	
	const KCL::uint32 pixel_count = m_viewport_width * m_viewport_height;

	std::vector<float> depth_data;
	depth_data.resize(pixel_count);

	KCL::int32 seed = 42;
	for (KCL::uint32 i = 0; i < pixel_count; i++)
	{
		depth_data[i] = KCL::Math::randomf(&seed);
	}
    
    // Using 32 bit float depth instead of 24bit (24bit depth not supported on all Metal platform)
    MTLTextureDescriptor *depth_tex_desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                                                              width:m_viewport_width
                                                                                             height:m_viewport_height
                                                                                          mipmapped:false];
#if !TARGET_OS_IPHONE
    depth_tex_desc.storageMode = MTLStorageModePrivate;
#endif
    
    m_depth_texture = [m_device newTextureWithDescriptor:depth_tex_desc];
    
#if TARGET_OS_IPHONE
    [m_depth_texture replaceRegion:MTLRegionMake3D(0, 0, 0, m_viewport_width, m_viewport_height, 1)
                       mipmapLevel:0
                         withBytes:depth_data.data()
                       bytesPerRow:4*m_viewport_width];
#else
    // On OSX the depth texture must be private, thus we unable to upload the data directly
    @autoreleasepool {
        id <MTLBuffer> temp_buffer = [m_device newBufferWithBytes:depth_data.data() length:sizeof(float) * m_viewport_width*m_viewport_height options:MTLStorageModeManaged];
        
        id <MTLCommandBuffer> command_buffer = [m_command_queue commandBuffer];
        id <MTLBlitCommandEncoder> blit_encodeer = [command_buffer blitCommandEncoder];
        
        [blit_encodeer copyFromBuffer:temp_buffer
                         sourceOffset:0
                    sourceBytesPerRow:4*m_viewport_width
                  sourceBytesPerImage:4*m_viewport_width*m_viewport_height
                           sourceSize:MTLSizeMake(m_viewport_width, m_viewport_height, 1)
                            toTexture:m_depth_texture
                     destinationSlice:0
                     destinationLevel:0
                    destinationOrigin:MTLOriginMake(0, 0, 0)];
        
        [blit_encodeer endEncoding];
        [command_buffer commit];
        [command_buffer waitUntilCompleted];
    }
#endif
}

bool CompressedFillTest2_Metal::render()
{	
@autoreleasepool
{
    id <MTLCommandBuffer> command_buffer = [m_command_queue commandBuffer];
    
    id <MTLTexture> frameBufferTexture = dynamic_cast<GLB::FBOMetalBase*>(GLB::FBO::GetLastBind())->GetTexture() ;
    
    MTLRenderPassDescriptor * render_pass_desc = [MTLRenderPassDescriptor renderPassDescriptor];
    render_pass_desc.colorAttachments[0].texture = frameBufferTexture ;
    render_pass_desc.colorAttachments[0].loadAction = MTLLoadActionClear ;
    render_pass_desc.colorAttachments[0].clearColor = MTLClearColorMake(0.0f,0.0f,0.0f,1.0f);
    render_pass_desc.colorAttachments[0].storeAction = MTLStoreActionStore ;
    render_pass_desc.depthAttachment.texture = nil ;
    render_pass_desc.depthAttachment.loadAction = MTLLoadActionDontCare;
    render_pass_desc.depthAttachment.storeAction = MTLStoreActionDontCare;
    
    // Get a render encoder
    id <MTLRenderCommandEncoder> encoder = [command_buffer renderCommandEncoderWithDescriptor:render_pass_desc];

	const float exposure_time = 800.0f;	
	float rotation = 0.0f;
	
	KCL::uint32 pass_index = 0;
    
    m_shader->Set(encoder);
    
    [encoder setVertexBuffer:m_vertex_buffer offset:0 atIndex:0];
    
    VertexUniforms vu;
    vu.aspectRatio = m_aspectRatio;
	for (int i = 0; i < m_displayed_element_count; i++)
	{	
		rotation = (m_time + exposure_time * i / float(m_displayed_element_count)) * 0.0003f;
        [encoder setBlendColorRed:1.0f green:1.0f blue:1.0f alpha:1.0f / (1 + i)];

		for (KCL::uint32 j = 0; j < m_render_pass_count; j++)
		{						
			pass_index = j % m_render_pass_count;
            m_render_passes[pass_index].textures[0]->Set(encoder, 0);
            m_render_passes[pass_index].textures[1]->Set(encoder, 1);
            m_render_passes[pass_index].textures[2]->Set(encoder, 2);
            m_render_passes[pass_index].textures[3]->Set(encoder, 3);

            vu.rotation = rotation;
            vu.offsets = m_render_passes[pass_index].offsets;
            
            [encoder setVertexBytes:&vu length:sizeof(VertexUniforms) atIndex:1];
			
            [encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                indexCount:6
                                 indexType:MTLIndexTypeUInt16
                               indexBuffer:m_index_buffer
                         indexBufferOffset:0];
		}	
	}

    [encoder setFragmentTexture:m_fullscreen_background atIndex:0];
    [encoder setFragmentSamplerState:m_near_sampler atIndex:0];

    m_cube_texture->Set(encoder, 1);

    [encoder setFragmentTexture:m_depth_texture atIndex:2];
    [encoder setFragmentSamplerState:m_near_sampler atIndex:2];

    [encoder setFragmentTexture:m_fullscreen_mask atIndex:3];
    [encoder setFragmentSamplerState:m_near_sampler atIndex:3];

    m_shader_cube->Set(encoder);
    
	rotation = (m_time + exposure_time) * 0.0003f;
    vu.rotation = rotation;
    
    [encoder setVertexBytes:&vu length:sizeof(VertexUniforms) atIndex:1];
    
    [encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                        indexCount:6
                         indexType:MTLIndexTypeUInt16
                       indexBuffer:m_index_buffer
                 indexBufferOffset:0];

	const KCL::uint32 pass_textels = m_viewport_width * m_viewport_height * (4 + 4 * m_displayed_element_count);
	m_transferred_texels += pass_textels;

    
    [encoder endEncoding];
    [command_buffer commit];
    
    return true;
}
}


void CompressedFillTest2_Metal::FreeResources()
{
    // finish all GPU work before dealloc resources
    MetalRender::Finish();
    
    MetalRender::Pipeline::ClearCashes();
	m_shader = 0;
	m_shader_cube = 0;
	m_shader_resize = 0;

    releaseObj(m_vertex_buffer_landscape);
    releaseObj(m_vertex_buffer_portrait);
	releaseObj(m_index_buffer);

	releaseObj(m_fullscreen_background);
	releaseObj(m_fullscreen_mask);
	releaseObj(m_depth_texture);
	m_cube_texture = 0;

	releaseObj(m_near_sampler);

	for (KCL::uint32 i = 0; i < m_textures.size(); i++)
	{
		delete m_textures[i];
	}
	m_textures.clear();

	for (KCL::uint32 i = 0; i < m_render_pass_count; i++)
	{
		m_render_passes[i].Clear();
	}
}


GLB::TestBase *CreateCompressedFillTestMetal2(const GlobalTestEnvironment* const gte)
{
    return new CompressedFillTest2_Metal(gte);
}

