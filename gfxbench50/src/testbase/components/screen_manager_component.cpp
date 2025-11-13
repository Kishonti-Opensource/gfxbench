/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "screen_manager_component.h"

#include "test_base_gfx.h"
#include "ngl.h"
#include "ksl_compiler.h"
#include "gfxb_barrier.h"
#include <kcl_os.h>

#include <assert.h>

#define MULTI_LINE_STRING(a) #a

using namespace GLB;

const char* ScreenManagerComponent::NAME = "ScreenManagerComponent";

enum ScreenManagerUniforms
{
	UNIFORM_TEX,
	UNIFORM_MAX
};

enum SceenManagerShaderCode
{
	SHADER_CODE_NORMAL,
	SHADER_CODE_FLIPPED,
	SHADER_CODE_ROTATED,
	SHADER_CODE_FLIPPED_ROTATED,
};


//
//	ScreenManager LoadShader
//


static void OffscreenManagerLoadShader(NGL_job_descriptor &jd, KCL::uint32 pass, KCL::uint32 shader_code, NGL_shader_source_descriptor ssd[NGL_NUM_SHADER_TYPES], std::vector<NGL_shader_uniform> &application_uniforms)
{
	{
		application_uniforms.resize(UNIFORM_MAX);
		application_uniforms[UNIFORM_TEX].m_name = "tex";
	}

	NGL_api api = nglGetApi();

	ssd[NGL_VERTEX_SHADER].m_source_data = "";

	switch (shader_code) {
		case SHADER_CODE_NORMAL: ssd[NGL_VERTEX_SHADER].m_source_data += "#define SHADER_CODE_NORMAL\n"; break;
		case SHADER_CODE_FLIPPED: ssd[NGL_VERTEX_SHADER].m_source_data += "#define SHADER_CODE_FLIPPED\n"; break;
		case SHADER_CODE_ROTATED: ssd[NGL_VERTEX_SHADER].m_source_data += "#define SHADER_CODE_ROTATED\n"; break;
		case SHADER_CODE_FLIPPED_ROTATED: ssd[NGL_VERTEX_SHADER].m_source_data += "#define SHADER_CODE_FLIPPED_ROTATED\n"; break;

		default:
			break;
	}

	ssd[NGL_VERTEX_SHADER].m_source_data +=
	"in float2 in_position;\n"
	"in float2 in_texcoord;\n"

	"out float2 v_texcoord;\n"

	"void main()\n"
	"{\n"

        "v_texcoord = in_texcoord;\n"
		"gl_Position = float4(in_position, 0.0, 1.0);\n"

	"#if defined SHADER_CODE_NORMAL\n"

		// do nothing

	"#elif defined SHADER_CODE_ROTATED\n"

		"v_texcoord = float2(v_texcoord.y,1.0-v_texcoord.x);\n"

    "#elif defined SHADER_CODE_FLIPPED\n"

        "v_texcoord.y = 1.0-v_texcoord.y;\n"

	"#elif defined SHADER_CODE_FLIPPED_ROTATED\n"

		"v_texcoord = v_texcoord.yx;\n"

	"#else\n"
	"#error\n"
	"#endif\n"

	"}\n";


	ssd[NGL_FRAGMENT_SHADER].m_source_data = MULTI_LINE_STRING(
		out float4 res{ color(0) };

	in float2 v_texcoord;

	uniform sampler2D<float> tex;

	void main()
	{
		res = texture(tex, v_texcoord);
	}
	);

	std::vector<KSLCompiler*> stage_compilers;
	stage_compilers.resize(NGL_NUM_SHADER_TYPES, NULL);

	for (uint32_t shader_type = 0; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
	{
		NGL_shader_source_descriptor &s = ssd[shader_type];

		if (s.m_source_data == "") continue;

		stage_compilers[shader_type] = new KSLCompiler();
		KSLCompiler &c = *stage_compilers[shader_type];

		c.SetSource(s.m_source_data);
		c.SetTargetApi(api);
		c.SetShaderType(NGL_shader_type(shader_type));
		c.SetShaderUniformInfo(application_uniforms);

		c.Analyze();

		if (c.HasErrors())
		{
			printf("Compile error:\n%s\n", c.GetLog().c_str());
			assert(0);
		}
	}

	KSLCompiler::ProcessStageInfo(stage_compilers);

	for (uint32_t shader_type = 0; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
	{
		if (stage_compilers[shader_type] == NULL) continue;

		NGL_shader_source_descriptor &s = ssd[shader_type];
		KSLCompiler &c = *stage_compilers[shader_type];

		c.Generate();

		s.m_source_data = "";
		switch (api)
		{
		case NGL_OPENGL_ES:
			s.m_source_data = "#version 300 es\n";
			break;
		case NGL_OPENGL:
			s.m_source_data = "#version 400\n";
			break;
		case NGL_VULKAN:
			s.m_source_data = "#version 430\n";
			break;
		case NGL_METAL_IOS:
		case NGL_METAL_MACOS:
		{
			switch (shader_type)
			{
			case NGL_VERTEX_SHADER:
			{
				c.CreateReflection(s);
				s.m_entry_point = "vertex_main";
				break;
			}
			case NGL_FRAGMENT_SHADER: s.m_entry_point = "fragment_main"; break;
			case NGL_COMPUTE_SHADER:  s.m_entry_point = "compute_main";  break;
			default:				  assert(0);						   break;
			}
		}
		break;
		case NGL_DIRECT3D_11:
		case NGL_DIRECT3D_12:
		{
			switch (shader_type)
			{
			case NGL_VERTEX_SHADER:
				s.m_entry_point = "vertex_main";
				s.m_version = "vs_5_0";
				break;
			case NGL_FRAGMENT_SHADER:
				s.m_entry_point = "fragment_main";
				s.m_version = "ps_5_0";
				break;
			case NGL_COMPUTE_SHADER:
				s.m_entry_point = "compute_main";
				s.m_version = "cs_5_0";
				break;
			default:
				assert(0);
				break;
			}
		}
		break;
		default:
			printf("Unhandled render api\n");
			assert(0);
			break;
		}
		s.m_source_data += c.GetResult();

		delete stage_compilers[shader_type];
	}
}


//
//	OnScreenManagerComponent
//


class OnScreenManagerComponent : public ScreenManagerComponent
{
public:
	OnScreenManagerComponent(TestBaseGFX* test, KCL::uint32 native_width, KCL::uint32 native_height)
		: ScreenManagerComponent(test, native_width, native_height)
	{
		m_backbuffers.push_back(0);
	}

	virtual KCL::uint32 GetActiveBackbufferId() const override { return 0; }
	virtual void GetTestSize(KCL::uint32 &width, KCL::uint32 &height) const override
	{
		width = m_native_width;
		height = m_native_height;
	}
	virtual void FinishTest() override { m_swapbuffer_needed = false; }
	virtual void FinishRender() override { }
	virtual void WaitFinish(bool needs_readpixel) override { nglFinish(); };
};


//
//	VirtualResolutionManagerComponent
//


class VirtualResolutionManagerComponent : public ScreenManagerComponent
{
public:
	static const KCL::uint32 BACKBUFFER_COUNT = 2;

	VirtualResolutionManagerComponent(TestBaseGFX *test, const TestDescriptor &td, KCL::uint32 native_width, KCL::uint32 native_height);

	virtual ~VirtualResolutionManagerComponent()
	{
	}

	virtual bool Init() override;
	virtual void Warmup() override;
	virtual void FinishRender() override;
	
	virtual void BeginFrame() override
	{
		// get next backbuffer id
		m_backbuffer_id = (m_backbuffer_id + 1) % BACKBUFFER_COUNT;
	}

	virtual uint32_t GetActiveBackbufferId() const override { return m_backbuffer_id; }
	virtual void GetTestSize(KCL::uint32 &width, KCL::uint32 &height) const override
	{
		width = m_test_width;
		height = m_test_height;
	}
	virtual void FinishTest() override { m_swapbuffer_needed = false; }
	virtual void WaitFinish(bool needs_readpixel) override { nglFinish(); };

protected:
	void InitGeometry();
	virtual void InitShaderCodes();

	KCL::uint32 m_test_width, m_test_height;
	KCL::uint32 m_backbuffer_id;

	KCL::uint32 m_vbo, m_ebo;
	KCL::uint32 m_onscreen_job;

	bool m_is_portrait;
	KCL::uint32 m_onscreen_shader_code;
	bool m_enable_screenshot;
};


VirtualResolutionManagerComponent::VirtualResolutionManagerComponent(TestBaseGFX *test, const TestDescriptor &td, KCL::uint32 native_width, KCL::uint32 native_height)
	: ScreenManagerComponent(test,native_width,native_height)
{
	m_test_width = td.m_test_width;
	m_test_height = td.m_test_height;

	m_backbuffer_id = BACKBUFFER_COUNT - 1;
	m_backbuffers.resize(2, 0);

	m_is_portrait = (m_native_height > m_native_width);
	m_onscreen_shader_code = SHADER_CODE_NORMAL;
	m_enable_screenshot = !test->GetTestDescriptor().m_screenshot_frames.empty();
}


bool VirtualResolutionManagerComponent::Init()
{
	// Create backbuffer textures
	for (size_t i = 0; i < BACKBUFFER_COUNT; i++)
	{
		m_backbuffers[i] = 0;
		NGL_texture_descriptor texture_layout;

		texture_layout.m_name = "screen_manager_backbuffer";
		texture_layout.m_size[0] = m_test_width;
		texture_layout.m_size[1] = m_test_height;
		texture_layout.m_format = NGL_R8_G8_B8_A8_UNORM;
		texture_layout.m_num_levels = 1;
		texture_layout.m_filter = NGL_NEAREST;
		texture_layout.m_wrap_mode = NGL_CLAMP_ALL;
		texture_layout.m_type = NGL_TEXTURE_2D;
		texture_layout.m_is_renderable = true;
		texture_layout.m_is_transfer_source = m_enable_screenshot;

		texture_layout.m_clear_value[0] = 0.0f;
		texture_layout.m_clear_value[1] = 0.0f;
		texture_layout.m_clear_value[2] = 0.0f;
		texture_layout.m_clear_value[3] = 0.0f;

		nglGenTexture(m_backbuffers[i], texture_layout, 0);
		GFXB::Transitions::Get().Register(m_backbuffers[i], texture_layout);
	}

	// Create onscreen job
	{
		NGL_job_descriptor jd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = 0;
			ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			jd.m_attachments.push_back(ad);
		}
		{
			NGL_subpass sp;
			sp.m_name = "screen_manager_onscreen_job";
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
			jd.m_subpasses.push_back(sp);
		}

		jd.m_load_shader_callback = OffscreenManagerLoadShader;
		jd.m_user_data = this;

		m_onscreen_job = nglGenJob(jd);
	}

	InitGeometry();
	InitShaderCodes();

	return true;
}


void VirtualResolutionManagerComponent::Warmup()
{
	m_is_warmup = true;

	nglBeginCommandBuffer(m_command_buffer);

	FinishRender();

	nglEndCommandBuffer(m_command_buffer);
	nglSubmitCommandBuffer(m_command_buffer);

	m_is_warmup = false;
}


void VirtualResolutionManagerComponent::InitShaderCodes()
{
	uint32_t raster_control_mode = nglGetInteger(NGL_RASTERIZATION_CONTROL_MODE);

	switch (raster_control_mode)
	{
	case NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP:
		m_onscreen_shader_code = (m_is_portrait) ? SHADER_CODE_FLIPPED_ROTATED : SHADER_CODE_FLIPPED;
		break;
	case NGL_ORIGIN_UPPER_LEFT:
		m_onscreen_shader_code = (m_is_portrait) ? SHADER_CODE_FLIPPED_ROTATED : SHADER_CODE_NORMAL;
		break;
	default:
		m_onscreen_shader_code = (m_is_portrait) ? SHADER_CODE_ROTATED : SHADER_CODE_NORMAL;
		break;
	}
}


void VirtualResolutionManagerComponent::FinishRender()
{
	int32_t viewport[4] = { 0, 0, (int32_t)m_native_width, (int32_t)m_native_height };
	nglViewportScissor(m_onscreen_job, viewport, viewport);

	//nglBeginCommandBuffer(m_command_buffer);

	GFXB::Transitions::Get()
		.TextureBarrier(m_backbuffers[m_backbuffer_id], NGL_SHADER_RESOURCE)
		.Execute(m_command_buffer);

	nglBegin(m_onscreen_job, m_command_buffer);

	const void *p[UNIFORM_MAX];
	p[UNIFORM_TEX] = &m_backbuffers[m_backbuffer_id];

	nglDrawTwoSided(m_onscreen_job, m_onscreen_shader_code, m_vbo, m_ebo, p);

	nglEnd(m_onscreen_job);
	//nglEndCommandBuffer(m_command_buffer);
	//nglSubmitCommandBuffer(m_command_buffer);
}


void VirtualResolutionManagerComponent::InitGeometry()
{
	{
		float vertices[4][4];

		vertices[0][0] = -1.0f;
		vertices[0][1] = -1.0f;
		vertices[0][2] = 0.0f;
		vertices[0][3] = 0.0f;

		vertices[1][0] = +1.0f;
		vertices[1][1] = -1.0f;
		vertices[1][2] = 1.0f;
		vertices[1][3] = 0.0f;

		vertices[2][0] = +1.0f;
		vertices[2][1] = +1.0f;
		vertices[2][2] = 1.0f;
		vertices[2][3] = 1.0f;

		vertices[3][0] = -1.0f;
		vertices[3][1] = +1.0f;
		vertices[3][2] = 0.0f;
		vertices[3][3] = 1.0f;

		NGL_vertex_descriptor vl;
		NGL_vertex_attrib vla;

		vla.m_semantic = "in_position";
		vla.m_format = NGL_R32_G32_FLOAT;
		vla.m_offset = 0;
		vl.m_attribs.push_back(vla);

		vla.m_semantic = "in_texcoord";
		vla.m_format = NGL_R32_G32_FLOAT;
		vla.m_offset = sizeof(float) * 2;
		vl.m_attribs.push_back(vla);

		vl.m_stride = sizeof(float) * 4;

		m_vbo = 0;
		nglGenVertexBuffer(m_vbo, vl, 4, vertices);
	}
	{
		uint16_t indices[6] =
		{
			0, 1, 2, 0, 2, 3
		};

		m_ebo = 0;
		nglGenIndexBuffer(m_ebo, NGL_R16_UINT, 6, indices);
	}
}


//
//	OffScreenManagerComponent
//


class OffScreenManagerComponent : public VirtualResolutionManagerComponent
{
public:

	static const KCL::uint32 SAMPLE_W = 48; //480x270, 100 * 48x27 sample
	static const KCL::uint32 SAMPLE_H = 27;
	static const KCL::uint32 SAMPLE_NUM_X = 10;
	static const KCL::uint32 SAMPLE_NUM_Y = 10;
	static const KCL::uint32 SAMPLE_C = SAMPLE_NUM_X * SAMPLE_NUM_Y;
	static const KCL::uint32 MOSAIC_WIDTH = SAMPLE_NUM_X * SAMPLE_W;
	static const KCL::uint32 MOSAIC_HEIGHT = SAMPLE_NUM_Y * SAMPLE_H;
	static const KCL::uint32 FLUSH_MOSAIC_TIME_WINDOW = 5000;

	KCL::uint32 max_buffered_offscreen_tile_count;
	KCL::uint32 buffered_offscreen_tile_count;

	virtual bool Init() override;
	
	virtual void BeginFrame() override
	{
		VirtualResolutionManagerComponent::BeginFrame();
		
		// get next mosaic id
		m_mosaic_id = (m_mosaic_id + 1) % SAMPLE_C;
	}
	
	virtual void FinishRender() override;

	virtual void BeginTest() override;

	OffScreenManagerComponent(TestBaseGFX *test, const TestDescriptor &td, KCL::uint32 native_width, KCL::uint32 native_height);
	virtual ~OffScreenManagerComponent();

	virtual void FinishTest() override;
	virtual void WaitFinish(bool needs_readpixel) override;

protected:
	void DrawMosaicToOnscreen();
	virtual void InitShaderCodes() override;

	KCL::uint32 m_mosaic_texture;
	KCL::uint32 m_mosaic_job;
	KCL::uint32 m_mosaic_id;
	KCL::uint32 m_mosaic_shader_code;
	int32_t m_onscr_mosaic_viewport[4];

	int64_t m_last_swapbuffer;
	TimeComponent *m_time_component;

	KCL::uint32 m_raster_control_mode;

	KCL::uint32 m_wait_finish_texture;
	KCL::uint32 m_wait_finish_job;
};


OffScreenManagerComponent::OffScreenManagerComponent(TestBaseGFX *test, const TestDescriptor &td, KCL::uint32 native_width, KCL::uint32 native_height)
	: VirtualResolutionManagerComponent(test, td, native_width, native_height)
{
	m_mosaic_id = SAMPLE_C - 1;
	m_mosaic_texture = 0;

	m_raster_control_mode = NGL_ORIGIN_LOWER_LEFT;

	m_mosaic_shader_code = SHADER_CODE_NORMAL;

	m_wait_finish_texture = 0;
	m_wait_finish_job = 0;

	max_buffered_offscreen_tile_count = td.max_offscreen_tile;
	buffered_offscreen_tile_count = 0;
}


OffScreenManagerComponent::~OffScreenManagerComponent()
{
}


bool OffScreenManagerComponent::Init()
{
	m_raster_control_mode = nglGetInteger(NGL_RASTERIZATION_CONTROL_MODE);

	m_time_component = dynamic_cast<TimeComponent*>(m_test->GetTestComponent(TimeComponent::NAME));

	VirtualResolutionManagerComponent::Init();

	// Create mosaic texture and job
	{
		NGL_texture_descriptor texture_layout;

		texture_layout.m_name = "offscreen_mosaic_texture";
		texture_layout.m_size[0] = MOSAIC_WIDTH;
		texture_layout.m_size[1] = MOSAIC_HEIGHT;
		texture_layout.m_format = NGL_R8_G8_B8_A8_UNORM;
		texture_layout.m_num_levels = 1;
		texture_layout.m_filter = NGL_NEAREST;
		texture_layout.m_wrap_mode = NGL_CLAMP_ALL;
		texture_layout.m_type = NGL_TEXTURE_2D;
		texture_layout.m_is_renderable = true;

		texture_layout.m_clear_value[0] = 0.0f;
		texture_layout.m_clear_value[1] = 0.0f;
		texture_layout.m_clear_value[2] = 0.0f;
		texture_layout.m_clear_value[3] = 0.0f;
		nglGenTexture(m_mosaic_texture, texture_layout, 0);
		GFXB::Transitions::Get().Register(m_mosaic_texture, texture_layout);

		NGL_job_descriptor jd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_mosaic_texture;
			ad.m_attachment_load_op = NGL_LOAD_OP_LOAD;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			jd.m_attachments.push_back(ad);
		}
		{
			NGL_subpass sp;
			sp.m_name = "offscreen_manager_mosaic_job";
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
			jd.m_subpasses.push_back(sp);
		}

		jd.m_load_shader_callback = OffscreenManagerLoadShader;
		jd.m_user_data = this;

		m_mosaic_job = nglGenJob(jd);
	}

	// clear mosaic fbo
	{
		NGL_job_descriptor jd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_mosaic_texture;
			ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			jd.m_attachments.push_back(ad);
		}
		{
			NGL_subpass sp;
			sp.m_name = "offscreen_manager_clear_mosaic_job";
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
			jd.m_subpasses.push_back(sp);
		}

		jd.m_load_shader_callback = OffscreenManagerLoadShader;
		jd.m_user_data = this;

		KCL::uint32 clear_mosaic_job = nglGenJob(jd);

		nglBeginCommandBuffer(m_command_buffer);
		GFXB::Transitions::Get().TextureBarrier(m_mosaic_texture, NGL_COLOR_ATTACHMENT).Execute(m_command_buffer);
		nglBegin(clear_mosaic_job, m_command_buffer);
		nglEnd(clear_mosaic_job);
		nglEndCommandBuffer(m_command_buffer);
		nglSubmitCommandBuffer(m_command_buffer);
	}

	// Create wait finish job and texture
	{
		NGL_texture_descriptor texture_layout;

		texture_layout.m_name = "offscreen_finish_texture";
		texture_layout.m_size[0] = NGL_WAIT_FINISH_RT_WIDTH;
		texture_layout.m_size[1] = NGL_WAIT_FINISH_RT_HEIGHT;
		texture_layout.m_format = NGL_R8_G8_B8_A8_UNORM;
		texture_layout.m_num_levels = 1;
		texture_layout.m_filter = NGL_NEAREST;
		texture_layout.m_wrap_mode = NGL_CLAMP_ALL;
		texture_layout.m_type = NGL_TEXTURE_2D;
		texture_layout.m_is_renderable = true;

		texture_layout.m_clear_value[0] = 0.0f;
		texture_layout.m_clear_value[1] = 0.0f;
		texture_layout.m_clear_value[2] = 0.0f;
		texture_layout.m_clear_value[3] = 0.0f;

		nglGenTexture(m_wait_finish_texture, texture_layout, 0);
		//GFXB::Transitions::Get().Register(m_wait_finish_texture, texture_layout);

		NGL_job_descriptor jd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_wait_finish_texture;
			ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			jd.m_attachments.push_back(ad);
		}
		{
			NGL_subpass sp;
			sp.m_name = "screen_manager_force_finish_job";
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
			jd.m_subpasses.push_back(sp);
		}

		jd.m_load_shader_callback = OffscreenManagerLoadShader;
		jd.m_user_data = this;

		m_wait_finish_job = nglGenJob(jd);
	}

	if (!m_is_portrait)
	{
		m_onscr_mosaic_viewport[0] = (int32_t)(m_native_width - MOSAIC_WIDTH) / 2;
		m_onscr_mosaic_viewport[1] = (int32_t)(m_native_height - MOSAIC_HEIGHT) / 2;
		m_onscr_mosaic_viewport[2] = (int32_t)MOSAIC_WIDTH;
		m_onscr_mosaic_viewport[3] = (int32_t)MOSAIC_HEIGHT;
	}
	else
	{
		m_onscr_mosaic_viewport[0] = (int32_t)(m_native_width - MOSAIC_HEIGHT) / 2;
		m_onscr_mosaic_viewport[1] = (int32_t)(m_native_height - MOSAIC_WIDTH) / 2;
		m_onscr_mosaic_viewport[2] = (int32_t)MOSAIC_HEIGHT;
		m_onscr_mosaic_viewport[3] = (int32_t)MOSAIC_WIDTH;
	}

	return true;
}


void OffScreenManagerComponent::InitShaderCodes()
{
	switch (m_raster_control_mode)
	{
	case NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP:
		m_onscreen_shader_code = (m_is_portrait) ? SHADER_CODE_FLIPPED_ROTATED : SHADER_CODE_FLIPPED;
		m_mosaic_shader_code = SHADER_CODE_FLIPPED;
		break;
	case NGL_ORIGIN_UPPER_LEFT:
		m_onscreen_shader_code = (m_is_portrait) ? SHADER_CODE_ROTATED : SHADER_CODE_NORMAL;
		m_mosaic_shader_code = (m_is_portrait) ? SHADER_CODE_FLIPPED : SHADER_CODE_NORMAL;
		break;
	default:
		m_onscreen_shader_code = (m_is_portrait) ? SHADER_CODE_ROTATED : SHADER_CODE_NORMAL;
		m_mosaic_shader_code = SHADER_CODE_NORMAL;
		break;
	}
}


void OffScreenManagerComponent::FinishRender()
{
	m_swapbuffer_needed = false;

	//nglBeginCommandBuffer(m_command_buffer);

	// render to the mosaic
	{
		// setup viewport
		{
			KCL::int32 x = m_mosaic_id % SAMPLE_NUM_X;
			KCL::int32 y = m_mosaic_id / SAMPLE_NUM_Y;

			int32_t viewport[4] = { 0, 0, (int32_t)SAMPLE_W,(int32_t)SAMPLE_H };

			switch (m_raster_control_mode)
			{
			case NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP:
				viewport[0] = (int32_t)(x * SAMPLE_W);
				viewport[1] = (int32_t)((SAMPLE_NUM_Y - y - 1) * SAMPLE_H);
				break;
			case NGL_ORIGIN_UPPER_LEFT:
				if (m_is_portrait)
				{
					viewport[0] = (int32_t)((SAMPLE_NUM_X - x - 1) * SAMPLE_W);
					viewport[1] = (int32_t)(y * SAMPLE_H);
				}
				else
				{
					viewport[0] = (int32_t)(x * SAMPLE_W);
					viewport[1] = (int32_t)((SAMPLE_NUM_Y - y - 1) * SAMPLE_H);
				}
				break;
			default:
				viewport[0] = (int32_t)(x * SAMPLE_W);
				viewport[1] = (int32_t)(y * SAMPLE_H);
				break;
			}

			nglViewportScissor(m_mosaic_job, viewport, viewport);
		}

		GFXB::Transitions::Get()
			.TextureBarrier(m_mosaic_texture, NGL_COLOR_ATTACHMENT)
			.TextureBarrier(m_backbuffers[m_backbuffer_id], NGL_SHADER_RESOURCE)
			.Execute(m_command_buffer);

		nglBegin(m_mosaic_job, m_command_buffer);

		const void *p[UNIFORM_MAX];
		p[UNIFORM_TEX] = &m_backbuffers[m_backbuffer_id];
		nglDrawTwoSided(m_mosaic_job, m_mosaic_shader_code, m_vbo, m_ebo,p);

		nglEnd(m_mosaic_job);
	}

	if (m_is_warmup)
	{
		DrawMosaicToOnscreen();
		return;
	}

	buffered_offscreen_tile_count++;

	bool draw_mosaic = false;
	int64_t time = m_time_component->GetElapsedTime();
	draw_mosaic |= ( (m_mosaic_id+1) == SAMPLE_C);
	draw_mosaic |= ( (time - m_last_swapbuffer) >= FLUSH_MOSAIC_TIME_WINDOW);
	draw_mosaic |= (buffered_offscreen_tile_count >= max_buffered_offscreen_tile_count);

	if (draw_mosaic)
	{
		DrawMosaicToOnscreen();

		m_last_swapbuffer = time;
		m_swapbuffer_needed = true;
		buffered_offscreen_tile_count = 0;
	}
	else
	{
		nglFlush();
	}

	//nglEndCommandBuffer(m_command_buffer);
	//nglSubmitCommandBuffer(m_command_buffer);
}


void OffScreenManagerComponent::DrawMosaicToOnscreen()
{
	nglViewportScissor(m_onscreen_job, m_onscr_mosaic_viewport, m_onscr_mosaic_viewport);

	GFXB::Transitions::Get().TextureBarrier(m_mosaic_texture, NGL_SHADER_RESOURCE).Execute(m_command_buffer);

	nglBegin(m_onscreen_job, m_command_buffer);

	const void *p[UNIFORM_MAX];
	p[UNIFORM_TEX] = &m_mosaic_texture;

	nglDrawTwoSided(m_onscreen_job, m_onscreen_shader_code, m_vbo, m_ebo, p);

	nglEnd(m_onscreen_job);
}


void OffScreenManagerComponent::WaitFinish(bool needs_readpixel)
{
	if (needs_readpixel)
	{
		nglBeginCommandBuffer(m_command_buffer);

		int32_t m_wait_finish_viewport[4] = { 0, 0, NGL_WAIT_FINISH_RT_WIDTH, NGL_WAIT_FINISH_RT_HEIGHT };
		nglViewportScissor(m_wait_finish_job, m_wait_finish_viewport, m_wait_finish_viewport);

		GFXB::Transitions::Get().TextureBarrier(m_mosaic_texture, NGL_SHADER_RESOURCE).Execute(m_command_buffer);

		nglBegin(m_wait_finish_job, m_command_buffer);

		const void *p[UNIFORM_MAX];
		p[UNIFORM_TEX] = &m_mosaic_texture;

		nglDrawTwoSided(m_wait_finish_job, m_onscreen_shader_code, m_vbo, m_ebo, p);

		nglEnd(m_wait_finish_job);

		nglEndCommandBuffer(m_command_buffer);
		nglSubmitCommandBuffer(m_command_buffer);

		nglCustomAction(m_wait_finish_job, NGL_CUSTOM_ACTION_WAIT_FINISH);
	}
	else
	{
		nglFinish();
	}
}


void OffScreenManagerComponent::FinishTest()
{
	if (m_mosaic_id != 0)
	{
		nglBeginCommandBuffer(m_command_buffer);

		DrawMosaicToOnscreen();
		m_swapbuffer_needed = true;

		nglEndCommandBuffer(m_command_buffer);
		nglSubmitCommandBuffer(m_command_buffer);
	}
	else
	{
		m_swapbuffer_needed = false;
	}
}


void OffScreenManagerComponent::BeginTest()
{
	m_last_swapbuffer = m_time_component->GetElapsedTime();
}


ScreenManagerComponent* ScreenManagerComponent::Create(TestBaseGFX* test, const TestDescriptor &td, uint32_t native_width, uint32_t native_height)
{
	if (td.GetScreenMode() == SMode_Onscreen)
	{
		bool use_virtual_resolution = td.m_virtual_resolution;
#ifdef ANDROID
		use_virtual_resolution = false;
#endif // ANDROID

		if (!use_virtual_resolution)
		{
			INFO("Using OnScreenManager");
			return new OnScreenManagerComponent(test, native_width, native_height);
		}
		else
		{
			INFO("Using VirtualResolutionManager");
			return new VirtualResolutionManagerComponent(test, td, native_width, native_height);
		}
	}
	else if (td.GetScreenMode() == SMode_Offscreen)
	{
		INFO("Using OffScreenManager");
		return new OffScreenManagerComponent(test, td, native_width, native_height);
	}
	return nullptr;
}


void GLB::ScreenManagerComponent::ClearScreen()
{
	NGL_job_descriptor jd;
	{
		NGL_attachment_descriptor ad;
		ad.m_attachment.m_idx = 0;
		ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
		ad.m_attachment_store_op = NGL_STORE_OP_STORE;
		jd.m_attachments.push_back(ad);
	}
	{
		NGL_subpass sp;
		sp.m_name = "screen_manager_clear_screen_job";
		sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
		jd.m_subpasses.push_back(sp);
	}

	jd.m_load_shader_callback = OffscreenManagerLoadShader;
	jd.m_user_data = nullptr;

	KCL::uint32 clear_screen_job = nglGenJob(jd);

	nglBeginCommandBuffer(m_command_buffer);
	nglBegin(clear_screen_job, m_command_buffer);
	nglEnd(clear_screen_job);
	nglEndCommandBuffer(m_command_buffer);
	nglSubmitCommandBuffer(m_command_buffer);
}
