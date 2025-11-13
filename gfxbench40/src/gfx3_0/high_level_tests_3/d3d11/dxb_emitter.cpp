/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "dxb_emitter.h"
#include "d3d11/DXUtils.h"
#include "d3d11/DX.h"

#define SAFE_DELETE(x)	if ((x)) { delete (x); (x) = NULL; }
#define SAFE_RELEASE(x)	if ((x)) { (x)->Release(); (x) = NULL; }

const D3D11_SO_DECLARATION_ENTRY DXB::DXBEmitter::m_streamOutputLayout[] =
{
    // stream index, semantic name, semantic index, start component, component count, output slot
	{ 0, "TEX_COORD",	0, 0, 3, 0 },	// Position
	{ 0, "TEX_COORD",	1, 0, 3, 0 },	// Age, Speed, Acceleration
	{ 0, "TEX_COORD",	2, 0, 3, 0 },	// Amplitude
	{ 0, "TEX_COORD",	3, 0, 3, 0 },	// Phase
	{ 0, "TEX_COORD",	4, 0, 3, 0 },	// Frequency
	{ 0, "TANGENT",		0, 0, 3, 0 },
	{ 0, "BINORMAL",	0, 0, 3, 0 },
	{ 0, "NORMAL",		0, 0, 3, 0 },
	{ 0, "TEX_COORD",	5, 0, 3, 0 },	// Velocity
};

const D3D11_INPUT_ELEMENT_DESC DXB::DXBEmitter::m_animationDataLayout[] =
{
	//  SemanticName; SemanticIndex; Format; InputSlot; AlignedByteOffset; InputSlotClass; InstanceDataStepRate;
	{ "TEX_COORD",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },	// Position
	{ "TEX_COORD",	1, DXGI_FORMAT_R32G32B32_FLOAT,	0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },	// Age, speed, acceleration
	{ "TEX_COORD",	2, DXGI_FORMAT_R32G32B32_FLOAT,	0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },	// Amplitude
	{ "TEX_COORD",	3, DXGI_FORMAT_R32G32B32_FLOAT,	0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },	// Phase
	{ "TEX_COORD",	4, DXGI_FORMAT_R32G32B32_FLOAT,	0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },	// Frequency
	{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 60, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "BINORMAL",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 72, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 84, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEX_COORD",	5, DXGI_FORMAT_R32G32B32_FLOAT,	0, 96, D3D11_INPUT_PER_VERTEX_DATA, 0 },	// Velocity
};

const D3D11_INPUT_ELEMENT_DESC DXB::DXBEmitter::m_renderVertexLayout[] =
{
	//  SemanticName; SemanticIndex; Format; InputSlot; AlignedByteOffset; InputSlotClass; InstanceDataStepRate;
	{ "POSITION",	0, DXGI_FORMAT_R32G32_FLOAT,	0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEX_COORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0,  8, D3D11_INPUT_PER_VERTEX_DATA, 0 },

	{ "TEX_COORD",	1, DXGI_FORMAT_R32G32B32_FLOAT,	1,  0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	// Position
	{ "TEX_COORD",	2, DXGI_FORMAT_R32_FLOAT,		1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	// Age
	{ "TEX_COORD",	5, DXGI_FORMAT_R32G32B32_FLOAT,	1, 96, D3D11_INPUT_PER_INSTANCE_DATA, 1 },	// Velocity
};

DXB::DXBEmitter::DXBEmitter(const std::string &name, KCL::ObjectType type, Node *parent, Object *owner) :
	 KCL::_emitter(name, type, parent, owner),
	 m_disabledRasterizerState(NULL),
	 m_startBirthIdx(0),
	 m_endBirthIdx(0),
	 m_constantBuffer(NULL),
	 m_animationShader(NULL),
	 m_geometryShader(NULL),
	 m_renderShader(NULL),
	 m_animateSource(NULL),
	 m_animateTarget(NULL),
	 m_renderSource(NULL),
	 m_particleInstanceLayout(NULL)
{
	m_instanceData[0] = NULL;
	m_instanceData[1] = NULL;
}

DXB::DXBEmitter::~DXBEmitter(void)
{
	SAFE_RELEASE(m_disabledRasterizerState);
	SAFE_RELEASE(m_geometryShader);
	SAFE_RELEASE(m_particleInstanceLayout);

	SAFE_DELETE(m_geometryData);
	SAFE_DELETE(m_instanceData[0]);
	SAFE_DELETE(m_instanceData[1]);
}

void DXB::DXBEmitter::Process()
{
	const int MAX_BUFFER_SIZE = 1000; //this shall be based on rate and lifetime
	m_max_particle_count = MAX_BUFFER_SIZE;

	std::set<std::string> defines;
	defines.insert("SV_30");
	KCL::KCL_Status result = KCL::KCL_TESTERROR_NOERROR;
	m_animationShader = Shader::CreateShader( "particleAdvect.vs", "particleAdvect.fs", &defines, result, Shader::ShaderTypes::TransformFeedback);

	UINT stride = sizeof(ParticleData);
	DX_THROW_IF_FAILED(
		DX::getDevice()->CreateGeometryShaderWithStreamOutput(
		&m_animationShader->m_vs.m_bytes[0],
		m_animationShader->m_vs.m_bytes.size(),
		m_streamOutputLayout,
		m_streamOutputLayoutCount,
		&stride,
		1,
		D3D11_SO_NO_RASTERIZED_STREAM,
		NULL,
		&m_geometryShader));

	if (result)
	{
		throw std::exception("Error in DXBEmitter::Process");
	}

	for (int i = 0; i < 2; i++)
	{
		m_instanceData[i] = dynamic_cast<DXB::DXBVertexBuffer*>(KCL::VertexBuffer::factory->CreateBuffer<ParticleData>(m_max_particle_count, false, true));
		m_instanceData[i]->commit();
	}

	DX_THROW_IF_FAILED(
			DX::getDevice()->CreateInputLayout(
				m_animationDataLayout,
				m_animationDataLayoutCount,
				&m_animationShader->m_vs.m_bytes[0],
				m_animationShader->m_vs.m_bytes.size(),
				&m_animationLayout));

	static const DirectX::XMFLOAT4 geometry[] =
	{
		DirectX::XMFLOAT4(-1.0f, -1.0f, 0.0f, 1.0f),
		DirectX::XMFLOAT4(1.0f,	-1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(1.0f,	1.0f, 1.0f, 0.0f),

		DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f),
		DirectX::XMFLOAT4(-1.0f, 1.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT4(-1.0f, -1.0f, 0.0f, 1.0f)
	};

	m_geometryData = KCL::VertexBuffer::factory->CreateBuffer<DirectX::XMFLOAT4>(geometry, 6);
	m_geometryData->commit();

	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(dsDesc));
	DX_THROW_IF_FAILED(
		DX::getDevice()->CreateDepthStencilState(&dsDesc, &m_disabledRasterizerState));

	// Initialize buffer pointers
	m_animateSource = m_instanceData[0];
	m_renderSource = m_instanceData[0];
	m_animateTarget = m_instanceData[1];
}

void DXB::DXBEmitter::Simulate(KCL::uint32 time_msec)
{
	m_prev_time2 = m_time;
	m_time = time_msec;

	float diff_time_sec = (m_time - m_prev_time2) / 1000.0f;

	m_actual_rate = m_rate;
	KCL::uint32 numSubsteps = CalculateNumSubsteps(diff_time_sec);
	
	if (m_spawning_rate_animated)
	{
		float nothing=0;
		float t = time_msec / 1000.0f;

		KCL::Vector4D result;
		
		KCL::_key_node::Get(result, m_spawning_rate_animated, t, nothing);

		if( result.x > 1.0f)
		{
			result.x = 1.0f;
		}
		m_actual_rate *= result.x;
	}

	DX::getContext()->OMSetDepthStencilState(m_disabledRasterizerState, 0);
	DX::getContext()->IASetInputLayout(m_animationLayout);
	DX::getContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	DX::getContext()->VSSetShader(m_animationShader->m_vs.m_shader.Get(), NULL, 0);
	DX::getContext()->GSSetShader(m_geometryShader, NULL, 0);
	DX::getContext()->PSSetShader(NULL, NULL, 0);

	for (KCL::uint32 i = 0; i < numSubsteps; i++)
	{
		simulateSubStep();
	}

	DX::getContext()->GSSetShader(NULL, NULL, 0);
}

void DXB::DXBEmitter::setConsts(ConstantBufferEmitter* buffer) const
{
	buffer->aperture = DirectX::XMFLOAT3(m_aperture.x, m_aperture.y, m_aperture.z);
	buffer->begin_size = m_begin_size;
	buffer->color = DirectX::XMFLOAT4(m_color.v);
	buffer->endBirthIndex = m_endBirthIdx;
	buffer->end_size = m_end_size;
	buffer->externalVelocity = DirectX::XMFLOAT3(m_external_velocity.v);
	buffer->focusdist = m_focus_distance;
	buffer->gravityFactor = m_gravity_factor;
	buffer->maxlife = m_lifespan;

	buffer->max_accel = m_max.m_acceleration;
	buffer->max_amplitude = DirectX::XMFLOAT3(m_max.m_amplitude.v);
	buffer->max_freq = DirectX::XMFLOAT3(m_max.m_frequency);
	buffer->max_speed = m_max.m_acceleration;

	buffer->min_accel = m_min.m_acceleration;
	buffer->min_amplitude = DirectX::XMFLOAT3(m_min.m_amplitude.v);
	buffer->min_freq = DirectX::XMFLOAT3(m_min.m_frequency);
	buffer->min_speed = m_min.m_acceleration;

	buffer->startBirthIndex = m_startBirthIdx;
	buffer->worldmat = DX::Float4x4toXMFloat4x4ATransposed(m_world_pom.v);
};

void DXB::DXBEmitter::swapBuffers()
{
	m_animateSource = m_animateTarget;
	m_animateTarget = m_renderSource;
	m_renderSource = m_animateSource;
}

void DXB::DXBEmitter::simulateSubStep()
{
	const float rate_divider = 40.0f;
	m_emit_count += m_actual_rate / rate_divider;
	m_emit_count = std::min<float>(m_emit_count, float(m_max_particle_count) / 4.0f); //cap max emission to limit overwriting existing 

	m_startBirthIdx = m_endBirthIdx;
	m_endBirthIdx = (m_startBirthIdx + (KCL::uint32)m_emit_count) % m_max_particle_count;

	m_visibleparticle_count = m_max_particle_count; //no CPU culling possible using transform feedback
	if( m_emit_count >= 1.0f)
	{
		m_emit_count = 0.0f;
	}

	ConstantBufferEmitter* buffer = (ConstantBufferEmitter*)m_constantBuffer->map();
	setConsts(buffer);
	m_constantBuffer->unmap();

	ID3D11Buffer* source = m_animateSource->getBuffer();
	UINT offsets = 0;
	UINT stride = m_animateSource->getVertexSize();
	DX::getContext()->IASetVertexBuffers(0, 1, &source, &stride, &offsets);

	ID3D11Buffer* target = m_animateTarget->getBuffer();
	DX::getContext()->SOSetTargets(1, &target, &offsets);

	DX::getContext()->Draw(m_visibleparticle_count, 0);

	// Unbind from stream-out: it is required inside the loop to ensure the buffer is not bound to the output when used as input
	target = NULL;
	DX::getContext()->SOSetTargets(1, &target, &offsets);

	swapBuffers();
}

void DXB::DXBEmitter::setRenderShader(const Shader* shader)
{
	if (m_renderShader == shader)
	{
		return;
	}

	SAFE_RELEASE(m_particleInstanceLayout);

	m_renderShader = NULL;
	if (shader)
	{
		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateInputLayout(
				m_renderVertexLayout,
				m_renderVertexLayoutCount,
				&shader->m_vs.m_bytes[0],
				shader->m_vs.m_bytes.size(),
				&m_particleInstanceLayout));

		m_renderShader = shader;
	}
}

void DXB::DXBEmitter::bindBuffers() const
{
	m_geometryData->bind(0);
	m_renderSource->bind(1);
	DX::getContext()->IASetInputLayout(m_particleInstanceLayout);
}

KCL::AnimatedEmitter* DXB::DXBAnimatedEmitterFactory::Create(const std::string &name, KCL::Node *parent, KCL::Object *owner)
{
	return new DXB::DXBEmitter(name, KCL::EMITTER2, parent, owner);
}