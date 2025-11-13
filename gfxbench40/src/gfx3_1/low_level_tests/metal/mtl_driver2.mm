/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "kcl_io.h"
#include "platform.h"
#include "kcl_math3d.h"
#include "metal/fbo.h"
#include "metal/fbo_metal.h"
#include "test_base.h"
#include "kcl_base.h"

#include "graphics/metalgraphicscontext.h"
#include "metal/mtl_pipeline_builder.h"
#include "mtl_texture.h"


class RenderState;

struct VertexUniforms
{
    KCL::Vector2D u_position;
    KCL::Vector2D u_gridsize;
    KCL::Vector2D u_margin;
    KCL::Vector2D u_globmargin;
    float u_transform_matrix[4];
};


struct FragmentUniforms
{
    KCL::Vector3D u_color;
    
    float __pad;
};



//
//  PipeLine State
//
struct SHADER_STATE
{
    enum
    {
        INVALID = -1,
        MAIN_SHADER,
        GLOW_SHADER,
        COUNT
    };
};

struct BUFFER_STATE
{
    enum
    {
        INVALID = -1,
        TRI,
        SQR,
        DOT,
        COUNT
    };
};

struct BLEND_STATE
{
    enum
    {
        INVALID = -1,
        DISABLED,
        SRC_ALPHA_X_ONE_MINUS_SRC_ALPHA,
        CONSTANT_COLOR_X_ONE_MINUS_CONSTANT_COLOR,
        ONE_X_ZERO,
        CONSTANT_ALPHA_X_ONE,
        COUNT
    };
};



//
//  Depth State
//
struct DEPTH_STATE
{
    enum
    {
        INVALID = -1,
        DISABLED,
        NOT_EQUAL,
        LESS,
        LEQUAL,
        COUNT
    };
};



class DriverOverheadTest2_Metal : public GLB::TestBase
{
    friend class RenderState;
    
private:
    id <MTLBuffer> m_vbuf_sqr;
    id <MTLBuffer> m_ibuf_sqr;
    
    id <MTLBuffer> m_vbuf_tri;
    id <MTLBuffer> m_ibuf_tri;
    
    id <MTLBuffer> m_vbuf_dot;
    id <MTLBuffer> m_ibuf_dot;
    
    int m_score;
    float m_transformmat[4];
    GLB::FBO* m_glowfbo;
    
    MetalRender::Texture* m_normal_tex;
    MetalRender::Texture* m_burnt_tex;
    MetalRender::Texture* m_burnt2_tex;
    id <MTLSamplerState> m_sampler;
    
    void RenderChannels(int RowCount, int ColumnCount, float* channels, float backgroundIntensity);
    void RenderGlow(int RowCount, int ColumnCount, float* channels);
    
    id <MTLDevice> m_device;
    id <MTLCommandQueue> m_command_queue;
    id <MTLTexture> m_depth_texture;
    id <MTLRenderCommandEncoder> m_encoder;

    MetalRender::Pipeline* m_pipelines[SHADER_STATE::COUNT][BUFFER_STATE::COUNT][BLEND_STATE::COUNT][2];
    id <MTLDepthStencilState> m_DepthStates[DEPTH_STATE::COUNT] ;
    
    RenderState* m_state;
    bool m_glow_pass;
 
    VertexUniforms vu;
    FragmentUniforms fu;
    
    void DrawWithState(KCL::uint32 count);
    
public:
    DriverOverheadTest2_Metal(const GlobalTestEnvironment* const gte);
    ~DriverOverheadTest2_Metal();
    
protected:
    virtual const char* getUom() const { return "frames"; }
    virtual bool isWarmup() const { return false; }
    virtual KCL::uint32 indexCount() const { return 0; }
    virtual float getScore () const { return m_score; }
    
    virtual KCL::KCL_Status init ();
    virtual bool animate (const int time);
    virtual bool render ();
    
    virtual void FreeResources();
};



//
//  RenderState
//
class RenderState
{
public:
    RenderState(DriverOverheadTest2_Metal *d2metal);
    
    // pipeline state setters
    void BlendEnabled(bool v)
    {
        m_blend_enabled = v;
        PipeLineStateChanged();
    }
    
    void SetBlendMode(KCL::int32 v)
    {
        assert(v != BLEND_STATE::DISABLED);
        m_blend_mode = v;
        PipeLineStateChanged();
    }
    
    void SetBuffer(KCL::int32 v)
    {
        m_buffer_type = v;
        PipeLineStateChanged();
    }
    
    KCL::int32 GetBuffer()
    {
        return m_buffer_type;
    }
    
    void SetShader(KCL::int32 v)
    {
        m_shader_type = v;
        PipeLineStateChanged();
    }

    void BindPipeLineState()
    {
        if (m_d2metal->m_encoder == nil) return ;
        
        KCL::uint32 blend_mode = (m_blend_enabled)?m_blend_mode:BLEND_STATE::DISABLED;
        m_d2metal->m_pipelines[m_shader_type][m_buffer_type][blend_mode][!m_d2metal->m_glow_pass]->Set(m_d2metal->m_encoder);
    }
    
    
    // depth state setters
    void DepthTestEnabled(bool v)
    {
        m_depth_test_enabled = v;
        DepthStateChanged();
    }
    
    void DepthTestMode(KCL::int32 v)
    {
        assert(v != DEPTH_STATE::DISABLED);
        m_depth_test_mode = v;
        DepthStateChanged();
    }
    
    void BindDepthState()
    {
        if (m_d2metal->m_encoder == nil) return ;
        
        KCL::uint32 depth_test_mode = (m_depth_test_enabled)?m_depth_test_mode:DEPTH_STATE::DISABLED;
        [m_d2metal->m_encoder setDepthStencilState:m_d2metal->m_DepthStates[depth_test_mode]];
    }
    
    
    // scissor state
    void ScissorTestEnabled(bool v)
    {
        m_scissor_test_enabled = v ;
        ScissorStateChanged();
    }
    
    void Scissor(int x, int y, int width, int height)
    {
        m_scissor_rect.x = x;
        m_scissor_rect.y = y;
        m_scissor_rect.width  = width;
        m_scissor_rect.height = height;
        ScissorStateChanged();
    }
    
    void BindScissorState()
    {
        if (m_d2metal->m_encoder == nil) return ;
        
        if (m_scissor_test_enabled)
        {
            [m_d2metal->m_encoder setScissorRect:m_scissor_rect];
        }
        else
        {
            MTLScissorRect r = {0, 0, (NSUInteger)GLB::FBO::GetLastBind()->getWidth(), (NSUInteger)GLB::FBO::GetLastBind()->getHeight()} ;
            [m_d2metal->m_encoder setScissorRect:r];
        }
    }
    
private:
    
    void PipeLineStateChanged()
    {
        BindPipeLineState();
    }
    
    void DepthStateChanged()
    {
        BindDepthState();
    }
    
    void ScissorStateChanged()
    {
        BindScissorState();
    }
    
    // pipeline state
    KCL::int32 m_shader_type;
    bool       m_blend_enabled;
    KCL::int32 m_blend_mode;
    KCL::int32 m_buffer_type;
    
    // depth_state
    bool       m_depth_test_enabled;
    KCL::int32 m_depth_test_mode;
    
    // scissor_state
    bool m_scissor_test_enabled;
    MTLScissorRect m_scissor_rect;
    
    DriverOverheadTest2_Metal *m_d2metal;
};


RenderState::RenderState(DriverOverheadTest2_Metal *d2metal) :
// default pipeline state
m_shader_type(SHADER_STATE::INVALID),
m_blend_enabled(false),
m_blend_mode(BLEND_STATE::DISABLED),
m_buffer_type(BUFFER_STATE::INVALID),
// default depth state
m_depth_test_enabled(false),
m_depth_test_mode(DEPTH_STATE::LESS),
// scissor state
m_scissor_test_enabled(false),
m_d2metal(d2metal)
{
    m_scissor_rect.x = 0;
    m_scissor_rect.y = 0;
    m_scissor_rect.width  = d2metal->GetSetting().m_viewport_width;
    m_scissor_rect.height = d2metal->GetSetting().m_viewport_height;
}



DriverOverheadTest2_Metal::DriverOverheadTest2_Metal(const GlobalTestEnvironment* const gte) : TestBase(gte),
	m_ibuf_sqr(0),
	m_vbuf_sqr(0),
	m_ibuf_tri(0),
	m_vbuf_tri(0),
	m_ibuf_dot(0),
	m_vbuf_dot(0),
	m_glowfbo(0),
	m_sampler(0),
	m_normal_tex(0),
	m_burnt_tex(0),
	m_burnt2_tex(0),
    m_encoder(nil),
    m_state(nullptr),
    m_glow_pass(false)
{
    MetalGraphicsContext* context = (MetalGraphicsContext*)gte->GetGraphicsContext() ;
    
    m_device = context->getDevice();
    m_command_queue = context->getMainCommandQueue();
}


DriverOverheadTest2_Metal::~DriverOverheadTest2_Metal()
{
	FreeResources();
}


bool DriverOverheadTest2_Metal::animate(const int time)
{
	SetAnimationTime(time);

	int score = m_frames;
	if (score > m_score)
	{
		m_score = score;
	}

	return time < m_settings->m_play_time;
}


id <MTLBuffer> GenerateBuffer(id <MTLDevice> device, size_t datasize, void* data)
{
#if TARGET_OS_IPHONE
    id <MTLBuffer> buffer = [device newBufferWithBytes:data length:datasize options:MTLResourceOptionCPUCacheModeDefault];
#else
    id <MTLBuffer> buffer = [device newBufferWithBytes:data length:datasize options:MTLResourceStorageModeManaged];
#endif
    return buffer;
}


MetalRender::Texture* CreateTexture(KCL::uint8 red, KCL::uint8 green, KCL::uint8 blue)
{
	KCL::Image *img;
	img = new KCL::Image(2, 2, KCL::Image_RGBA8888);
	KCL::uint8* i = (KCL::uint8*)img->getData();
	for (KCL::uint8* i = (KCL::uint8*)img->getData(); i < (KCL::uint8*)img->getData() + img->getDataSize();)
	{
		*(i++) = red;
		*(i++) = green;
		*(i++) = blue;
		*(i++) = 255;
	}

    MetalRender::Texture *tex=new MetalRender::Texture(img,true);
	tex->setMagFilter(KCL::TextureFilter_Nearest);
	tex->setMinFilter(KCL::TextureFilter_Nearest);
	tex->setMipFilter(KCL::TextureFilter_NotApplicable);
	tex->commit();
	return tex;
}


KCL::KCL_Status DriverOverheadTest2_Metal::init()
{
    m_state = new RenderState(this);
    
    MTLSamplerDescriptor *samplerDesc = [[MTLSamplerDescriptor alloc] init];
    samplerDesc.minFilter = MTLSamplerMinMagFilterLinear;
    samplerDesc.magFilter = MTLSamplerMinMagFilterLinear;
    samplerDesc.mipFilter = MTLSamplerMipFilterNotMipmapped;
    
    samplerDesc.sAddressMode = MTLSamplerAddressModeClampToEdge;
    samplerDesc.tAddressMode = MTLSamplerAddressModeClampToEdge;
    samplerDesc.rAddressMode = MTLSamplerAddressModeClampToEdge;
    
    m_sampler = [m_device newSamplerStateWithDescriptor:samplerDesc];


    if (m_window_height <= m_window_width || GetScreenMode() != 0 ||  GetSetting().m_virtual_resolution)
	{
		m_transformmat[0] = 1; m_transformmat[1] = 0;
		m_transformmat[2] = 0; m_transformmat[3] = 1;
	}
	else {
		m_transformmat[0] = 0; m_transformmat[1] = -1;
		m_transformmat[2] = 1; m_transformmat[3] = 0;
	}

	m_score = 0;

    MTLVertexDescriptor* vertex_descriptors[BUFFER_STATE::COUNT];
    {
        NSUInteger sqr_stride = 5 * sizeof(float);
        MTLVertexDescriptor* sqr_vertexDesc = [[MTLVertexDescriptor alloc] init];
        
        sqr_vertexDesc.attributes[0].format = MTLVertexFormatFloat3;
        sqr_vertexDesc.attributes[0].bufferIndex = 0;
        sqr_vertexDesc.attributes[0].offset = 0;
        
        sqr_vertexDesc.layouts[0].stride = sqr_stride ;
        sqr_vertexDesc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
        
        vertex_descriptors[BUFFER_STATE::SQR] = sqr_vertexDesc;
    }
    
    {
        NSUInteger tri_stride = 4 * sizeof(float);
        MTLVertexDescriptor* tri_vertexDesc = [[MTLVertexDescriptor alloc] init];
        
        tri_vertexDesc.attributes[0].format = MTLVertexFormatFloat3;
        tri_vertexDesc.attributes[0].bufferIndex = 0;
        tri_vertexDesc.attributes[0].offset = 0;
        
        tri_vertexDesc.layouts[0].stride = tri_stride ;
        tri_vertexDesc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
        
        vertex_descriptors[BUFFER_STATE::TRI] = tri_vertexDesc;
    }
    
    {
        NSUInteger dot_stride = 3 * sizeof(float);
        MTLVertexDescriptor* dot_vertexDesc = [[MTLVertexDescriptor alloc] init];
        
        dot_vertexDesc.attributes[0].format = MTLVertexFormatFloat3;
        dot_vertexDesc.attributes[0].bufferIndex = 0;
        dot_vertexDesc.attributes[0].offset = 0;
        
        dot_vertexDesc.layouts[0].stride = dot_stride ;
        dot_vertexDesc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
        
        vertex_descriptors[BUFFER_STATE::DOT] = dot_vertexDesc;
    }

    MTLPipeLineBuilder::SetScene(KCL::SV_INVALID, nullptr);
	MTLPipeLineBuilder sb;
    sb.AddShaderDir("shaders_mtl/lowlevel2/");
	KCL::KCL_Status err;
    
    const char* shader_names[SHADER_STATE::COUNT] = { nullptr, nullptr };
    shader_names[SHADER_STATE::MAIN_SHADER] = "driver2_solid.shader";
    shader_names[SHADER_STATE::GLOW_SHADER] = "driver2_quad.shader";
    
    int blend_types[BLEND_STATE::COUNT];
    for (int i = 0; i < BLEND_STATE::COUNT; i++) { blend_types[i] = -1 ; }
    blend_types[BLEND_STATE::DISABLED] = MetalRender::Pipeline::BlendType::DISABLED ;
    blend_types[BLEND_STATE::SRC_ALPHA_X_ONE_MINUS_SRC_ALPHA] = MetalRender::Pipeline::BlendType::ALPHA_X_ONE_MINUS_SOURCE_ALPHA;
    blend_types[BLEND_STATE::CONSTANT_COLOR_X_ONE_MINUS_CONSTANT_COLOR] = MetalRender::Pipeline::BlendType::CONSTANT_COLOR_X_ONE_MINUS_CONSTANT_COLOR;
    blend_types[BLEND_STATE::ONE_X_ZERO] = MetalRender::Pipeline::BlendType::ONE_X_ZERO;
    blend_types[BLEND_STATE::CONSTANT_ALPHA_X_ONE] = MetalRender::Pipeline::CONSTANT_ALPHA_X_ONE;
    
    //
    //  Init pipeline states
    //
    bool force_highp = GetSetting().m_force_highp;
#if !TARGET_OS_IPHONE
    force_highp = true;
#endif
    for (int i = 0; i < SHADER_STATE::COUNT; i++)
    {
        for (int j = 0; j < BUFFER_STATE::COUNT; j++)
        {
            for (int k = 0; k < BLEND_STATE::COUNT; k++)
            {
                for (int l = 0; l < 2; l++)
                {
                    sb.ShaderFile(shader_names[i]);
                    sb.ForceHighp(force_highp);
                    sb.SetVertexLayout(vertex_descriptors[j]);
                    sb.SetTypeByPixelFormat(MTLPixelFormatBGRA8Unorm);
                    sb.SetBlendType(static_cast<MetalRender::Pipeline::BlendType>(blend_types[k]));
                    sb.HasDepth(l);
                    
                    m_pipelines[i][j][k][l] = sb.Build(err);
                }
            }
        }
    }
    
    //
    //  Init Depth States
    //
    MTLDepthStencilDescriptor *depthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
    depthStateDesc.depthWriteEnabled = false ;
    depthStateDesc.depthCompareFunction = MTLCompareFunctionAlways ;
    
    for (int i = 0; i < DEPTH_STATE::COUNT; i++)
    {
        switch (i) {
            case DEPTH_STATE::DISABLED:
                depthStateDesc.depthWriteEnabled = false ;
                depthStateDesc.depthCompareFunction = MTLCompareFunctionAlways ;
                break;
            case DEPTH_STATE::NOT_EQUAL:
                depthStateDesc.depthWriteEnabled = true ;
                depthStateDesc.depthCompareFunction = MTLCompareFunctionNotEqual ;
                break;
            case DEPTH_STATE::LESS:
                depthStateDesc.depthWriteEnabled = true ;
                depthStateDesc.depthCompareFunction = MTLCompareFunctionLess ;
                break;
            case DEPTH_STATE::LEQUAL:
                depthStateDesc.depthWriteEnabled = true ;
                depthStateDesc.depthCompareFunction = MTLCompareFunctionLessEqual ;
                break;
                
            default:
                assert(0);
                break;
        }
        
        m_DepthStates[i] = [m_device newDepthStencilStateWithDescriptor:depthStateDesc];
    }

	float vertices_sqr[] = {
		-1.0f, -1.0f, 0.5f, 0.0f, 0.0f,
		1.0f, -1.0f, 0.5f, 0.0f, 0.0f,
		-1.0f, 1.0f, 0.5f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.5f, 0.0f, 0.0f
	};
	KCL::uint16 indices_sqr[] = { 0, 1, 2, 3, 2, 1 };

	float vertices_tri[] = {
		-1.0f, -1.0f, 0.5f, 0.0f,
		1.0f, 0.0f, 0.5f, 0.0f,
		- 1.0f, 1.0f, 0.5f, 0.0f
	};
	KCL::uint16 indices_tri[] = { 0, 1, 2 };

	float vertices_dot[] = {
		0.0f, 0.0f, 0.5f,
		0.0f, 1.0f, 0.5f,
		0.75f, 0.75f, 0.5f,
		1.0f, 0.0f, 0.5f,
		0.75f, -0.75f, 0.5f,
		0.0f, -1.0f, 0.5f,
		-0.75f, -0.75f, 0.5f,
		-1.0f, 0.0f, 0.5f,
		-0.75f, 0.75f, 0.5f
	};
	KCL::uint16 indices_dot[] = { 0, 2, 1, 0, 3, 2, 0, 4, 3, 0, 5, 4, 0, 6, 5, 0, 7, 6, 0, 8, 7, 0, 1, 8 };

	if ( (m_vbuf_sqr = GenerateBuffer(m_device, sizeof(vertices_sqr), vertices_sqr)) == nil ) return KCL::KCL_TESTERROR_VBO_ERROR;
	if ( (m_ibuf_sqr = GenerateBuffer(m_device, sizeof(indices_sqr), indices_sqr)) == nil ) return KCL::KCL_TESTERROR_VBO_ERROR;
	if ( (m_vbuf_tri = GenerateBuffer(m_device, sizeof(vertices_tri), vertices_tri)) == nil ) return KCL::KCL_TESTERROR_VBO_ERROR;
	if ( (m_ibuf_tri = GenerateBuffer(m_device, sizeof(indices_tri), indices_tri)) == nil ) return KCL::KCL_TESTERROR_VBO_ERROR;
	if ( (m_vbuf_dot = GenerateBuffer(m_device, sizeof(vertices_dot), vertices_dot)) == nil ) return KCL::KCL_TESTERROR_VBO_ERROR;
	if ( (m_ibuf_dot = GenerateBuffer(m_device, sizeof(indices_dot), indices_dot)) == nil ) return KCL::KCL_TESTERROR_VBO_ERROR;

    
    m_glowfbo= GLB::FBO::CreateFBO(m_gte,64, 64, 0, GLB::RGB888_Linear, GLB::DEPTH_None, "");

	m_normal_tex = CreateTexture(127, 127, 127);
	m_burnt_tex = CreateTexture(215, 195, 127);
	m_burnt2_tex = CreateTexture(50, 25, 90);
    
    //
    //  Create depth texture
    //
    MTLTextureDescriptor *depth_tex_desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                                                              width:GetSetting().m_viewport_width
                                                                                             height:GetSetting().m_viewport_height
                                                                                          mipmapped:NO];
    depth_tex_desc.storageMode = MTLStorageModePrivate ;
    depth_tex_desc.usage = MTLTextureUsageRenderTarget ;
    
    m_depth_texture = [m_device newTextureWithDescriptor:depth_tex_desc];
    releaseObj(depth_tex_desc);

	return KCL::KCL_TESTERROR_NOERROR;
}

inline float frac(float val)
{
	return val - floor(val);
}


void DriverOverheadTest2_Metal::DrawWithState(KCL::uint32 count)
{
    id <MTLBuffer> idbuffer = nil;
    
    switch (m_state->GetBuffer()) {
        case BUFFER_STATE::SQR: idbuffer = m_ibuf_sqr; break;
        case BUFFER_STATE::TRI: idbuffer = m_ibuf_tri; break;
        case BUFFER_STATE::DOT: idbuffer = m_ibuf_dot; break;
        default: assert(0);
    }
    
    [m_encoder setVertexBytes:&vu length:sizeof(VertexUniforms) atIndex:1];
    [m_encoder setFragmentBytes:&fu length:sizeof(FragmentUniforms) atIndex:1];
    
    m_state->BindPipeLineState();
    m_state->BindDepthState();
    m_state->BindScissorState();
    
    [m_encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                        indexCount:count
                         indexType:MTLIndexTypeUInt16
                       indexBuffer:idbuffer
                 indexBufferOffset:0];
}


void DriverOverheadTest2_Metal::RenderChannels(int RowCount, int ColumnCount, float* channels, float backgroundIntensity)
{
	//glUniform2f(m_uniGridSize, 1, 1);
	//glUniform2f(m_uniMargin, 0, 0);
	//glUniform2f(m_uniGlobalMargin, 0, 0);
	//glUniform3f(m_uniColor, backgroundIntensity, backgroundIntensity, backgroundIntensity);
	//glUniform2f(m_uniPosition, 0, 0);
	//glUniformMatrix2fv(m_uniTransMat, 1, 0, m_transformmat);
    
    vu.u_gridsize   = KCL::Vector2D(1,1);
    vu.u_margin     = KCL::Vector2D(0,0);
    vu.u_globmargin = KCL::Vector2D(0,0);
    fu.u_color      = KCL::Vector3D(backgroundIntensity, backgroundIntensity, backgroundIntensity);
    vu.u_position   = KCL::Vector2D(0,0);
    memcpy(vu.u_transform_matrix, m_transformmat, 4*sizeof(float));
    
	//glDisable(GL_DEPTH_TEST);
    m_state->DepthTestEnabled(false);
    
	//glDisable(GL_BLEND);
    m_state->BlendEnabled(false);
    
	//m_normal_tex->bind(0);
    m_normal_tex->Set(m_encoder, 0);
    
	//glBindBuffer(GL_ARRAY_BUFFER, m_vbuf_sqr);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibuf_sqr);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    [m_encoder setVertexBuffer:m_vbuf_sqr offset:0 atIndex:0];
    m_state->SetBuffer(BUFFER_STATE::SQR);

    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    DrawWithState(6);
    
	for (int x = 0; x < ColumnCount; ++x)
	{
		for (int y = 0; y < RowCount; ++y)
		{
			int e = x + y*ColumnCount;
			if (channels[y] > x / (float)ColumnCount)
			{
				if (e % 153 == 0 || e % 255 == 95)
				{
					//m_burnt_tex->bind(0);
                    m_burnt_tex->Set(m_encoder, 0);
				}
				else if (e % 46 == 0 || e % 163 == 95)
				{
					//m_burnt2_tex->bind(0);
                    m_burnt2_tex->Set(m_encoder, 0);
				}
				else
				{
					//m_normal_tex->bind(0);
                    m_normal_tex->Set(m_encoder, 0);
				}

			}
			else
			{
				//m_normal_tex->bind(0);
                m_normal_tex->Set(m_encoder, 0);
			}
            
            //glUniform2f(m_uniGridSize, ColumnCount, RowCount + (RowCount - 1) / 2);
            //glUniform2f(m_uniMargin, 0.2f, 0.4f);
            //glUniform2f(m_uniGlobalMargin, 0.05f, 0.1f);
            
            vu.u_gridsize   = KCL::Vector2D(ColumnCount, RowCount + (RowCount - 1) / 2);
            vu.u_margin     = KCL::Vector2D(0.2f, 0.4f);
            vu.u_globmargin = KCL::Vector2D(0.05f, 0.1f);
            
			if (x < ColumnCount*0.9)
			{
				if (channels[y] > x / (float)ColumnCount)
				{
					//glUniform3f(m_uniColor, 0.2f, 0.8f, 0.2f);
                    fu.u_color = KCL::Vector3D(0.2f, 0.8f, 0.2f);
				}
				else {
					//glUniform3f(m_uniColor, 0.1f, 0.2f, 0.1f);
                    fu.u_color = KCL::Vector3D(0.1f, 0.2f, 0.1f);
				}
			}
			else {
				if (channels[y] > x / (float)ColumnCount)
				{
					//glUniform3f(m_uniColor, 0.8f, 0.2f, 0.2f);
                    fu.u_color = KCL::Vector3D(0.8f, 0.2f, 0.2f);
				}
				else {
					//glUniform3f(m_uniColor, 0.2f, 0.1f, 0.1f);
                    fu.u_color = KCL::Vector3D(0.2f, 0.1f, 0.1f);
				}
			}
			//glUniform2f(m_uniPosition, x, y + y / 2);
            vu.u_position = KCL::Vector2D(x, y + y / 2);

			if (y%16<8)
			{
				//glEnable(GL_DEPTH_TEST);
                m_state->DepthTestEnabled(true);
				//glDepthFunc(GL_NOTEQUAL);
                m_state->DepthTestMode(DEPTH_STATE::NOT_EQUAL);
				//glEnable(GL_BLEND);
                m_state->BlendEnabled(true);
				//glDisable(GL_SCISSOR_TEST);
                m_state->ScissorTestEnabled(false);
				//glScissor(0, 0, 0, 0);
                m_state->Scissor(0, 0, 1, 1);
				//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                m_state->SetBlendMode(BLEND_STATE::SRC_ALPHA_X_ONE_MINUS_SRC_ALPHA);
				//glBindBuffer(GL_ARRAY_BUFFER, m_vbuf_sqr);
                [m_encoder setVertexBuffer:m_vbuf_sqr offset:0 atIndex:0];
				//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibuf_sqr);
				//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
                m_state->SetBuffer(BUFFER_STATE::SQR);
				//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
                DrawWithState(6);
			}
			else if (y % 16<12) {
				//glEnable(GL_DEPTH_TEST);
                m_state->DepthTestEnabled(false);
				//glDepthFunc(GL_LESS);
                m_state->DepthTestMode(DEPTH_STATE::LESS);
                //glEnable(GL_BLEND);
                m_state->BlendEnabled(true);
				//glEnable(GL_SCISSOR_TEST);
                m_state->ScissorTestEnabled(true);
                //glScissor(0, 0, GetSetting().m_viewport_width, GetSetting().m_viewport_height);
                m_state->Scissor(0, 0, GetSetting().m_viewport_width, GetSetting().m_viewport_height);
				//glBlendColor(1.0f, 1.0f, 1.0f, 1.0f);
                [m_encoder setBlendColorRed:1.0f green:1.0f blue:1.0f alpha:1.0f];
				//glBlendFunc(GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR);
                m_state->SetBlendMode(BLEND_STATE::CONSTANT_COLOR_X_ONE_MINUS_CONSTANT_COLOR);
                //glBindBuffer(GL_ARRAY_BUFFER, m_vbuf_tri);
                [m_encoder setVertexBuffer:m_vbuf_tri offset:0 atIndex:0];
				//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibuf_tri);
				//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
                m_state->SetBuffer(BUFFER_STATE::TRI);
				//glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);
                DrawWithState(3);
			}
			else {
                //glEnable(GL_DEPTH_TEST);
                m_state->DepthTestEnabled(true);
				//glDepthFunc(GL_LEQUAL);
                m_state->DepthTestMode(DEPTH_STATE::LEQUAL);
				//glEnable(GL_BLEND);
                m_state->BlendEnabled(true);
				//glBlendFunc(GL_ONE,GL_ZERO);
                m_state->SetBlendMode(BLEND_STATE::ONE_X_ZERO);
				//glDisable(GL_SCISSOR_TEST);
                m_state->ScissorTestEnabled(false);
				//glScissor(0, 0, 0, 0);
                m_state->Scissor(0, 0, 1, 1);
				//glUniform2f(m_uniMargin, 0.2f, 0.7f);
                vu.u_margin = KCL::Vector2D(0.2f,0.7f);
				//glBindBuffer(GL_ARRAY_BUFFER, m_vbuf_dot);
                [m_encoder setVertexBuffer:m_vbuf_dot offset:0 atIndex:0];
				//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibuf_dot);
				//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
                m_state->SetBuffer(BUFFER_STATE::DOT);
				//glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_SHORT, 0);
                DrawWithState(24);
			}
		}
	}
    
	//glDisable(GL_DEPTH_TEST);
	//glDisable(GL_BLEND);
	//glDisable(GL_SCISSOR_TEST);
    
    m_state->DepthTestEnabled(false);
    m_state->BlendEnabled(false);
    m_state->ScissorTestEnabled(false);
}

void DriverOverheadTest2_Metal::RenderGlow(int RowCount, int ColumnCount, float* channels)
{
	//glUniformMatrix2fv(m_uniTransMat, 1, 0, m_transformmat);
    memcpy(&vu.u_transform_matrix,m_transformmat,4*sizeof(float));
	//m_normal_tex->bind(0);
    m_normal_tex->Set(m_encoder, 0);
	for (int x = 0; x < ColumnCount; ++x)
	{
		for (int y = 0; y < RowCount; ++y)
		{
			if (channels[y] < x / (float)ColumnCount) continue;
			
            //glUniform2f(m_uniGridSize, ColumnCount, RowCount + (RowCount - 1) / 2);
			//glUniform2f(m_uniMargin, 0.0f, 0.0f);
			//glUniform2f(m_uniGlobalMargin, 0.05f, 0.1f);
            vu.u_gridsize = KCL::Vector2D(ColumnCount, RowCount + (RowCount - 1) / 2);
            vu.u_margin = KCL::Vector2D(0.0f, 0.0f);
            vu.u_globmargin = KCL::Vector2D(0.05f, 0.1f);

			if (x < ColumnCount*0.9)
			{
				//glUniform3f(m_uniColor, 0.2f, 0.8f, 0.2f);
                fu.u_color = KCL::Vector3D(0.2f, 0.8f, 0.2f);
			}
			else {
				//glUniform3f(m_uniColor, 0.8f, 0.2f, 0.2f);
                fu.u_color = KCL::Vector3D(0.8f, 0.2f, 0.2f);
			}
			//glUniform2f(m_uniPosition, x, y + y / 2);
            vu.u_position = KCL::Vector2D(x, y + y / 2);

			//glBindBuffer(GL_ARRAY_BUFFER, m_vbuf_sqr);
            [m_encoder setVertexBuffer:m_vbuf_sqr offset:0 atIndex:0];
			//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibuf_sqr);
			//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);
            m_state->SetBuffer(BUFFER_STATE::SQR);
			//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
            DrawWithState(6);
		}
	}
	//glBindTexture(GL_TEXTURE_2D, 0);
    [m_encoder setVertexTexture:nil atIndex:0];
    [m_encoder setFragmentTexture:nil atIndex:0];
}

bool DriverOverheadTest2_Metal::render()
{
@autoreleasepool
{
    id <MTLCommandBuffer> command_buffer = [m_command_queue commandBuffer];
	m_encoder = nil;
    
	const int ColumnCount = 200;
	const int RowCount = 32;

	float time = GetAnimationTime() / 1000.0f;

	float beat = fabs(0.5 - (time * 2 - floor(time * 2)));

	float channels[RowCount];

	channels[0] = channels[31] = (0.95 - frac(time * 2)*0.1 - frac(time * 4)*0.1 )*0.9;
	channels[1] = channels[30] = 0.95 - frac(time * 2)*0.1 - frac(time * 4)*0.1;

	channels[2] = channels[29] = 0.86 + frac(time * 46)*0.1;
	channels[3] = channels[28] = 0.85 + frac(time * 50)*0.1;

	channels[4] = channels[27] = 0.9 - frac(time*3)*0.2;
	channels[5] = channels[26] = 0.9 - frac(time * 4)*0.2;

	channels[6] = channels[25] = 0.9 + 0.05*sin(time*6) - frac(time * 20)*0.1;
	channels[7] = channels[24] = 0.9 + 0.05*sin(time*6-1) - frac(time * 21)*0.1;

	channels[8] = channels[23] = 0.5 - frac(time * 4)*0.1;
	channels[9] = channels[22] = 0.5 - frac(time * 4-0.5)*0.1;

	channels[10] = channels[21] = 0.85 - frac(time * 6-0.4)*0.1;
	channels[11] = channels[20] = 0.95 - frac(time * 6)*0.1;

	channels[12] = channels[19] = 0.55 - frac(time * 7-0.4)*0.1;
	channels[13] = channels[18] = 0.65 - frac(time * 7)*0.1;

	channels[14] = channels[17] = 0.25 - frac(time * 9)*0.1;
	channels[15] = channels[16] = 0.15 - frac(time * 9-0.4)*0.1;

	//GLB::OpenGLStateManager::DisableAllCapabilites();
	//GLB::OpenGLStateManager::DisableAllVertexAttribs();
    m_state->BlendEnabled(false);
    m_state->DepthTestEnabled(false);
    
    //glUseProgram(m_shader);
    m_state->SetShader(SHADER_STATE::MAIN_SHADER);

	// rendering glow to fbo
	GLB::FBO::bind(m_glowfbo);
    m_glow_pass = true;
    //glClearColor(0, 0, 0, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    id <MTLTexture> glowTexture = dynamic_cast<GLB::FBOMetalBase*>(GLB::FBO::GetLastBind())->GetTexture() ;
    MTLRenderPassDescriptor * glow_render_pass_desc = [MTLRenderPassDescriptor renderPassDescriptor];
    glow_render_pass_desc.colorAttachments[0].texture = glowTexture ;
    glow_render_pass_desc.colorAttachments[0].loadAction = MTLLoadActionClear ;
    glow_render_pass_desc.colorAttachments[0].clearColor = MTLClearColorMake(0.0f,0.0f,0.0f,1.0f);
    glow_render_pass_desc.colorAttachments[0].storeAction = MTLStoreActionStore ;
    glow_render_pass_desc.depthAttachment.texture = nil ;
    glow_render_pass_desc.depthAttachment.loadAction = MTLLoadActionClear ;
    glow_render_pass_desc.depthAttachment.clearDepth = 1.0f ;
    glow_render_pass_desc.depthAttachment.storeAction = MTLStoreActionDontCare;
    
    // Get a render encoder
    m_encoder = [command_buffer renderCommandEncoderWithDescriptor:glow_render_pass_desc];

    //glViewport(0, 0, m_glowfbo->getWidth(), m_glowfbo->getHeight());
    MTLViewport glow_viewport = { 0, 0, (double)m_glowfbo->getWidth(), (double)m_glowfbo->getHeight(), 0.0, 1.0};
    [m_encoder setViewport:glow_viewport];

	RenderGlow(RowCount, ColumnCount, channels);
    
    [m_encoder endEncoding];
    
	// rendering main
    GLB::FBO::bind(0);
    m_glow_pass = false;
    
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    id <MTLTexture> frameBufferTexture = dynamic_cast<GLB::FBOMetalBase*>(GLB::FBO::GetLastBind())->GetTexture() ;
    MTLRenderPassDescriptor * main_render_pass_desc = [MTLRenderPassDescriptor renderPassDescriptor];
    main_render_pass_desc.colorAttachments[0].texture = frameBufferTexture ;
    main_render_pass_desc.colorAttachments[0].loadAction = MTLLoadActionClear ;
    main_render_pass_desc.colorAttachments[0].clearColor = MTLClearColorMake(0.0f,0.0f,0.0f,1.0f);
    main_render_pass_desc.colorAttachments[0].storeAction = MTLStoreActionStore ;
    main_render_pass_desc.depthAttachment.texture = m_depth_texture ;
    main_render_pass_desc.depthAttachment.loadAction = MTLLoadActionClear ;
    main_render_pass_desc.depthAttachment.clearDepth = 1.0f ;
    main_render_pass_desc.depthAttachment.storeAction = MTLStoreActionDontCare;
    
    // Get a render encoder
    m_encoder = [command_buffer renderCommandEncoderWithDescriptor:main_render_pass_desc];
    
    //glViewport(0, 0, GetSetting().m_viewport_width, GetSetting().m_viewport_height);
    MTLViewport viewport = { 0, 0, (double)GetSetting().m_viewport_width, (double)GetSetting().m_viewport_height, 0.0, 1.0};
    [m_encoder setViewport:viewport];
    
	RenderChannels(RowCount, ColumnCount, channels, beat / 5);

	// add glow
	//glEnable(GL_BLEND);
    m_state->BlendEnabled(true);
	//glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE);
    m_state->SetBlendMode(BLEND_STATE::CONSTANT_ALPHA_X_ONE);
	//glBlendColor(0.0f, 0.0f, 0.0f, 0.5f);
    [m_encoder setBlendColorRed:0.0f green:0.0f blue:0.0f alpha:0.5f];
	//glUseProgram(m_renderglowshader);
    m_state->SetShader(SHADER_STATE::GLOW_SHADER);
    //glBindBuffer(GL_ARRAY_BUFFER, m_vbuf_sqr);
    [m_encoder setVertexBuffer:m_vbuf_sqr offset:0 atIndex:0];
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibuf_sqr);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    m_state->SetBuffer(BUFFER_STATE::SQR);
	//GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_glowfbo->getTextureName());
    [m_encoder setFragmentTexture:glowTexture atIndex:0];
	//glBindSampler(0, m_sampler);
    [m_encoder setFragmentSamplerState:m_sampler atIndex:0];
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    DrawWithState(6);

	//glDisable(GL_BLEND);
    m_state->BlendEnabled(false);
	//glBindTexture(GL_TEXTURE_2D, 0);
    [m_encoder setFragmentTexture:nil atIndex:0];
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glDisableVertexAttribArray(0);
    [m_encoder setVertexBuffer:nil offset:0 atIndex:0];
    
    [m_encoder endEncoding];
    [command_buffer commit];

	return true;
}
}


void DriverOverheadTest2_Metal::FreeResources()
{
    // finish all GPU work before dealloc resources
    MetalRender::Finish();
    
    releaseObj(m_depth_texture);
    releaseObj(m_command_queue);
    releaseObj(m_device);
    
    delete m_state;
    
    MetalRender::Pipeline::ClearCashes();
    
    for (int i = 0; i < DEPTH_STATE::COUNT; i++)
    {
        releaseObj(m_DepthStates[i]);
    }

    releaseObj(m_vbuf_sqr);
    releaseObj(m_ibuf_sqr);
    releaseObj(m_vbuf_tri);
    releaseObj(m_ibuf_tri);
    releaseObj(m_vbuf_dot);
    releaseObj(m_ibuf_dot);
    
	delete m_glowfbo;
	m_glowfbo = 0;
	delete m_normal_tex;
	delete m_burnt_tex;
	delete m_burnt2_tex;

    releaseObj(m_sampler);
}


GLB::TestBase *CreateDriverOverhead2TestMetal(const GlobalTestEnvironment* const gte)
{
    return new DriverOverheadTest2_Metal(gte);
}

