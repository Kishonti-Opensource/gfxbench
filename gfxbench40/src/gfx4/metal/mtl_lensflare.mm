/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_lensflare.h"

#include <iostream>
#include "platform.h"
#include "kcl_io.h"
#include "fbo.h"
#include "fbo_metal.h"
#include "mtl_texture.h"
#include "metal/mtl_pipeline_builder.h"
#include "mtl_dynamic_data_buffer.h"
#include "ng/json.h"

using namespace KCL ;

#define OFFSET_BUFFER_BIND 1

namespace MetalRender
{

struct LensFlareConstants
{
	KCL::Matrix4x4 mvp;
	KCL::Vector2D sun_dir;
	float flare_size;
	float flare_distance;
};


Lensflare::Lensflare(id<MTLDevice> device)
	: m_device(device)
{
	m_compute_oc = NULL ;
	m_flares = NULL ;
	m_lens_dirt = NULL ;

	m_offset_count = 128 ;
	m_flare_count = 0 ;

    m_color_texture = nil;
    m_depth_texture = nil;
	
	m_portrait_mode = false;
}


Lensflare::~Lensflare()
{
	delete m_lens_dirt ;
	
	for ( std::map<std::string,Texture* >::iterator it = m_texture_pool.begin() ; it != m_texture_pool.end() ; it++)
	{
		delete it->second ;
	}

	delete[] m_flares ;
}


void Lensflare::Init(int width, int height, bool portrait_mode)
{
	m_viewport_width = width ;
	m_viewport_height = height ;
	m_portrait_mode = portrait_mode;

	m_offset_count = 128 ;


	//
	//	Load config file
	//
	MetalRender::TextureFactory texture_factory;

	std::string base_dir = "common/" ;
	bool has_error = false ;
	
	std::string lensflare_cfg_str ;

	if (KCL::AssetFile::Exists(base_dir+"lensflare.json"))
	{
		KCL::AssetFile lensflare_cfg_file(base_dir+"lensflare.json") ;
		
		lensflare_cfg_str = lensflare_cfg_file.GetBuffer() ;
	}
	else
	{
		has_error = true ;
		INFO("ERROR: lensflare config file not found") ;
	}


	ng::JsonValue lensflare_config ;
	if ( !has_error)
	{
		ng::Result res;
		
		lensflare_config.fromString(lensflare_cfg_str.c_str(),res) ;

		if (!res.ok())
		{
			has_error = true ;
			INFO("ERROR: unable to parse lensflare config file") ;
		}
	}


	if (!has_error) 
	{
		double distance_scale = lensflare_config["distance_scale"].number() ;
		double size_scale     = lensflare_config["size_scale"].number() ;

		ng::JsonValue flares_json = lensflare_config["flares"] ;
		m_flare_count = flares_json.size() ;
		m_flares = new Flare[m_flare_count] ;

		bool sundir_valid = lensflare_config["sun_dir_x"].isNumber() && lensflare_config["sun_dir_y"].isNumber() && lensflare_config["sun_dir_z"].isNumber() ;
		if ( sundir_valid )
		{
			m_light_dir.x = lensflare_config["sun_dir_x"].number() ;
			m_light_dir.y = lensflare_config["sun_dir_y"].number() ;
			m_light_dir.z = lensflare_config["sun_dir_z"].number() ;

			m_light_dir.normalize() ;
		}
		else
		{
			INFO("ERROR: sundir not set in lensflare config, lensflare disabled") ;
			has_error = true ;
		}

		for (int i = 0 ; i < m_flare_count ; i++)
		{
			m_flares[i].distance = distance_scale*flares_json[i]["distance"].number() ;
			m_flares[i].size = size_scale*flares_json[i]["size"].number() ;

			std::string texture_name = base_dir + flares_json[i]["texture"].string() ;

			if ( m_texture_pool.find(texture_name) == m_texture_pool.end() )
			{
				Texture* t = (Texture*)texture_factory.CreateAndSetup(KCL::Texture_2D, texture_name.c_str(), KCL::TC_Clamp | KCL::TC_NoMipmap);
				m_texture_pool[texture_name] = t ;
			}

			m_flares[i].texture = m_texture_pool[texture_name] ;
		}
	
		m_lens_dirt = (Texture*)texture_factory.CreateAndSetup(KCL::Texture_2D, (base_dir + "lensdirt.png").c_str(), KCL::TC_Clamp | KCL::TC_NoMipmap);
	}


	if (has_error)
	{
		m_flare_count = 0 ;
	}
	

	//
	//	occlusion shader build shader 
	//
	MTLPipeLineBuilder sb;

	KCL::KCL_Status result ;

	sb.ShaderFile("lensflare_occlusion.cshader") ;

	sb.AddDefine("COLOR_TEX_TYPE rgba16f") ;
	sb.AddDefine("COLOR_TEX_BIND 0") ;

	sb.AddDefineInt("WORK_GROUP_SIZE",m_offset_count) ;

	sb.AddDefineInt("VIEWPORT_WIDTH", width) ;
	sb.AddDefineInt("VIEWPORT_HEIGHT", height) ;
	sb.AddDefineInt("OFFSET_BUFFER_BIND", OFFSET_BUFFER_BIND) ;

	m_compute_oc = sb.Build(result) ;


	//
	//	render shader
	//
	MTLVertexDescriptor * vertex_desc = [[MTLVertexDescriptor alloc] init];

	vertex_desc.attributes[0].format = MTLVertexFormatFloat2;
	vertex_desc.attributes[0].bufferIndex = 0;
	vertex_desc.attributes[0].offset = 0;
	vertex_desc.attributes[1].format = MTLVertexFormatFloat2;
	vertex_desc.attributes[1].bufferIndex = 0;
	vertex_desc.attributes[1].offset = 2 * sizeof(float);

	vertex_desc.layouts[0].stride = sizeof(float) * 4;
	vertex_desc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

	sb.ShaderFile("lensflare.shader") ;

	if (m_portrait_mode)
	{
		sb.AddDefineInt("VIEWPORT_WIDTH", height);
		sb.AddDefineInt("VIEWPORT_HEIGHT", width);
	}
	else
	{
		sb.AddDefineInt("VIEWPORT_WIDTH", width);
		sb.AddDefineInt("VIEWPORT_HEIGHT", height);
	}
	sb.AddDefineInt("MAX_SAMPLE", m_offset_count) ;
	sb.SetVertexLayout(vertex_desc);
	sb.SetBlendType(MetalRender::Pipeline::BlendType::ONE_X_ONE);
	sb.SetTypeByPixelFormat(MTLPixelFormatBGRA8Unorm);


	m_lensflare_shader = sb.Build(result) ;
	releaseObj(vertex_desc);

	//
	//	VBO, VAO
	//
#define LENSFLARE_VBO_DATA_COUNT 16
	static const float v[LENSFLARE_VBO_DATA_COUNT] =
		{
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f, 1.0f,
            1.0f,  1.0f, 1.0f, 1.0f
		};


	m_vertex_data = [m_device newBufferWithBytes:v length:sizeof(v) options:STORAGE_MODE_MANAGED_OR_SHARED];

	//
	//	Generate offsets
	//
	int offset_seed = 54643  ;
	std::vector<KCL::Vector2D> offsets ;
	offsets.resize(m_offset_count) ;
	
	for (int i = 0 ; i < m_offset_count ; i++)
	{
		float t = 2.0 * Math::kPi * Math::randomf( &offset_seed ) ; 
		float u = Math::randomf( &offset_seed ) + Math::randomf( &offset_seed ) ;
		float r = ( u > 1.0 ) ? 2.0 - u : u ;

		// offsets for landscape depth
		offsets[i].x = r * cos(t) ;
		offsets[i].y = r * sin(t) * m_viewport_width / m_viewport_height ;

		// offsets for portrait depth
		// offsets[i].y = r * cos(t) ;
		// offsets[i].x = -r * sin(t) * m_viewport_height / m_viewport_width ;
	}

	m_offsets_buffer = [m_device newBufferWithBytes:&offsets[0]
											 length:m_offset_count*sizeof(KCL::Vector2D)
											options:STORAGE_MODE_MANAGED_OR_SHARED];
}


void Lensflare::Execute(id<MTLCommandBuffer> command_buffer, MetalRender::DynamicDataBuffer * ddb)
{	
	//
	//	calculate sun position
	//
	m_screenspace_sun_pos = m_camera->GetViewProjection()*KCL::Vector4D(m_light_dir,0.0f) ;
	float t_w = m_screenspace_sun_pos.w ;
	m_screenspace_sun_pos /= m_screenspace_sun_pos.w ;
	
	KCL::Vector4D normalized_screenspace_sun_pos = m_screenspace_sun_pos ;
	normalized_screenspace_sun_pos.x = (m_screenspace_sun_pos.x + 1.0) / 2.0 ;
	normalized_screenspace_sun_pos.y = (m_screenspace_sun_pos.y + 1.0) / 2.0 ;
	normalized_screenspace_sun_pos.w = t_w ;


	//
	//	clear atomic counter
	//
	KCL::uint32 zero = 0;

	m_atomic_counter = [m_device newBufferWithBytes:&zero length:sizeof(KCL::uint32) options:STORAGE_MODE_MANAGED_OR_SHARED];

	//
	//	Dispatch Compute
	//
	id<MTLComputeCommandEncoder> encoder = [command_buffer computeCommandEncoder];
	encoder.label = @"Compute Lens Flare";

	m_compute_oc->SetAsCompute(encoder);


	[encoder setTexture:m_depth_texture atIndex:0];
	[encoder setBuffer:m_offsets_buffer offset:0 atIndex:0];

	size_t sun_pos_offs = ddb->WriteDataAndGetOffset(nil, &normalized_screenspace_sun_pos.v, sizeof(KCL::Vector4D));
	[encoder setBuffer:ddb->GetCurrentBuffer() offset:sun_pos_offs atIndex:1];
	[encoder setBuffer:m_atomic_counter offset:0 atIndex:2];

	MTLSize threadsPerGroup = { m_offset_count, 1, 1 };
	MTLSize numThreadgroups = { 1, 1, 1 };

	[encoder dispatchThreadgroups:numThreadgroups threadsPerThreadgroup:threadsPerGroup];
	[encoder endEncoding];
}


void Lensflare::Render(id<MTLCommandBuffer> command_buffer, MetalRender::DynamicDataBuffer * ddb)
{
	if (m_flare_count == 0) return ;

	KCL::Matrix4x4 m;
	if(!m_portrait_mode)
	{
		float aspect_ratio = ((float)m_viewport_width) / m_viewport_height;
		KCL::Matrix4x4::Ortho( m, -aspect_ratio, aspect_ratio, -1, 1, -1, 1);
	}
	else
	{
		float aspect_ratio = ((float)m_viewport_width) / m_viewport_height;
		KCL::Matrix4x4::Ortho( m, -1, 1, -aspect_ratio, aspect_ratio, -1, 1);
	}

	MTLRenderPassDescriptor * desc = [[MTLRenderPassDescriptor alloc] init];

	desc.colorAttachments[0].texture = dynamic_cast<GLB::FBOMetalBase*>(GLB::FBO::GetLastBind())->GetTexture();
	desc.colorAttachments[0].loadAction = MTLLoadActionLoad;
	desc.colorAttachments[0].storeAction = MTLStoreActionStore;

	id <MTLRenderCommandEncoder> encoder = [command_buffer renderCommandEncoderWithDescriptor:desc];
	encoder.label = @"Render Lens Flares";
	releaseObj(desc);

	m_lensflare_shader->Set(encoder);

	[encoder setVertexBuffer:m_vertex_data offset:0 atIndex:0];
	[encoder setVertexBuffer:m_atomic_counter offset:0 atIndex:2];

	[encoder setFragmentBuffer:m_atomic_counter offset:0 atIndex:2];
	[encoder setFragmentTexture:m_lens_dirt->GetTexture() atIndex:1];

	LensFlareConstants lf_consts;

	lf_consts.mvp = m;
	lf_consts.sun_dir = KCL::Vector2D(m_screenspace_sun_pos.v);

	for (int i = 0 ; i < m_flare_count ; i++)
	{
		[encoder setFragmentTexture:m_flares[i].texture->GetTexture() atIndex:0];

		lf_consts.flare_distance = m_flares[i].distance;
		lf_consts.flare_size = m_flares[i].size;
		ddb->WriteAndSetData<true, true>(encoder, 1, &lf_consts, sizeof(LensFlareConstants));

		[encoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
	}

	[encoder endEncoding];
}

}
