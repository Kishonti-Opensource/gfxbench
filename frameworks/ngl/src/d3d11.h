/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
inline void SetDebugObjectName(ID3D11DeviceChild *resource, const std::string &name)
{
#if defined(_DEBUG) || defined(PROFILE)
	HRESULT result = resource->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.length(), name.c_str());
	if (FAILED(result))
	{
		_logf("Can not set debug name for object: %s", name.c_str());
	}
#endif
}

struct D3D11_instance;

struct D3D11_texture : public NGL_texture
{
	D3D11_TEXTURE2D_DESC m_descriptor;

	ID3D11Texture2D *m_tex;

	ID3D11SamplerState *m_sampler;
	ID3D11ShaderResourceView *m_default_srv;
	ID3D11UnorderedAccessView *m_uav;

	std::vector<ID3D11ShaderResourceView*> m_sr_views;
	std::vector<ID3D11RenderTargetView*> m_rt_views;
	std::vector<ID3D11DepthStencilView*> m_ds_views;
	std::vector<ID3D11DepthStencilView*> m_readonly_ds_views;

	uint32_t m_subresource_count;

	D3D11_texture() : m_default_srv(0), m_tex(0), m_sampler(0), m_uav(0), m_subresource_count(0)
	{
		memset(&m_descriptor, 0, sizeof(D3D11_TEXTURE2D_DESC));
	}
	~D3D11_texture();

	void Release();

	ID3D11ShaderResourceView *GetSRV(const NGL_texture_subresource &subresource);
 	ID3D11RenderTargetView *GetRTV(const NGL_texture_subresource &subresource);
	ID3D11DepthStencilView *GetDSV(const NGL_texture_subresource &subresource);
	ID3D11DepthStencilView *GetReadOnlyDSV(const NGL_texture_subresource &subresource);
	uint32_t GetSubresourceIndex(const NGL_texture_subresource &subresource) const;
};


struct D3D11_vertex_buffer : public NGL_vertex_buffer
{
	ID3D11Buffer	*pVBuffer;
	ID3D11Buffer	*m_structured_buffer;
	ID3D11ShaderResourceView* m_srv;
	ID3D11UnorderedAccessView* m_uav;
	D3D11_SHADER_RESOURCE_VIEW_DESC m_base_srv_desc;
	D3D11_UNORDERED_ACCESS_VIEW_DESC m_base_uav_desc;
	std::map<uint64_t, ID3D11ShaderResourceView*> m_subresource_srvs;
	std::map<uint64_t, ID3D11UnorderedAccessView*> m_subresource_uavs;

	D3D11_vertex_buffer() : pVBuffer(0), m_structured_buffer(0), m_srv(0), m_uav(0)
	{

	}
	~D3D11_vertex_buffer();

	uint64_t GetKey(const NGL_buffer_subresource &subresource);
	ID3D11ShaderResourceView *GetSRV(const NGL_buffer_subresource &subresource);
	ID3D11UnorderedAccessView *GetUAV(const NGL_buffer_subresource &subresource);

	void Release();
};


struct D3D11_index_buffer : public NGL_index_buffer
{
	uint32_t m_num_indices;
	ID3D11Buffer	*pIBuffer;
	DXGI_FORMAT m_D3D11_format;

	D3D11_index_buffer() : m_num_indices(0), pIBuffer(0)
	{
		m_D3D11_format = DXGI_FORMAT_UNKNOWN;
	}
	~D3D11_index_buffer();

	void Release();
};


struct D3D11_ubo
{
	ID3D11Buffer *m_buffer;
	uint8_t *m_mapped_ptr;

	D3D11_ubo() : m_buffer(0), m_mapped_ptr(0)
	{

	}
	void Allocate(uint32_t memory_size);
	~D3D11_ubo();
};


struct D3D11_uniform_group
{
	D3D11_ubo m_ubos[NGL_NUM_SHADER_TYPES];
	uint32_t m_ubo_memory_sizes[NGL_NUM_SHADER_TYPES];
	uint32_t m_ubo_binding_points[NGL_NUM_SHADER_TYPES];

	void MapUBO(uint32_t shader_type);
	void UnmapUBO(uint32_t shader_type);

	void BindUniform(const NGL_used_uniform &uu, const void *ptr);

	D3D11_uniform_group()
	{
		memset(m_ubos, 0, sizeof(m_ubos));
		memset(m_ubo_memory_sizes, 0, sizeof(m_ubo_memory_sizes));
		memset(m_ubo_binding_points, 0, sizeof(m_ubo_binding_points));
	}
};


struct D3D11_renderer : public NGL_renderer
{
	struct _used_vertex_buffer
	{
		uint32_t m_slot;
		uint32_t m_buffer_idx;
	};

	ID3D11VertexShader *pVS;
	ID3D11PixelShader *pPS;
	ID3D11ComputeShader *pCS;
	ID3D11InputLayout *pLayout;
	std::vector<_used_vertex_buffer> m_used_vbs;
	D3D11_uniform_group m_uniform_groups[3];
	int32_t m_max_SR_bindpoint[NGL_NUM_SHADER_TYPES];
	int32_t m_max_CB_bindpoint[NGL_NUM_SHADER_TYPES];
	int32_t m_max_UAV_bindpoint[NGL_NUM_SHADER_TYPES];

	D3D11_renderer();
	~D3D11_renderer();

	static ID3DBlob* CompileShader(const std::string &str, const std::string &entry_point, const std::string &version, std::string &info_string);
	void LinkShader();
	bool GetActiveAttribs2(_shader_reflection &r, uint32_t num_vbos, uint32_t *vbos, std::vector<D3D11_INPUT_ELEMENT_DESC> &input_elements);
	void GetActiveResources2(_shader_reflection &r, std::vector<NGL_shader_uniform> &application_uniforms);
};


struct D3D11_subpass
{
	struct _color_clear_data
	{
		ID3D11RenderTargetView *m_rtv;
		float m_value[4];
		_color_clear_data()
		{
			m_rtv = nullptr;
			memset(m_value, 0, sizeof(m_value));
		}
	};

	struct _depth_clear_data
	{
		ID3D11DepthStencilView *m_dsv;
		float m_value;
		_depth_clear_data()
		{
			m_dsv = nullptr;
			m_value = 0.0f;
		}
	};

	std::string m_name;
	std::vector<_color_clear_data> m_color_clears;
	std::vector<_depth_clear_data> m_depth_clear;
	std::vector<ID3D11View*> m_load_discards;
	std::vector<ID3D11View*> m_store_discards;

	std::vector<NGL_attachment_descriptor> m_color_attachments;
	NGL_attachment_descriptor m_depth_attachment;

	std::vector<ID3D11RenderTargetView*> m_color_attachment_views;
	ID3D11DepthStencilView *m_depth_stencil_view;

	//std::vector<NGL_attachment_descriptor> m_depth_attachment;
	//ID3D11DepthStencilView *used_depth_attachment;
	//ID3D11DepthStencilView *used_readonly_depth_attachment;
	//std::vector<ID3D11RenderTargetView*> used_color_attachments;

	D3D11_subpass()// : used_depth_attachment(0), used_readonly_depth_attachment(0)
	{
		m_depth_stencil_view = nullptr;
	}

	void BindRenderTargets();
	void ExecuteLoadOperations();
	void ExecuteStoreOperations();
};


struct D3D11_job : public NGL_job
{
	bool m_is_started;
	std::vector<D3D11_subpass> m_subpasses;
	D3D11_VIEWPORT m_viewport;
	uint32_t m_current_subpass;

	D3D11_job() : m_is_started(false)
	{
		m_current_subpass = 0;
	}
	~D3D11_job()
	{
	}


	NGL_renderer* CreateRenderer(NGL_state &sh, uint32_t num_vbos, uint32_t *vbos);

	void UnbindResources(const D3D11_renderer *renderer);

	D3D11_subpass& F()
	{
		return m_subpasses[m_current_subpass - 1];
	}

	D3D11_subpass& G()
	{
		return m_subpasses[m_current_subpass];
	}

	void CreateSubpasses();
};


struct BlendStateComparator
{
	bool operator()(const NGL_state& A, const NGL_state& B) const
	{
		return memcmp(&A.m_blend_state, &B.m_blend_state, sizeof(A.m_blend_state)) > 0;
	}
};


struct DepthStateComparator
{
	bool operator()(const NGL_state& A, const NGL_state& B) const
	{
		return memcmp(&A.m_depth_state, &B.m_depth_state, sizeof(A.m_depth_state)) > 0;
	}
};


struct RasterStateComparator
{
	bool operator()(const NGL_state& A, const NGL_state& B) const
	{
		if (A.m_cull_mode == B.m_cull_mode)
		{
			if (A.m_depth_state.m_func == B.m_depth_state.m_func)
			{
				return A.m_depth_state.m_mask < A.m_depth_state.m_mask;
			}

			return A.m_depth_state.m_func < B.m_depth_state.m_func;
		}
		return A.m_cull_mode < B.m_cull_mode;
	}
};


struct D3D11_instance : public NGL_instance
{
	std::vector<D3D11_vertex_buffer*> m_vertex_buffers;
	std::vector<D3D11_index_buffer*> m_index_buffers;
	std::vector<D3D11_texture*> m_textures;
	std::vector<D3D11_job*> m_jobs;

	std::vector<size_t*> m_null_resources;

	std::vector<std::pair<D3D11_SAMPLER_DESC, ID3D11SamplerState*>> m_samplers;
	std::map<std::string, ID3DBlob*> m_shader_cache;
	std::map<NGL_state, ID3D11BlendState*, BlendStateComparator> m_blend_states;
	std::map<NGL_state, ID3D11DepthStencilState*, DepthStateComparator> m_depth_states;
	std::map<NGL_state, ID3D11RasterizerState*, RasterStateComparator> m_raster_states;
	IDXGISwapChain1*        m_swapChain;
	ID3D11Device1*          m_d3dDevice;
	ID3D11DeviceContext1*   m_d3dContext;
	ID3D11Texture2D*		m_renderTargetTexture;
	ID3D11RenderTargetView* m_d3dRenderTargetView;
	ID3D11SamplerState *m_shadow_sampler;
	ID3D11Query *m_finish_query;

	ID3D11Debug* m_d3dDebug;
	ID3D11InfoQueue *m_d3dInfoQueue;

	D3D_DRIVER_TYPE m_driverType;
	int						m_major;
	int						m_minor;

	D3D11_instance();
	~D3D11_instance();
	static D3D11_instance *This;

	bool Init();
	void InitDevice();
	void getSwapChainTexture();

	HWND hWnd;

	ID3D11SamplerState* GetSamplerState(const D3D11_SAMPLER_DESC& desc);

	ID3D11BlendState* GetBlendState(NGL_state &state)
	{
		std::map<NGL_state, ID3D11BlendState*, BlendStateComparator>::iterator i = m_blend_states.find(state);

		if (i != m_blend_states.end())
		{
			return i->second;
		}
		else
		{
			D3D11_BLEND_DESC blendDesc;

			ZeroMemory(&blendDesc, sizeof(blendDesc));
			blendDesc.IndependentBlendEnable = FALSE;

			for (size_t i = 0; i < 8; i++)
			{
				D3D11_RENDER_TARGET_BLEND_DESC &rtbd = blendDesc.RenderTarget[i];

				switch (state.m_blend_state.m_funcs[i])
				{
				case NGL_BLEND_DISABLED:
				{
					rtbd.BlendEnable = FALSE;
					break;
				}
				case NGL_BLEND_ADDITIVE:
				{
					rtbd.BlendEnable = TRUE;
					rtbd.SrcBlend = D3D11_BLEND_ONE;
					rtbd.DestBlend = D3D11_BLEND_ONE;
					rtbd.BlendOp = D3D11_BLEND_OP_ADD;
					rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
					rtbd.DestBlendAlpha = D3D11_BLEND_ONE;
					rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
					break;
				}
				case NGL_BLEND_ALFA:
				{
					rtbd.BlendEnable = TRUE;
					rtbd.SrcBlend = D3D11_BLEND_SRC_ALPHA;
					rtbd.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
					rtbd.BlendOp = D3D11_BLEND_OP_ADD;
					rtbd.SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
					rtbd.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
					rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
					break;
				}
				case NGL_BLEND_ADDITIVE_ALFA:
				{
					rtbd.BlendEnable = TRUE;
					rtbd.SrcBlend = D3D11_BLEND_ONE;
					rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
					rtbd.DestBlend = D3D11_BLEND_SRC_ALPHA;
					rtbd.DestBlendAlpha = D3D11_BLEND_SRC_ALPHA;
					rtbd.BlendOp = D3D11_BLEND_OP_ADD;
					rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
					break;
				}
				case NGL_BLEND_ADDITIVE_INVERSE_ALFA:
				{
					rtbd.BlendEnable = TRUE;
					rtbd.SrcBlend = D3D11_BLEND_ONE;
					rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
					rtbd.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
					rtbd.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
					rtbd.BlendOp = D3D11_BLEND_OP_ADD;
					rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
					break;
				}
				case NGL_BLEND_DECAL:
				{
					rtbd.BlendEnable = TRUE;
					rtbd.SrcBlend = D3D11_BLEND_DEST_COLOR;
					rtbd.SrcBlendAlpha = D3D11_BLEND_DEST_ALPHA;
					rtbd.DestBlend = D3D11_BLEND_SRC_COLOR;
					rtbd.DestBlendAlpha = D3D11_BLEND_SRC_ALPHA;
					rtbd.BlendOp = D3D11_BLEND_OP_ADD;
					rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
					break;
				}
				case NGL_BLEND_MODULATIVE:
				{
					rtbd.BlendEnable = TRUE;
					rtbd.SrcBlend = D3D11_BLEND_DEST_COLOR;
					rtbd.SrcBlendAlpha = D3D11_BLEND_DEST_ALPHA;
					rtbd.DestBlend = D3D11_BLEND_ZERO;
					rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
					rtbd.BlendOp = D3D11_BLEND_OP_ADD;
					rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
					break;
				}
				case NGL_BLEND_TRANSPARENT_ACCUMULATION:
				{
					rtbd.BlendEnable = TRUE;
					rtbd.SrcBlend = D3D11_BLEND_ONE;
					rtbd.SrcBlendAlpha = D3D11_BLEND_ZERO;
					rtbd.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
					rtbd.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
					rtbd.BlendOp = D3D11_BLEND_OP_ADD;
					rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
					break;
				}
				default:
				{
					_logf("Unknown blend state: %d", state.m_blend_state.m_funcs[i]);
					assert(0);
					break;
				}
				}

				rtbd.RenderTargetWriteMask = 0;
				rtbd.RenderTargetWriteMask += ((state.m_blend_state.m_masks[i] | NGL_CHANNEL_R) > 0) * D3D11_COLOR_WRITE_ENABLE_RED;
				rtbd.RenderTargetWriteMask += ((state.m_blend_state.m_masks[i] | NGL_CHANNEL_G) > 0) * D3D11_COLOR_WRITE_ENABLE_GREEN;
				rtbd.RenderTargetWriteMask += ((state.m_blend_state.m_masks[i] | NGL_CHANNEL_B) > 0) * D3D11_COLOR_WRITE_ENABLE_BLUE;
				rtbd.RenderTargetWriteMask += ((state.m_blend_state.m_masks[i] | NGL_CHANNEL_A) > 0) * D3D11_COLOR_WRITE_ENABLE_ALPHA;
			}

			ID3D11BlendState *blend_state;
			DX_THROW_IF_FAILED(m_d3dDevice->CreateBlendState(&blendDesc, &blend_state));
			SetDebugObjectName(blend_state, "blend_state");
			m_blend_states[state] = blend_state;

			return blend_state;
		}
	}

	ID3D11DepthStencilState* GetDepthState(NGL_state &state)
	{
		std::map<NGL_state, ID3D11DepthStencilState*, DepthStateComparator>::iterator i = m_depth_states.find(state);

		if (i != m_depth_states.end())
		{
			return i->second;
		}
		else
		{
			D3D11_DEPTH_STENCIL_DESC depthDesc = CD3D11_DEPTH_STENCIL_DESC(D3D11_DEFAULT);

			switch (state.m_depth_state.m_func)
			{
			case NGL_DEPTH_DISABLED:
			{
				depthDesc.DepthEnable = FALSE;
				depthDesc.DepthFunc = D3D11_COMPARISON_NEVER;
				break;
			}
			case NGL_DEPTH_LESS:
			{
				depthDesc.DepthEnable = TRUE;
				depthDesc.DepthFunc = D3D11_COMPARISON_LESS;
				break;
			}
			case NGL_DEPTH_GREATER:
			{
				depthDesc.DepthEnable = TRUE;
				depthDesc.DepthFunc = D3D11_COMPARISON_GREATER;
				break;
			}
			case NGL_DEPTH_TO_FAR:
			{
				depthDesc.DepthEnable = TRUE;
				depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
				//glDepthFunc( GL_EQUAL);
				//glDepthRange( 1, 1);
				break;
			}
			case NGL_DEPTH_LESS_WITH_OFFSET:
			{
				depthDesc.DepthEnable = TRUE;
				depthDesc.DepthFunc = D3D11_COMPARISON_LESS;
				break;
			}
			case NGL_DEPTH_ALWAYS:
			{
				depthDesc.DepthEnable = TRUE;
				depthDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
				break;
			}
			case NGL_DEPTH_EQUAL:
			{
				depthDesc.DepthEnable = TRUE;
				depthDesc.DepthFunc = D3D11_COMPARISON_EQUAL;
				break;
			}
			case NGL_DEPTH_LEQUAL:
			{
				depthDesc.DepthEnable = TRUE;
				depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
				break;
			}
			default:
			{
				_logf("Unknown depth state: %d", state.m_depth_state.m_func);
				assert(0);
				break;
			}
			}

			depthDesc.DepthWriteMask = state.m_depth_state.m_mask ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;

			ID3D11DepthStencilState *depth_state;
			DX_THROW_IF_FAILED(m_d3dDevice->CreateDepthStencilState(&depthDesc, &depth_state));
			SetDebugObjectName(depth_state, "depth_state");
			m_depth_states[state] = depth_state;

			return depth_state;
		}
	}

	ID3D11RasterizerState* GetRasterizerState(NGL_state &state)
	{
		std::map<NGL_state, ID3D11RasterizerState*, RasterStateComparator>::iterator i = m_raster_states.find(state);

		if (i != m_raster_states.end())
		{
			return i->second;
		}
		else
		{
			D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);
			rasterDesc.FrontCounterClockwise = TRUE;
			rasterDesc.ScissorEnable = TRUE;
			rasterDesc.ScissorEnable = FALSE;

			switch (state.m_cull_mode)
			{
			case NGL_FRONT_SIDED:
			{
				rasterDesc.CullMode = D3D11_CULL_BACK;
				break;
			}
			case NGL_BACK_SIDED:
			{
				rasterDesc.CullMode = D3D11_CULL_FRONT;
				break;
			}
			case NGL_TWO_SIDED:
			{
				rasterDesc.CullMode = D3D11_CULL_NONE;
				break;
			}
			default:
			{
				_logf("Unknown cull mode: %d", state.m_cull_mode);
				assert(0);
				break;
			}
			}

			if (state.m_depth_state.m_func == NGL_DEPTH_LESS_WITH_OFFSET)
			{
				rasterDesc.DepthBiasClamp = 200.0f;
				rasterDesc.SlopeScaledDepthBias = 1.0f;
			}

			ID3D11RasterizerState *raster_state;
			DX_THROW_IF_FAILED(m_d3dDevice->CreateRasterizerState(&rasterDesc, &raster_state));
			SetDebugObjectName(raster_state, "rasterizer_state");
			m_raster_states[state] = raster_state;

			return raster_state;

		}
	}
};


inline bool CreateJobFBO(D3D11_job *job)
{
	NGL_job_descriptor &descriptor = job->m_descriptor;

	for (size_t j = 0; j < job->m_subpasses.size(); j++)
	{
		D3D11_subpass &subpass = job->m_subpasses[j];

		for (size_t i = 0; i< subpass.m_color_attachments.size(); i++)
		{
			NGL_attachment_descriptor &atd = subpass.m_color_attachments[i];

			D3D11_texture *t = D3D11_instance::This->m_textures[atd.m_attachment.m_idx];

			if (!t->m_texture_descriptor.m_is_renderable)
			{
				_logf("not renderable\n");
				return false;
			}
			else if (!t->m_is_color)
			{
				_logf("type mismatch\n");
				return false;
			}

			job->m_current_state.m_viewport[0] = 0;
			job->m_current_state.m_viewport[1] = 0;
			job->m_current_state.m_viewport[2] = t->m_texture_descriptor.m_size[0] / (1 << atd.m_attachment.m_level);
			job->m_current_state.m_viewport[3] = t->m_texture_descriptor.m_size[1] / (1 << atd.m_attachment.m_level);

			job->m_current_state.m_scissor[0] = 0;
			job->m_current_state.m_scissor[1] = 0;
			job->m_current_state.m_scissor[2] = t->m_texture_descriptor.m_size[0] / (1 << atd.m_attachment.m_level);
			job->m_current_state.m_scissor[3] = t->m_texture_descriptor.m_size[1] / (1 << atd.m_attachment.m_level);
		}

		if (subpass.m_depth_stencil_view != nullptr)
		{
			NGL_attachment_descriptor &atd = subpass.m_depth_attachment;

			if (atd.m_attachment.m_idx == 0)
			{
				_logf("default depth attachment is not supported!\n");
				return false;
			}

			D3D11_texture *t = D3D11_instance::This->m_textures[atd.m_attachment.m_idx];

			if (!t->m_texture_descriptor.m_is_renderable)
			{
				_logf("not renderable\n");
				return false;
			}
			else if (t->m_is_color)
			{
				_logf("type mismatch\n");
				return false;
			}

			job->m_current_state.m_viewport[0] = 0;
			job->m_current_state.m_viewport[1] = 0;
			job->m_current_state.m_viewport[2] = t->m_texture_descriptor.m_size[0] / (1 << atd.m_attachment.m_level);
			job->m_current_state.m_viewport[3] = t->m_texture_descriptor.m_size[1] / (1 << atd.m_attachment.m_level);

			job->m_current_state.m_scissor[0] = 0;
			job->m_current_state.m_scissor[1] = 0;
			job->m_current_state.m_scissor[2] = t->m_texture_descriptor.m_size[0] / (1 << atd.m_attachment.m_level);
			job->m_current_state.m_scissor[3] = t->m_texture_descriptor.m_size[1] / (1 << atd.m_attachment.m_level);

		}
	}

	return true;
}
