/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_compute_lightning.h"

#include <string>
#include <sstream>
#include <set>

#include "platform.h"
#include "kcl_io.h"
#include "render_statistics_defines.h"
#include "metal/mtl_pipeline_builder.h"

typedef float hfloat;
typedef KCL::Vector4D hfloat3;  // float3 padded to float4 in metal
typedef KCL::Vector4D hfloat4;
typedef KCL::Matrix4x4 hfloat4x4;

#include "lightning_pass1.h"
#include "lightning_pass2.h"

using namespace MetalRender ;

#if 0
void LOG_RUN()  { INFO("LIGHTNING_RUN" ) ; }
void LOG_DRAW() { INFO("LIGHTNING_DRAW") ; }
#else
#define LOG_RUN(...)  ;
#define LOG_DRAW(...) ;
#endif


MTLComputeLightning::MTLComputeLightning(id <MTLDevice> device) : m_data_prefix( "lightning_misc/")
{
	m_lightning_count = 0;

	m_render_buffer = 0;
	m_lightning_buffer = 0;	
	m_endpoint_buffer = 0;
		
	m_noise_texture = NULL;

	m_shader_pass1 = NULL;
	m_shader_pass2 = NULL;
	m_shader_render = NULL;
     
	m_lights_buffer = 0;
	
	m_ground_endoint_offset = 0;
	m_sky_endpoint_offset = 0;
    
    m_device = device ;

	status = KCL::KCL_TESTERROR_UNKNOWNERROR;
}


MTLComputeLightning::~MTLComputeLightning()
{
    releaseObj(m_render_buffer) ;
    releaseObj(m_lightning_buffer) ;
    releaseObj(m_endpoint_buffer) ;
    
	delete m_noise_texture;
}


void MTLComputeLightning::Init(id <MTLBuffer> lights_buffer, MetalRender::ShaderType main_shader_type)
{
	status = KCL::KCL_TESTERROR_NOERROR;

	m_lightning_count = 0;

	m_lights_buffer = lights_buffer;

#if 0
    if (m_lightning_buffer == nil)
    {
        INFO("WARNING! Debug code, remove from production code") ;
        
        size_t lights_buffer_size = 2*1024*1024 ;
        m_lights_buffer = [m_device newBufferWithLength:lights_buffer_size options:MTLResourceOptionCPUCacheModeDefault];
    }
#endif

	// Load the endpoints
	m_ground_endoint_offset = LoadEndpoints("lightning_points_poles.txt");	
	m_sky_endpoint_offset = LoadEndpoints("lightning_points_new.txt");
	LoadEndpoints("lightning_points_up.txt");
  
    
	struct DrawArraysIndirectCmd
	{
        KCL::uint32 count;
		KCL::uint32 instance_count;
		KCL::uint32 first;
		KCL::uint32 reserved;
	};

    size_t render_buffer_size = RENDER_BUFFER_SIZE * sizeof(KCL::Vector4D) + sizeof(DrawArraysIndirectCmd) ;
#if !TARGET_OS_EMBEDDED
    render_buffer_size += 60*sizeof(int); // pad the indirectdraw buffer on OSX
    m_render_buffer = [m_device newBufferWithLength:render_buffer_size
                                            options:MTLResourceStorageModeManaged];
#else
    m_render_buffer = [m_device newBufferWithLength:render_buffer_size
                                               options:MTLResourceOptionCPUCacheModeDefault];
#endif
    
    
	// Create and zero out the lightning headers
	float headers[LIGHTNING_COUNT + 64]; // SSBO's can be larger than 9 floats so just mmake it bigger
	memset(headers, 0, sizeof(headers));
    
    size_t lightning_buffer_size = sizeof(headers) + LIGHTNING_COUNT * LIGHTNING_BUFFER_SIZE * sizeof(KCL::Vector4D) ;
#if TARGET_OS_EMBEDDED
    m_lightning_buffer = [m_device newBufferWithLength:lightning_buffer_size
                                              options:MTLResourceOptionCPUCacheModeDefault];
#else
    m_lightning_buffer = [m_device newBufferWithLength:lightning_buffer_size
                                               options:MTLResourceStorageModeManaged];
#endif
    
    memcpy([m_lightning_buffer contents], (const void*)(headers), sizeof(headers));

#if !TARGET_OS_EMBEDDED
    [m_lightning_buffer didModifyRange:NSMakeRange(0, sizeof(headers))];
#endif
    
    
    size_t endpoint_buffer_size = m_endpoints.size() * sizeof(KCL::Vector4D);
#if TARGET_OS_EMBEDDED
    m_endpoint_buffer = [m_device newBufferWithLength:endpoint_buffer_size
                                              options:MTLResourceOptionCPUCacheModeDefault];
    memcpy([m_endpoint_buffer contents], (const void*)(&m_endpoints[0]), endpoint_buffer_size);
#else
    m_endpoint_buffer = [m_device newBufferWithBytes:(const void*)(&m_endpoints[0]) length:endpoint_buffer_size options:MTLResourceStorageModeManaged];
#endif
    
	
	// Load the shaders
	m_shader_pass1 = LoadShader("lightning_pass1.shader", true, main_shader_type);
    m_shader_pass1->IsThreadCountOk("lightning pass1", LP1_WORK_GROUP_SIZE);
	m_shader_pass2 = LoadShader("lightning_pass2.shader", true, main_shader_type);
    m_shader_pass2->IsThreadCountOk("lightning pass2", LP2_WORK_GROUP_SIZE);
	m_shader_render = LoadShader("lightning_render.shader", false, main_shader_type);
		
	// Load the noise texture
	MetalRender::TextureFactory f;
	KCL::Texture *noise_texture = f.CreateAndSetup(KCL::Texture_2D, std::string( m_data_prefix + "lightning_data2.png").c_str() , KCL::TC_Commit | KCL::TC_NearestFilter | KCL::TC_NoMipmap);
    m_noise_texture = dynamic_cast<MetalRender::Texture*>(noise_texture);
	if (!m_noise_texture)
	{
		INFO("ComputeLightning - ERROR: lightning_data.png not found!\n");
	}
    
    MTLDepthStencilDescriptor *lightningDepthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
    lightningDepthStateDesc.depthWriteEnabled = NO;
    lightningDepthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
    
    m_lightningDepthStencilState = [m_device newDepthStencilStateWithDescriptor:lightningDepthStateDesc];
}


KCL::uint32 MTLComputeLightning::LoadEndpoints(const char *filename)
{
	std::string fn = m_data_prefix + filename;

	KCL::AssetFile pos_endpoints_file(fn.c_str());	
	if(pos_endpoints_file.GetLastError())
	{
		INFO("ComputeLightning - ERROR: %s not found!\n", fn.c_str());
		return m_endpoints.size();
	}

	std::stringstream ss(pos_endpoints_file.GetBuffer());		
	while(true)
	{
		KCL::Vector4D pos;
		if (!(ss >> pos.x))
		{
			break;
		}
		ss >> pos.y;
		ss >> pos.z;

		// Filter out the lower points
		if (pos.y <= 0.5f)
		{
			continue;
		}

		m_endpoints.push_back(pos);
	}
	return m_endpoints.size();
}


MetalRender::Pipeline *MTLComputeLightning::LoadShader(const char *filename, bool force_highp, MetalRender::ShaderType shader_type)
{
	MTLPipeLineBuilder sb;
    KCL::KCL_Status error;

	sb.AddDefineInt("LIGHTNING_COUNT", LIGHTNING_COUNT);
	sb.AddDefineInt("BUFFER_SIZE", LIGHTNING_BUFFER_SIZE);
	sb.AddDefineInt("ENDPOINT_COUNT", m_endpoints.size());
	sb.AddDefineInt("GROUND_ENDPOINT_OFFSET", m_ground_endoint_offset);
	sb.AddDefineInt("SKY_ENDPOINT_OFFSET", m_sky_endpoint_offset);
	sb.AddDefineInt("MAX_LIGHTNING_BUFFER", (LIGHTNING_BUFFER_SIZE * LIGHTNING_COUNT));
	sb.AddDefineInt("MAX_RENDER_BUFFER", RENDER_BUFFER_SIZE);
    sb.SetType(shader_type) ;
    sb.HasDepth(true);
    sb.ForceHighp(force_highp) ;
			
	MetalRender::Pipeline *shader = sb.ShaderFile(filename).Build(error);
	if (error != KCL::KCL_TESTERROR_NOERROR)
	{
		status = error;
	}
	return shader;
}

 
void MTLComputeLightning::RunPass1(float animation_time, KCL::Actor *focus, id <MTLCommandBuffer> command_buffer, DynamicDataBuffer* ddb)
{
    LOG_RUN() ;
    
	if (!IsInited())
	{
		return;
	}

	// Update the lightning count
	if (animation_time < 2000)
		m_lightning_count = 1;

	else if (animation_time < 3000)
		m_lightning_count = 5;

	else if (animation_time < 9000)
		m_lightning_count = 7;
	else
		m_lightning_count = LIGHTNING_COUNT;

	m_lightning_count = m_lightning_count > LIGHTNING_COUNT ? LIGHTNING_COUNT : m_lightning_count;
	
	// Get the model matrix of the actor
	KCL::Matrix4x4 model_matrix = focus->m_root->m_world_pom;

	// Calculate the right vector of the actor
	KCL::Vector3D actor_pos(&model_matrix.v[12]);
	KCL::Vector4D actor_right_vector_h = model_matrix * KCL::Vector4D(0.0f, 0.0f, 42.0f, 1.0); // Z points to the right in the model coordinate system
	KCL::Vector3D actor_right_vector(actor_right_vector_h.x / actor_right_vector_h.w, actor_right_vector_h.y / actor_right_vector_h.w, actor_right_vector_h.z / actor_right_vector_h.w);
	actor_right_vector = actor_right_vector - actor_pos;

	// Get the bones of the actor
	m_bone_segments.clear();

	GetBonesForActor(focus->m_root);
    
    id <MTLComputeCommandEncoder> compute_encoder = [command_buffer computeCommandEncoder] ;
	
	// Dispatch the first pass. Calculate the lightnings as line segments
    m_shader_pass1->SetAsCompute(compute_encoder) ;
    
    
	// Set the uniforms
    lightning_pass1_uniforms lp1u ;
    lp1u.time = animation_time ;
    lp1u.actor_pos = KCL::Vector4D(actor_pos) ;
    lp1u.actor_right = KCL::Vector4D(actor_right_vector) ;
    lp1u.bone_segment_count = m_bone_segments.size() / 2 ;
    memcpy(lp1u.bone_segments, m_bone_segments[0].v, m_bone_segments.size()*sizeof(KCL::Vector4D)) ;
    
    size_t lp1u_offset = ddb->WriteDataAndGetOffset(nil, &lp1u, sizeof(lightning_pass1_uniforms)) ;
    [compute_encoder setBuffer:ddb->GetCurrentBuffer() offset:lp1u_offset atIndex:LP1_UNIFORMS_BFR_SLOT] ;
     
    
	// Bind the storage buffers
    [compute_encoder setBuffer:m_lights_buffer    offset:0 atIndex:LP1_LIGHTS_BFR_SLOT] ;
    [compute_encoder setBuffer:m_lightning_buffer offset:0 atIndex:LP1_LIGHTNING_BFR_SLOT] ;
    [compute_encoder setBuffer:m_endpoint_buffer  offset:0 atIndex:LP1_ENDPOINTS_BFR_SLOT] ;
    

	// Bind the noise texture
    [compute_encoder setTexture:m_noise_texture->GetTexture() atIndex:LP1_NOISE_MAP_TEX_SLOT] ;
    
    
    MTLSize threadsPerGroup = { LP1_WORK_GROUP_SIZE, 1, 1 };
    MTLSize numThreadgroups = { m_lightning_count, 1, 1};
    
    [compute_encoder dispatchThreadgroups:numThreadgroups
                    threadsPerThreadgroup:threadsPerGroup] ;
	
    [compute_encoder endEncoding] ;
}


void MTLComputeLightning::RunPass2(KCL::Camera2 *camera, id<MTLCommandBuffer> command_buffer, DynamicDataBuffer* ddb)
{
    if (!IsInited())
    {
        return;
    }
    
    KCL::Matrix4x4 mvp = camera->GetViewProjection();

    id <MTLComputeCommandEncoder> compute_encoder = [command_buffer computeCommandEncoder] ;
    
    // Dispatch the second pass. Create quads
    m_shader_pass2->SetAsCompute(compute_encoder) ;
    
    lightning_pass2_uniforms lp2u ;
    lp2u.current_lightning_count = m_lightning_count ;
    lp2u.mvp = mvp ;
    size_t lp2u_offset = ddb->WriteDataAndGetOffset(nil, &lp2u, sizeof(lightning_pass2_uniforms)) ;
    [compute_encoder setBuffer:ddb->GetCurrentBuffer() offset:lp2u_offset atIndex:LP2_UNIFORMS_BFR_SLOT] ;
    
    [compute_encoder setBuffer:m_lightning_buffer offset:0 atIndex:LP2_LIGHTNING_BFR_SLOT] ;
    [compute_encoder setBuffer:m_render_buffer    offset:0 atIndex:LP2_RENDER_BFR_SLOT] ;
    
    MTLSize threadsPerGroup = { LP2_WORK_GROUP_SIZE, 1, 1 };
    MTLSize numThreadgroups = { m_lightning_count, 1, 1};
    
    [compute_encoder dispatchThreadgroups:numThreadgroups
                    threadsPerThreadgroup:threadsPerGroup] ;
    
    [compute_encoder endEncoding] ;
}


void MTLComputeLightning::Draw(KCL::Camera2 *camera, id <MTLRenderCommandEncoder> render_encoder)
{
    LOG_DRAW() ;
    
	if (!IsInited())
	{
		return;
	}

    m_shader_render->Set(render_encoder) ;
	
    
#if !TARGET_OS_EMBEDDED
    [render_encoder setVertexBuffer:m_render_buffer offset:256 atIndex:0] ;
    
    [render_encoder setDepthStencilState:m_lightningDepthStencilState] ;
    
    [render_encoder drawPrimitives:MTLPrimitiveTypeTriangle indirectBuffer:m_render_buffer indirectBufferOffset:0];
#else
    [render_encoder setVertexBuffer:m_render_buffer offset:16 atIndex:0] ;
    [render_encoder setVertexBuffer:m_render_buffer offset:0  atIndex:1] ;

    [render_encoder setDepthStencilState:m_lightningDepthStencilState] ;

    [render_encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:(RENDER_BUFFER_SIZE/2)] ;
#endif
}


KCL::uint32 MTLComputeLightning::GetLightCount()
{
	// Every lightning creates two lights
	return m_lightning_count * 2;	
}


void MTLComputeLightning::GetBonesForActor(KCL::Node *node)
{
	if (node->m_name.find("jnt") != node->m_name.size() - 3)
	{
		// Bones with _jnt are not part of the robot
		return;
	}

	// Skip the fingers
	if (node->m_name.find("L_1") != std::string::npos)
	{
		return;
	}

	if (node->m_name.find("L_2") != std::string::npos)
	{
		return;
	}

	// Skip the toes and feet
	if (node->m_name.find("toe") != std::string::npos)
	{
		return;
	}

	if (node->m_name.find("foot") != std::string::npos)
	{
		return;
	}	

	if (node->m_name.find("leg") != std::string::npos)
	{
		return;
	}	

	if (node->m_name.find("hip") != std::string::npos)
	{
		return;
	}	

	
	if (node->m_parent) 
	{
		KCL::Vector4D pos1;
		KCL::Vector4D pos2;
		if (node->m_parent->m_name.find("chest") != std::string::npos)
		{
			pos2 = KCL::Vector4D(&node->m_parent->m_world_pom.v[12]);
			pos1 = KCL::Vector4D(&node->m_world_pom.v[12]);
		}
		else
		{
			pos1 = KCL::Vector4D(&node->m_parent->m_world_pom.v[12]);
			pos2 = KCL::Vector4D(&node->m_world_pom.v[12]);
		}

		pos1.w = 1.0f;
		pos2.w = 1.0f;
		
		// Put the gun to the list more time lower_arm_R
		if (node->m_name.find("gun") != std::string::npos)
		{				
			m_bone_segments.push_back(pos2);
			m_bone_segments.push_back(pos1);

			m_bone_segments.push_back(pos1);
			m_bone_segments.push_back(pos2);

			m_bone_segments.push_back(pos2);
			m_bone_segments.push_back(pos1);
		}

		m_bone_segments.push_back(pos1);
		m_bone_segments.push_back(pos2);	
	}

	for( KCL::uint32 i=0; i<node->m_children.size(); i++)
	{
		GetBonesForActor(node->m_children[i]);
	}
}


