/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <ppltasks.h>

#include "offscrman.h"
#include "kcl_image.h"
#include "kcl_os.h"
#include "misc2.h"
#include "platform.h"
#include "test_descriptor.h"

#include "kcl_base.h"
#include "d3d11/vbopool.h"

#include "fbo3.h"

using namespace GLB;

OffscreenManager* OffscreenManager::Create(const GlobalTestEnvironment* const gte, int w, int h)
{
	return new OffscreenManager( w, h);
}

OffscreenManager::OffscreenManager( int w, int h) : m_offscreen_default_viewport_width( w), m_offscreen_default_viewport_height( h)
{
	srand( time(0));

	memset(m_sample_times, 0, sizeof(KCL::uint32)*SAMPLE_C);
	memset(m_sample_coords_x, 0, sizeof(KCL::uint32)*SAMPLE_C);
	memset(m_sample_coords_y, 0, sizeof(KCL::uint32)*SAMPLE_C);
	memset(m_mosaic_coords_x, 0, sizeof(KCL::uint32)*SAMPLE_C);
	memset(m_mosaic_coords_y, 0, sizeof(KCL::uint32)*SAMPLE_C);

	m_width = 0;
	m_height = 0;
	//m_globalFBO = 0;
	m_globalFBO = 0;
	
	m_scratch_fbo[0] = 0;
	m_scratch_fbo[1] = 0;
	m_current_fbo_index = 0;
	m_mosaic_fbo = 0;

	for (KCL::uint32 i = 0; i < NUM_OFFSCREEN_FBOS; ++i)
	{
		m_offscreen_FBO[i] = 0;
		m_offscreen_query[i] = nullptr;
	}

	m_onscr_mosaic_fbo = 0;
	m_onscr_mosaic_idx = 0;
	m_onscr_mosaic_program[0] = 0;
	m_onscr_mosaic_program[1] = 0;
	m_onscr_mosaic_text_unif_loc[0] = 0;
	m_onscr_mosaic_text_unif_loc[1] = 0;
	init_onscr_mosaic_coords();

	m_offscreen_vertex_shader = 0;
	m_offscreen_fragment_shader = 0;
	m_offscreen_program = 0;
	m_offscreen_vbo = 0;
	m_offscreen_ebo = 0;
	m_offscr_text_unif_loc = 0;
	
	m_next_slot_time_interval = 0;
	m_next_slot_previous_time = 0;
	m_mosaic_idx = 0;

	m_msaa = 1;
}


void OffscreenManager::init_onscr_mosaic_coords()
{
	for(KCL::uint32 i=0; i<ONSCR_SAMPLE_C; ++i)
	{
		m_onscr_mosaic_coords_x[i] = (i % ONSCR_SAMPLE_NUM_X) * ONSCR_SAMPLE_W;
		m_onscr_mosaic_coords_y[i] = (i / ONSCR_SAMPLE_NUM_X) * ONSCR_SAMPLE_H;
	}
}


OffscreenManager::~OffscreenManager()
{
	Clear();

//	delete m_GLState;
}


bool OffscreenManager::createOffscrProg()
{
	m_globalFBO = FBO::GetGlobalFBO();

	KCL::KCL_Status error;
	m_shader         = Shader::CreateShader("offscreenmanager.vs",         "offscreenmanager.fs", 0, error);
	m_shader_rotated = Shader::CreateShader("offscreenmanager.rotated.vs", "offscreenmanager.fs", 0, error);
	
	if (m_shader == NULL || m_shader_rotated == NULL)
	{
		return false;
	}

	const float vbodata[16] = {
	 -1.0f, //0 vertex
	 -1.0f, //0 vertex
	  0.0f, //0 tc
	  0.0f, //0 tc
	  1.0f, //1 vertex
	 -1.0f, //1 vertex
	  1.0f, //1 tc
	  0.0f, //1 tc
	  1.0f, //2 vertex
	  1.0f, //2 vertex
	  1.0f, //2 tc
	  1.0f, //2 tc
	 -1.0f, //3 vertex
	  1.0f, //3 vertex
	  0.0f, //3 tc
	  1.0f  //3 tc
	};

	const KCL::uint16 idxdata[] = {0, 1, 2, 0, 2, 3}; //6
	
	KCL::uint32 offset;
	VboPool::Instance()->AddData(16*sizeof(float), (const void*)vbodata, m_onscr_mosaic_vbo, offset, 4*sizeof(float));
	IndexBufferPool::Instance()->AddData(6*sizeof(KCL::uint16), (const void*)idxdata, m_onscr_mosaic_ebo, offset);

	

	std::vector<D3D11_INPUT_ELEMENT_DESC> vertexDesc;
		
	D3D11_INPUT_ELEMENT_DESC position   = { "POSITION",    0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	D3D11_INPUT_ELEMENT_DESC texcoord0  = { "TEX_COORD",   0, DXGI_FORMAT_R32G32_FLOAT,	   0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		
	vertexDesc.push_back( position   );
	vertexDesc.push_back( texcoord0  );

	DX_THROW_IF_FAILED(
		DX::getDevice()->CreateInputLayout(
			&vertexDesc[0],
			vertexDesc.size(),
			&m_shader->m_vs.m_bytes[0],
			m_shader->m_vs.m_bytes.size(),
			&m_onscr_mosaic_inputlayout
			)
		);

	return true;
}	 


void OffscreenManager::deleteOffscrProg()
{
	for (KCL::uint32 i = 0; i < NUM_OFFSCREEN_FBOS; ++i)
	{
		delete m_offscreen_FBO[i];
		m_offscreen_FBO[i]=0;
	}
	delete m_scratch_fbo[0];
	delete m_scratch_fbo[1];
	delete m_mosaic_fbo;
	delete m_onscr_mosaic_fbo;

	m_scratch_fbo[0] = 0;
	m_scratch_fbo[1] = 0;
	m_mosaic_fbo = 0;
	m_onscr_mosaic_fbo = 0;

	m_onscr_mosaic_idx = 0;
	if(!m_offscreen_program == 0)
	{
		return;
	}

	m_offscreen_program = 0;

	if(m_onscr_mosaic_program[0])
	{
		m_onscr_mosaic_program[0] = 0;
	}
	if(m_onscr_mosaic_program[1])
	{
		m_onscr_mosaic_program[1] = 0;
	}	
}


void OffscreenManager::renderRectWithTexture(KCL::uint32 textureId, TextureCoordinatesMode textureCoordinatesMode) const
{
	return;
	//glUseProgram(m_offscreen_program);
	//  PostRender calls glUseProgram(m_offscreen_program) only once!
	//  Also, glUseProgram(0) isn't needed, m_GLState->Pop() takes care at the end of PostRender

	size_t offset = 0;

	switch ( textureCoordinatesMode)
	{
	case USE_MOSAIC:
		offset = (m_mosaic_idx + 1) * 8 * sizeof(float);
		break;
	case USE_SCRATCH:
		offset = (8 * SAMPLE_C + 8) * sizeof(float);
		break;
	case USE_FULL:
		offset = (8 * SAMPLE_C + 16) * sizeof(float);
		break;
	}
}

void OffscreenManager::renderRectWithTexture(ID3D11ShaderResourceView *texture, TextureCoordinatesMode textureCoordinatesMode) const
{
	DX::getContext()->PSSetShaderResources(0, 1, &texture);

	switch ( textureCoordinatesMode)
	{
	case USE_MOSAIC:
	case USE_SCRATCH:
	case USE_FULL:
		{
			VboPool::Instance()->BindBuffer(m_onscr_mosaic_vbo);
			DX::getContext()->IASetInputLayout(m_onscr_mosaic_inputlayout.Get());
		}
		break;
	}

	IndexBufferPool::Instance()->BindBuffer(m_onscr_mosaic_ebo);
	
	DX::getStateManager()->ApplyStates();
	DX::getContext()->DrawIndexed(
		6,
		0,
		0
		);

}


void OffscreenManager::calcSampleTexCoords(size_t idx, float &X_L, float &Y_D, float &X_R, float &Y_U)
{
	KCL::uint32 xui = rand() % (m_width - SAMPLE_W);
	KCL::uint32 yui = rand() % (m_height - SAMPLE_H);

	X_L = (float)xui / (float)m_width;
	Y_D = (float)yui / (float)m_height;
	X_R = X_L + (float)SAMPLE_W / m_width;
	Y_U = Y_D + (float)SAMPLE_H / m_height;

	m_sample_coords_x[idx] = xui;
	m_sample_coords_y[idx] = yui;
}


void OffscreenManager::clear_saved_sample_data()
{
	memset(m_sample_times, 0, sizeof(KCL::uint32)*SAMPLE_C);
	memset(m_sample_coords_x, 0, sizeof(KCL::uint32)*SAMPLE_C);
	memset(m_sample_coords_y, 0, sizeof(KCL::uint32)*SAMPLE_C);
	memset(m_mosaic_coords_x, 0, sizeof(KCL::uint32)*SAMPLE_C);
	memset(m_mosaic_coords_y, 0, sizeof(KCL::uint32)*SAMPLE_C);
	m_mosaic_idx = 0;
}


void OffscreenManager::renderToScratch() const
{
	return;
	FBO::bind( m_scratch_fbo[m_current_fbo_index] );	

	//renderRectWithTexture(m_offscreen_fbo[m_current_fbo_index]->getTextureName(), USE_MOSAIC);
}


void OffscreenManager::renderScratchToMosaic() const
{
	return;
	FBO::bind( m_mosaic_fbo);
	//NO CLEAR!!! We use m_mosaic_fbo for validation!!!
	setMosaicViewPort();

	//renderRectWithTexture(m_scratch_fbo[m_current_fbo_index]->getTextureName(), USE_SCRATCH);
}


void OffscreenManager::renderScratchToBackScreen() const
{
	return;
	FBO::bind( 0 );

	//renderRectWithTexture(m_scratch_fbo[m_current_fbo_index]->getTextureName(), USE_SCRATCH);
}


void OffscreenManager::renderToOnscrMosaic()
{
	m_onscr_mosaic_idx %= ONSCR_SAMPLE_C;

	FBO::bind( m_onscr_mosaic_fbo );
	if( m_onscr_mosaic_idx == 0 )
	{
		FBO::clear( m_onscr_mosaic_fbo, 0, 0, 0, 1);
	}

	setOnscrSampleViewPort();
	++m_onscr_mosaic_idx;
	

	renderRectWithTexture( m_offscreen_FBO[m_current_fbo_index]->Get(), USE_FULL);
}


void OffscreenManager::renderOnscrMosaicToBackScreen(const bool isRotated) const
{
	FBO::bind( 0 );
	
	Shader *currShader = ( isRotated ? m_shader_rotated : m_shader);
	currShader->Bind();
	
	FBO::bind( 0 );
	DX::Clear(0,0,0,1);

	setOnscrMosaicViewPort(isRotated);

	renderRectWithTexture(m_onscr_mosaic_fbo->Get(), USE_FULL);
}


void OffscreenManager::setOnscrSampleViewPort() const
{
	CD3D11_VIEWPORT viewPort(m_onscr_mosaic_coords_x[m_onscr_mosaic_idx], m_onscr_mosaic_coords_y[m_onscr_mosaic_idx], ONSCR_SAMPLE_W, ONSCR_SAMPLE_H);
	
	DX::getStateManager()->SetViewport(viewPort);
}


void OffscreenManager::setOnscrMosaicViewPort(const bool isRotated) const
{
	CD3D11_VIEWPORT viewPort( m_onscr_mosaic_x[isRotated], m_onscr_mosaic_y[isRotated], m_onscr_mosaic_viewport_width[isRotated], m_onscr_mosaic_viewport_height[isRotated]);
	
	DX::getStateManager()->SetViewport(viewPort);
   
}


int OffscreenManager::Init( unsigned int onscreen_width, unsigned int onscreen_height, const TestDescriptor &td)
{
	FBO_COLORMODE color_mode;
	FBO_DEPTHMODE depth_mode;

	m_refresh_msec = td.m_hybrid_refresh_msec;
	m_method = (SMode_Offscreen == td.GetScreenMode() ? OM_ORIGINAL : OM_HYBRID);

	int w = td.m_viewport_width == -1 ? m_offscreen_default_viewport_width : td.m_viewport_width;
	int h = td.m_viewport_height == -1 ? m_offscreen_default_viewport_height : td.m_viewport_height;

	m_state.m_blend.RenderTarget[0].BlendEnable = false;
	m_state.m_rasterizer.CullMode = D3D11_CULL_NONE;
	m_state.m_depth_stencil.DepthEnable = false;
	
#if 0
	int max_texture_dim;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_dim);
	INFO("Max texture size: %d" , max_texture_dim);

	if( w > max_texture_dim || h > max_texture_dim)
	{
		return KCL::KCL_TESTERROR_INVALID_SCREEN_RESOLUTION;
	}

	if( m_offscreen_default_viewport_width > max_texture_dim || m_offscreen_default_viewport_height > max_texture_dim)
	{
		return KCL::KCL_TESTERROR_INVALID_SCREEN_RESOLUTION;
	}
#endif

	if( w < 1 || h < 1)
	{
		return KCL::KCL_TESTERROR_INVALID_SCREEN_RESOLUTION;
	}

	FBO::SetGlobalFBO(NULL);

	if( td.m_play_time < 1)
		assert(0);

	m_width = w;
	m_height = h;

	offscreen_viewport.Width = w;
	offscreen_viewport.Height = h;
	offscreen_viewport.TopLeftX = 0;
	offscreen_viewport.TopLeftY = 0;
	offscreen_viewport.MinDepth = 0;
	offscreen_viewport.MaxDepth = 1;

	m_onscr_mosaic_viewport_width [0] = ONSCR_MOSAIC_WIDTH;
	m_onscr_mosaic_viewport_height[0] = ONSCR_MOSAIC_HEIGHT;

	m_onscr_mosaic_viewport_width [1] = ONSCR_MOSAIC_HEIGHT;
	m_onscr_mosaic_viewport_height[1] = ONSCR_MOSAIC_WIDTH;
	
	m_onscr_mosaic_x[0] = onscreen_width  > m_onscr_mosaic_viewport_width [0] ? (onscreen_width  - m_onscr_mosaic_viewport_width [0]) / 2 : 0;
	m_onscr_mosaic_y[0] = onscreen_height > m_onscr_mosaic_viewport_height[0] ? (onscreen_height - m_onscr_mosaic_viewport_height[0]) / 2 : 0;
	
	m_onscr_mosaic_x[1] = onscreen_width  > m_onscr_mosaic_viewport_width [1] ? (onscreen_width  - m_onscr_mosaic_viewport_width [1]) / 2 : 0;
	m_onscr_mosaic_y[1] = onscreen_height > m_onscr_mosaic_viewport_height[1] ? (onscreen_height - m_onscr_mosaic_viewport_height[1]) / 2 : 0;

	bool succeeded = createOffscrProg();
	if (!succeeded)
	{
		return KCL::KCL_TESTERROR_SHADER_ERROR;
	}

	m_next_slot_time_interval = (KCL::uint32)td.m_play_time / (int)SAMPLE_C;
	if((KCL::uint32)td.m_play_time % (int)SAMPLE_C)
	{
		++m_next_slot_time_interval;
	}
	m_next_slot_previous_time = 0;
	
	//m_globalFBO = (uint32)g_os->GetGlobalFBO();
	m_globalFBO = FBO::GetGlobalFBO();

	if( td.m_color_bpp >= 24)
	{
		color_mode = RGB888_Linear;
	}
	else
	{
		color_mode = RGB565_Linear;
	}

	if( td.m_depth_bpp >= 24)
	{
		depth_mode = DEPTH_24_RB;
	}
	else
	{
		depth_mode = DEPTH_16_RB;
	}

#if 0
	bool fbo_success =
		m_offscreen_fbo[0]->init( m_width, m_height, color_mode, depth_mode) &&
		m_offscreen_fbo[1]->init( m_width, m_height, color_mode, depth_mode) &&
		m_mosaic_fbo->init(MOSAIC_WIDTH, MOSAIC_HEIGHT, color_mode, DEPTH_None) &&
		m_scratch_fbo[0]->init(SCRATCH_WIDTH, SCRATCH_HEIGHT, color_mode, DEPTH_None) &&
		m_scratch_fbo[1]->init(SCRATCH_WIDTH, SCRATCH_HEIGHT, color_mode, DEPTH_None) &&
		m_onscr_mosaic_fbo->init(ONSCR_MOSAIC_WIDTH, ONSCR_MOSAIC_HEIGHT, color_mode, DEPTH_None);
	
	if(fbo_success==false)
	{
		return KCL::KCL_TESTERROR_OFFSCREEN_NOT_SUPPORTED;
	}

	g_os->SetGlobalFBO( m_globalFBO );
#endif

	delete m_onscr_mosaic_fbo;
	m_onscr_mosaic_fbo = new FBO ();

	m_onscr_mosaic_fbo->init( ONSCR_MOSAIC_WIDTH, ONSCR_MOSAIC_HEIGHT,  color_mode, DEPTH_None);

	m_msaa = td.m_fsaa ? 4 : 1;

	for (KCL::uint32 i = 0; i < NUM_OFFSCREEN_FBOS; ++i)
	{
		delete m_offscreen_FBO[i];
		m_offscreen_FBO[i] = new FBO;
		m_offscreen_FBO[i]->init( m_width, m_height, color_mode, depth_mode, m_msaa);
	}

	{
		bool isRotated = onscreen_width < onscreen_height;
		
		float vw = m_width;
		float vh = m_height;
		float w = !isRotated ? onscreen_width : onscreen_height;
		float h = !isRotated ? onscreen_height : onscreen_width;
		float var = vw / vh;
		float ar = w / h;

		if ( var > ar )
		{
			h = w / var;
		}
		else
		{
			w = var * h;
		}

		if (isRotated)
		{
			float tmp = w;
			w = h;
			h = tmp;
		}

		m_hybrid_onscreen_width = w;
		m_hybrid_onscreen_height = h;
		m_hybrid_onscreen_viewport = CD3D11_VIEWPORT(
			(onscreen_width-m_hybrid_onscreen_width)/2, 
			(onscreen_height-m_hybrid_onscreen_height)/2, 
			m_hybrid_onscreen_width, m_hybrid_onscreen_height
        );
	}

	return KCL::KCL_TESTERROR_NOERROR;
}


void OffscreenManager::PreRender() const
{
	ID3D11ShaderResourceView*const textureView[1] = {NULL};
	DX::getContext()->PSSetShaderResources(0, 1, textureView);
	FBO::SetGlobalFBO( m_offscreen_FBO[m_current_fbo_index]);
	FBO::bind(m_offscreen_FBO[m_current_fbo_index]);

	DX::getContext()->RSSetViewports(1, &offscreen_viewport);
}

bool OffscreenManager::SwapBuffers()
{
	//start current query
    const D3D11_QUERY_DESC queryDesc = {D3D11_QUERY_EVENT, 0};
	DX::getDevice()->CreateQuery(&queryDesc, &m_offscreen_query[m_current_fbo_index]);
	DX::getContext()->End(m_offscreen_query[m_current_fbo_index].Get());
	DX::getContext()->Flush();

	//wait and render previous frame, 
	if (++m_current_fbo_index >= NUM_OFFSCREEN_FBOS)
	{
		m_current_fbo_index = 0;
	}
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context = DX::getContext();
	Microsoft::WRL::ComPtr<ID3D11Query> query = m_offscreen_query[m_current_fbo_index];

    if (query)
	{
		HRESULT hr = context->GetData(query.Get(), NULL, 0, 0);
        if(hr != S_OK)
		{
			// Poll on the query, no need to flush the pipeline as we've flushed it above.
            while (context->GetData(query.Get(), 0, NULL, D3D11_ASYNC_GETDATA_DONOTFLUSH) != S_OK);
		}

        return true;
	}

	return false;
}


void OffscreenManager::PostRender(KCL::uint32 time, KCL::uint32 frame, int current_viewport_width, int current_viewport_height, bool force_swap_buffer)
{
	DX::getStateManager()->SetDepthEnabled(false);
	DX::getStateManager()->SetViewportDepthrange();
	DX::getStateManager()->SetBlendEnabled(true);
	DX::getStateManager()->SetBlendFunction(D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD);
	DX::getStateManager()->SetRasterCullMode(D3D11_CULL_NONE);
	DX::getStateManager()->ApplyStates(true);

	switch( m_method)
	{
	case OM_ORIGINAL:
		{
			PostRender_original( time, frame, current_viewport_width, current_viewport_height, force_swap_buffer);
			break;
		}
	case OM_HYBRID:
		{
			PostRender_hybrid( time, frame, current_viewport_width, current_viewport_height, force_swap_buffer);
			break;
		}
	}
}


void OffscreenManager::PostRender_original(KCL::uint32 time, KCL::uint32 frame, int current_viewport_width, int current_viewport_height, bool force_swap_buffer)
{
	if (SwapBuffers())
	{
		DX::getStateManager()->Push(m_state);

		m_shader->Bind();

		renderToScratch();

		if(time - m_next_slot_previous_time >= m_next_slot_time_interval)
		{
			renderScratchToMosaic();
			save_sample_time(time);
			m_next_slot_previous_time = time;
			++m_mosaic_idx;
		}

		renderToOnscrMosaic();

		//g_os->SetGlobalFBO( (void*)m_globalFBO );
		FBO::SetGlobalFBO( m_globalFBO );

		if(( frame % OFFSCR_RENDER_TIMER) == 0 || force_swap_buffer)
		{
			renderScratchToBackScreen();
		}

		if( IsSwapBufferNeeded() || force_swap_buffer)
		{
			renderOnscrMosaicToBackScreen(current_viewport_width < current_viewport_height);
		}

		DX::getStateManager()->Pop();
	}

}

void OffscreenManager::PostRender_hybrid(KCL::uint32 time, KCL::uint32 frame, int current_viewport_width, int current_viewport_height, bool force_swap_buffer)
{
	if (SwapBuffers())
	{
		DX::getStateManager()->Push(m_state);

		m_shader->Bind();

		renderToScratch();

		if(time - m_next_slot_previous_time >= m_next_slot_time_interval)
		{
			renderScratchToMosaic();
			save_sample_time(time);
			m_next_slot_previous_time = time;
			++m_mosaic_idx;
		}

		renderToOnscrMosaic();

		FBO::SetGlobalFBO( m_globalFBO );

		if(( frame % OFFSCR_RENDER_TIMER) == 0)
		{
			renderScratchToBackScreen();
		}

		//if( IsSwapBufferNeeded())
		//{
			//renderOnscrMosaicToBackScreen(current_viewport_width < current_viewport_height);
		//}

		if( time - m_last_refresh_msec >= m_refresh_msec)
		{
			bool isRotated = current_viewport_width < current_viewport_height;
			Shader *currShader = ( isRotated ? m_shader_rotated : m_shader);
			currShader->Bind();
	
			FBO::bind( 0 );
			DX::Clear(0,0,0,1);
		
			DX::getStateManager()->SetViewport(m_hybrid_onscreen_viewport);

			renderRectWithTexture( m_offscreen_FBO[!m_current_fbo_index]->Get(), USE_FULL);

			ID3D11ShaderResourceView*const textureView[1] = {NULL};
			DX::getContext()->PSSetShaderResources(0, 1, textureView);

			m_last_refresh_msec = time;
			m_onscr_mosaic_idx = ONSCR_SAMPLE_C;
		}

		DX::getStateManager()->Pop();
	}
}


void OffscreenManager::Clear()
{
	deleteOffscrProg();
	clear_saved_sample_data();
}


void OffscreenManager::SaveSamples(OffscrResults &results) const
{
	KCL::uint32 sample_bpp = 32;
	//sample_bpp = 16;

	FBO::bind( m_mosaic_fbo );
	for(size_t i = 0; i < m_mosaic_idx; ++i)
	{
		OffscrResultElement *e = new OffscrResultElement( 
			m_sample_times[i],
			m_sample_coords_x[i],
			m_sample_coords_y[i],
			SAMPLE_W,
			SAMPLE_H,
			sample_bpp);

		results.m_data.push_back( e);
	}
	FBO::bind( 0 );
}


void OffscreenManager::ResetGlobalFBO(int current_viewport_width, int current_viewport_height) const
{
	//g_os->SetGlobalFBO( (void*)m_globalFBO );
	FBO::SetGlobalFBO( m_globalFBO );
	FBO::bind( 0 );
	FBO::bind( 0 );
}


void OffscreenManager::setMosaicViewPort() const
{
	CD3D11_VIEWPORT viewPort(m_mosaic_coords_x[m_mosaic_idx], m_mosaic_coords_y[m_mosaic_idx], SAMPLE_W, SAMPLE_H);
	
	DX::getStateManager()->SetViewport(viewPort);
}

#include <kcl_io.h>
/* kuka
void OffscreenManager::SaveForTesting() const
{	
	KCL::uint8* captured_samples = new KCL::uint8[MOSAIC_WIDTH * MOSAIC_HEIGHT * 4];
	std::stringstream ss;
	int timestamp = (int)time(0);

	FBO::bind( m_mosaic_fbo );	
	FBO::bind( 0 );
	
	ss << "mosaic_fbo_" << timestamp << ".tga";
	convertRGBAtoBGR(captured_samples, MOSAIC_WIDTH * MOSAIC_HEIGHT);

	saveTga(ss.str().c_str(), MOSAIC_WIDTH, MOSAIC_HEIGHT, captured_samples, KCL::Image_RGB888);

	delete[] captured_samples;
}*/



void OffscrResultElement::serialize(std::string &result, bool usePercentEncodedPadding) const
{
	std::string buf2 = "";
	result = "OffscrResultElement_";
	const char sep = '|';

	{
		//*
		std::ostringstream os;	
		os << m_time << sep << m_x_offset << sep << m_y_offset << sep << m_sample_width << sep << m_sample_height << sep << m_sample_bpp << sep;

		result += os.str();
		//*/

		/*
		const size_t bufSz = 256;
		char buf[bufSz];
		memset(buf, 0, bufSz);

		sprintf(buf, "%u%c%u%c%u%c%u%c%u%c%u%c", m_time , sep , m_x_offset , sep , m_y_offset , sep , m_sample_width , sep , m_sample_height , sep , m_sample_bpp , sep);

		result += buf;
		//*/
	}

	/*
	std::string buf;

	EncodeBase64URL(buf, (const void*)m_sample, (size_t)(m_sample_width * m_sample_height * (m_sample_bpp / 8)), usePercentEncodedPadding);

	result += buf;
	//*/

	EncodeBase64URL(buf2, (const void*)m_sample, (size_t)(m_sample_width * m_sample_height * (m_sample_bpp / 8)), usePercentEncodedPadding);

	result += buf2;
}


void OffscrResults::serialize(std::string &result, bool usePercentEncodedPadding) const
{
	result = " OffscrResults_";
	const char sep = '|';

	{
		//*
		std::ostringstream os;	
		os << m_data.size();

		result += os.str();
		//*/

		/*
		const size_t bufSz = 32;
		char buf[bufSz];
		memset(buf, 0, bufSz);

		sprintf(buf, "%u", m_data.size() );

		result += buf;
		//*/
	}

	std::string buf;

	for(size_t i=0; i<m_data.size(); ++i)
	{
		result += sep;
		m_data[i]->serialize(buf, usePercentEncodedPadding);
		result += buf;
	}
}
